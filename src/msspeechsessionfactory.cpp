#include <iostream>
#include <assert.h>
#include <fmt/printf.h>
#include <time.h>

#include "msspeechsessionfactory.h"
#include "msspeechsession.h"

std::shared_ptr<spdlog::logger> MSSpeechSessionFactory::Logger = spdlog::stderr_logger_mt("factory");

MSSpeechSessionFactory::MSSpeechSessionFactory(const std::string &msspeechSubscriptionKey, int maxSessions)
{
    this->msspeechSubscriptionKey = msspeechSubscriptionKey;
    this->maxSessions = maxSessions;
    this->runloopStarted = false;

    ms_speech_set_logging(65535, &msspeech_global_log);
    this->msspeechContext = ms_speech_create_context();
}

MSSpeechSessionFactory::~MSSpeechSessionFactory()
{
    if (runloopStarted)
        this->stop();
}

std::shared_ptr<MSSpeechSession> MSSpeechSessionFactory::getSession(const std::string &uri, const std::string &requestId)
{
    Logger->info("{0}: session requested for {1}", requestId, uri);

    std::unique_lock<std::mutex> lock(this->lock);
    if (this->availableSessionsByUri.find(uri) == this->availableSessionsByUri.end() ||
        this->availableSessionsByUri[uri].size() == 0) {
        Logger->debug("{0}: no sessions found for {1}, creating new one", requestId, uri);
        this->createNewSession(uri);
        Logger->debug("{0}: waiting for session for {1}", requestId, uri);

        // TODO: can't wait forever if connection failed
        while(this->availableSessionsByUri[uri].size() == 0) {
            this->sessionCondition.wait(lock);
        }
        // TODO: Verify session is still good due to race conditions
    }
    Logger->debug("{0}: got session for {1}", requestId, uri);
    auto session = *(this->availableSessionsByUri[uri].begin());

    session->session = std::shared_ptr<MSSpeechSession>(new MSSpeechSession(session->connection));
    session->requestId = requestId;
    session->checkedOut = true;
    session->checkoutTime = std::time(NULL);
    session->session->setRequestId(requestId);

    this->sessionsBySession[session->session] = session;
    this->availableSessionsByUri[uri].erase(session);
    this->busySesssions.insert(session);
    return session->session;
}

void MSSpeechSessionFactory::putSession(std::shared_ptr<MSSpeechSession> &session)
{
    // TODO: track lifetime of connection
    std::lock_guard<std::mutex> lock(this->lock);

    assert(this->sessionsBySession.find(session) != this->sessionsBySession.end());
    auto recoSession = this->sessionsBySession[session];
    assert(this->busySesssions.find(recoSession) != this->busySesssions.end());

    Logger->debug("{0}: returning session for {1}", recoSession->requestId, recoSession->uri);
   
    this->sessionsBySession.erase(recoSession->session);
     recoSession->requestId.clear();
    recoSession->session = NULL;
    recoSession->checkedOut = false;

    this->availableSessionsByUri[recoSession->uri].insert(recoSession);
    this->sessionCondition.notify_all();
}

std::shared_ptr<RecognitionSession> MSSpeechSessionFactory::createNewSession(const std::string &uri)
{
    std::shared_ptr<RecognitionSession> session(new RecognitionSession());
    session->connection = NULL;
    session->session = NULL;
    session->factory = this;
    session->uri = uri;
    session->callbackData = NULL;
    this->pendingSessions.insert(session);
    ms_speech_service_cancel_step(this->msspeechContext);

    return session;
}

void MSSpeechSessionFactory::start()
{
    this->terminateRunloop = false;
    this->runloopThread = std::thread([this]{
        this->runloopStarted = true;
        Logger->info("starting session factory run loop");
    
        while(!this->terminateRunloop) {
            ms_speech_service_step(this->msspeechContext, 50000);

            processPendingSessions();
        }

        this->runloopStarted = false;
        Logger->info("session factory run loop terminated");
    });
}

void MSSpeechSessionFactory::stop()
{
    Logger->info("signalling run loop to terminate");

    this->terminateRunloop = true;
    ms_speech_service_cancel_step(this->msspeechContext);
    this->runloopThread.join();
}

void MSSpeechSessionFactory::processPendingSessions()
{
    std::unique_lock<std::mutex> lock(this->lock);
    if (this->pendingSessions.size() > 0) {
        auto pendingSession = this->pendingSessions;
        lock.unlock();

        ms_speech_client_callbacks_t callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.log = &msspeech_log;
        callbacks.provide_authentication_header = &msspeech_provide_authentication_header;
        callbacks.connection_closed = &msspeech_connection_closed;
        callbacks.connection_error = &msspeech_connection_error;
        callbacks.client_ready = &msspeech_client_ready;
        callbacks.speech_startdetected = &msspeech_speech_startdetected;
        callbacks.speech_enddetected = &msspeech_speech_enddetected;
        callbacks.speech_hypothesis = &msspeech_speech_hypothesis;
        callbacks.speech_fragment = &msspeech_speech_fragment;
        callbacks.speech_result = &msspeech_speech_result;
        callbacks.turn_start = &msspeech_turn_start;
        callbacks.turn_end = &msspeech_turn_end;

        for(auto pendingSession : pendingSessions) {
            ms_speech_connection_t connection;
            Logger->debug("starting connection to {0}", pendingSession->uri);
            pendingSession->callbackData = new std::shared_ptr<RecognitionSession>(pendingSession);
            callbacks.user_data = pendingSession->callbackData;
            int r = ms_speech_connect(this->msspeechContext, pendingSession->uri.c_str(), &callbacks, &connection);
            if (r) {
                Logger->error("failed to create connection to {0}: {1}", pendingSession->uri, strerror(r));
            }
            else {
                pendingSession->connection = connection;
                lock.lock();
                if (this->pendingSessions.erase(pendingSession)) {
                    this->sessions.insert(pendingSession);
                }
            }
        }
    }
}

