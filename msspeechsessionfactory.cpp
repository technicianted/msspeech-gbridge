#include <iostream>
#include <assert.h>
#include <fmt/printf.h>
#include <time.h>

#include "msspeechsessionfactory.h"
#include "msspeechsession.h"

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

MSSpeechSession * MSSpeechSessionFactory::getSession(const std::string &uri)
{
    this->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("Session requested for %s", uri));

    std::unique_lock<std::mutex> lock(this->lock);
    if (this->availableSessionsByUri.find(uri) == this->availableSessionsByUri.end() ||
        this->availableSessionsByUri[uri].size() == 0) {
        this->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("No sessions found for %s, creating new one", uri));
        this->createNewSession(uri);
        this->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("Waiting for session for %s", uri));

        // TODO: can't wait forever if connection failed
        while(this->availableSessionsByUri[uri].size() == 0) {
            this->sessionCondition.wait(lock);
        }
        // TODO: Verify session is still good due to race conditions
    }
    this->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("Got session for %s", uri));
    auto session = *(this->availableSessionsByUri[uri].begin());

    session->session = new MSSpeechSession(session->connection);
    this->sessionsBySession[session->session] = session;
    this->availableSessionsByUri[uri].erase(session);
    this->busySesssions.insert(session);
    return session->session;
}

void MSSpeechSessionFactory::putSession(MSSpeechSession *session)
{
    // TODO: track lifetime of connection
    std::lock_guard<std::mutex> lock(this->lock);

    assert(this->sessionsBySession.find(session) != this->sessionsBySession.end());
    auto recoSession = this->sessionsBySession[session];
    assert(this->busySesssions.find(recoSession) != this->busySesssions.end());
    this->availableSessionsByUri[recoSession->uri].insert(recoSession);
    this->sessionCondition.notify_all();
}

RecognitionSession * MSSpeechSessionFactory::createNewSession(const std::string &uri)
{
    auto session = new RecognitionSession();
    session->connection = NULL;
    session->session = NULL;
    session->factory = this;
    session->uri = uri;
    this->pendingSessions.insert(session);
    ms_speech_service_cancel_step(this->msspeechContext);

    return session;
}

void MSSpeechSessionFactory::start()
{
    this->terminateRunloop = false;
    this->runloopThread = std::thread([this]{
        this->runloopStarted = true;
        this->log(MS_SPEECH_LOG_INFO, "Starting session factory run loop");
    
        while(!this->terminateRunloop) {
            ms_speech_service_step(this->msspeechContext, 50000);

            processPendingSessions();
        }

        this->runloopStarted = false;
        this->log(MS_SPEECH_LOG_INFO, "Session factory run loop terminated");
    });

    sleep(1);
}

void MSSpeechSessionFactory::stop()
{
    this->log(MS_SPEECH_LOG_INFO, "Signalling run loop to terminate");

    this->terminateRunloop = true;
    ms_speech_service_cancel_step(this->msspeechContext);
    this->runloopThread.join();
}

void MSSpeechSessionFactory::processPendingSessions()
{
    std::lock_guard<std::mutex> lock(this->lock);
    if (this->pendingSessions.size() > 0) {
        ms_speech_client_callbacks_t callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.log = &msspeech_log;
        callbacks.provide_authentication_header = &msspeech_provide_authentication_header;
        callbacks.connection_error = &msspeech_connection_error;
        callbacks.client_ready = &msspeech_client_ready;
        callbacks.speech_startdetected = &msspeech_speech_startdetected;
        callbacks.speech_enddetected = &msspeech_speech_enddetected;
        callbacks.speech_hypothesis = &msspeech_speech_hypothesis;
        callbacks.speech_fragment = &msspeech_speech_fragment;
        callbacks.speech_result = &msspeech_speech_result;
        callbacks.turn_start = &msspeech_turn_start;
        callbacks.turn_end = &msspeech_turn_end;

        auto iter = this->pendingSessions.begin();
        while(iter != this->pendingSessions.end()) {
            auto session = *iter;
            ms_speech_connection_t connection;
            this->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("Starting connection to %s", session->uri));
            callbacks.user_data = session;
            int r = ms_speech_connect(this->msspeechContext, session->uri.c_str(), &callbacks, &connection);
            if (r) {
                this->log(MS_SPEECH_LOG_ERR, fmt::sprintf("Failed to create connection to %s: %s", session->uri, strerror(r)));
                iter++;
            }
            else {
                session->connection = connection;
                this->sessions.insert(session);
                this->pendingSessions.erase(iter++);
            }
        }
    }
}