void MSSpeechSessionFactory::log(ms_speech_log_level_t level, const std::string &message)
{
    switch(level) {
        case MS_SPEECH_LOG_ERR:
            Logger->error(message);
            break;
	    case MS_SPEECH_LOG_WARN:
            Logger->warn(message);
            break;
	    case MS_SPEECH_LOG_NOTICE:
            Logger->notice(message);
            break;
	    case MS_SPEECH_LOG_INFO:
            Logger->info(message);
            break;
	    case MS_SPEECH_LOG_DEBUG:
            Logger->debug(message);
            break;
	    case MS_SPEECH_LOG_PARSER:
            Logger->debug(fmt::format("parser: {0}", message));
            break;
	    case MS_SPEECH_LOG_HEADER:
            Logger->debug(fmt::format("header: {0}", message));
            break;
	    case MS_SPEECH_LOG_EXT:
            Logger->debug(fmt::format("ext: {0}", message));
            break;
	    case MS_SPEECH_LOG_CLIENT:
            Logger->debug(fmt::format("client: {0}", message));
            break;
	    case MS_SPEECH_LOG_LATENCY:
            Logger->debug(fmt::format("latency: {0}", message));
            break;
	    case MS_SPEECH_LOG_USER:
            Logger->debug(fmt::format("user: {0}", message));
            break;
    }
}

void MSSpeechSessionFactory::msspeech_connection_established(ms_speech_connection_t connection, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    Logger->debug("msspeech_connection_established: {0}", session->uri);
}

void MSSpeechSessionFactory::msspeech_connection_error(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    Logger->debug("{0}: msspeech_connection_error: {1}: {2}: {3}", session->requestId, session->uri, http_status, error_message);
}

void MSSpeechSessionFactory::msspeech_connection_closed(ms_speech_connection_t connection, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    Logger->debug("{0}: msspeech_connection_closed: {1}", session->requestId, session->uri);
    if (session->checkedOut) {
        // TODO: need to tell handlers about this.
    }

    factory->availableSessionsByUri[session->uri].erase(session);
    factory->sessionsBySession.erase(session->session);
    factory->sessions.erase(session);
}

void MSSpeechSessionFactory::msspeech_client_ready(ms_speech_connection_t connection, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    std::lock_guard<std::mutex> lock(factory->lock);
    assert(factory->sessions.find(session) != factory->sessions.end());

    Logger->debug("msspeech_client_ready: {0}", session->uri);

    factory->availableSessionsByUri[session->uri].insert(session);
    factory->sessionCondition.notify_all();
}

const char * MSSpeechSessionFactory::msspeech_provide_authentication_header(ms_speech_connection_t connection, void *user_data, size_t max_len)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    Logger->debug("{0}: msspeech_provide_authentication_header", session->requestId);

    static char buffer[1024];
    sprintf(buffer, "Ocp-Apim-Subscription-Key: %s", factory->msspeechSubscriptionKey.c_str());
    return buffer;
}

void MSSpeechSessionFactory::msspeech_message_overlay(ms_speech_connection_t connection, ms_speech_user_message_type type, json_object *body, void *user_data)
{

}

void MSSpeechSessionFactory::msspeech_speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_speech_startdetected", session->requestId);
    session->session->msspeechStartDetected(message);
}

void MSSpeechSessionFactory::msspeech_speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_speech_enddetected", session->requestId);
    session->session->msspeechEndDetected(message);
}

void MSSpeechSessionFactory::msspeech_speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_speech_hypothesis", session->requestId);
    session->session->msspeechHypothesis(message);
}

void MSSpeechSessionFactory::msspeech_speech_fragment(ms_speech_connection_t connection, ms_speech_fragment_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_speech_fragment", session->requestId);
    session->session->msspeechFragment(message);
}

void MSSpeechSessionFactory::msspeech_speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_result", session->requestId);
    session->session->msspeechResult(message);
}

void MSSpeechSessionFactory::msspeech_turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_turn_start", session->requestId);
    session->session->msspeechTurnStart(message);
}

void MSSpeechSessionFactory::msspeech_turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    Logger->debug("{0}: msspeech_turn_end", session->requestId);
    session->session->msspeechTurnEnd(message);
}

void MSSpeechSessionFactory::msspeech_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message)
{
    auto session = *static_cast<std::shared_ptr<RecognitionSession> *>(user_data);

    auto m = fmt::format("{0}: {1}", session->requestId, message);
    if (m.length() > 0 && m[m.length()-1] == '\n')
        m = m.substr(0, m.length()-1);

    log(level, m);
}

void MSSpeechSessionFactory::msspeech_global_log(ms_speech_log_level_t level, const char *message)
{
    auto m = fmt::format("msspeech: {0}", message);
    if (m.length() > 0 && m[m.length()-1] == '\n')
        m = m.substr(0, m.length()-1);

    log(level, m);
}

RecognitionSession::RecognitionSession()
{
    this->factory = NULL;
    this->connection = NULL;
    this->session = NULL;
    this->checkedOut = false;
    this->checkoutTime = std::time(NULL);
    this->callbackData = NULL;
}

RecognitionSession::~RecognitionSession()
{
    delete this->callbackData;
}