void MSSpeechSessionFactory::log(ms_speech_log_level_t level, const std::string &message)
{
    time_t current_time;
    struct tm * time_info;
    char timeString[10];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

    std::cout << timeString << ": " << message << std::endl;
}

void MSSpeechSessionFactory::msspeech_connection_established(ms_speech_connection_t connection, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_connection_established: %s", session->uri));
}

void MSSpeechSessionFactory::msspeech_connection_error(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_connection_error: %s: %d: %s", session->uri, http_status, error_message));
}

void MSSpeechSessionFactory::msspeech_connection_closed(ms_speech_connection_t connection, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_connection_closed: %s", session->uri));
}

void MSSpeechSessionFactory::msspeech_client_ready(ms_speech_connection_t connection, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    std::lock_guard<std::mutex> lock(factory->lock);
    assert(factory->sessions.find(session) != factory->sessions.end());

    factory->availableSessionsByUri[session->uri].insert(session);
    factory->sessionCondition.notify_all();
}

const char * MSSpeechSessionFactory::msspeech_provide_authentication_header(ms_speech_connection_t connection, void *user_data, size_t max_len)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->sessions.find(session) != factory->sessions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_provide_authentication_header: %s", session->uri));

    static char buffer[1024];
    sprintf(buffer, "Ocp-Apim-Subscription-Key: %s", factory->msspeechSubscriptionKey.c_str());
    return buffer;
}

void MSSpeechSessionFactory::msspeech_message_overlay(ms_speech_connection_t connection, ms_speech_user_message_type type, json_object *body, void *user_data)
{

}

void MSSpeechSessionFactory::msspeech_speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_speech_startdetected: %s", session->uri));
    session->session->msspeechStartDetected(message);
}

void MSSpeechSessionFactory::msspeech_speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_speech_enddetected: %s", session->uri));
    session->session->msspeechEndDetected(message);
}

void MSSpeechSessionFactory::msspeech_speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_speech_hypothesis: %s", session->uri));
    session->session->msspeechHypothesis(message);
}

void MSSpeechSessionFactory::msspeech_speech_fragment(ms_speech_connection_t connection, ms_speech_fragment_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_speech_fragment: %s", session->uri));
    session->session->msspeechFragment(message);
}

void MSSpeechSessionFactory::msspeech_speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_result: %s", session->uri));
    session->session->msspeechResult(message);
}

void MSSpeechSessionFactory::msspeech_turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_turn_start: %s", session->uri));
    session->session->msspeechTurnStart(message);
}

void MSSpeechSessionFactory::msspeech_turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data)
{
    auto session = static_cast<RecognitionSession *>(user_data);
    auto factory = session->factory;
    {
        std::lock_guard<std::mutex> lock(factory->lock);
        assert(factory->busySesssions.find(session) != factory->busySesssions.end());
    }

    factory->log(MS_SPEECH_LOG_DEBUG, fmt::sprintf("msspeech_turn_end: %s", session->uri));
    session->session->msspeechTurnEnd(message);
}

void MSSpeechSessionFactory::msspeech_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message)
{
    log(level, message);
}

void MSSpeechSessionFactory::msspeech_global_log(ms_speech_log_level_t level, const char *message)
{
    log(level, message);
}
