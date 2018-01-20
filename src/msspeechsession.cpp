#include "msspeechsession.h"

#include <iostream>

MSSpeechSession::MSSpeechSession(ms_speech_connection_t connection)
{
    this->connection = connection;
    this->recognitionStarted = false;
}

MSSpeechSession::~MSSpeechSession()
{
}

void MSSpeechSession::recognizeStream(MSSpeechSessionHandler *sessionHandler)
{
    if (this->recognitionStarted)
        throw std::logic_error("Recognition already started");

    this->sessionHandler = sessionHandler;
    this->pendingAudio = false;
    ms_speech_start_stream(this->connection, &msspeech_audio_callback, this->requestId.c_str(), this);
    this->recognitionStarted = true;
    this->completionLock.lock();
}

void MSSpeechSession::sendAudio(const char *buffer, int buffer_len)
{
    if (!this->recognitionStarted)
        throw std::logic_error("Recognition hasn't started");

    this->audioQueue.put(buffer, buffer_len);
    if (this->pendingAudio) {
        this->pendingAudio = false;
        ms_speech_resume_stream(this->connection);
    }
}

void MSSpeechSession::endAudio()
{
    if (!this->recognitionStarted)
        throw std::logic_error("Recognition hasn't started");
        
    this->audioQueue.markEnd();
}

void MSSpeechSession::setRequestId(const std::string &requestId)
{
    this->requestId = requestId;
}

const std::string & MSSpeechSession::getRequestId() const
{
    return requestId;
}

MSSpeechSession::CallStatus MSSpeechSession::waitForCompletion()
{
    // TODO: add timeout
    this->completionLock.lock();

    return CallStatus();
}

void MSSpeechSession::msspeechConnectionClosed(unsigned int httpStatus, const std::string &message)
{

}

void MSSpeechSession::msspeechStartDetected(ms_speech_startdetected_message_t *message)
{

}

void MSSpeechSession::msspeechEndDetected(ms_speech_enddetected_message_t *message)
{

}

void MSSpeechSession::msspeechHypothesis(ms_speech_hypothesis_message_t *message)
{
     this->sessionHandler->speechHypothesis(message);
}

void MSSpeechSession::msspeechFragment(ms_speech_fragment_message_t *message)
{
     this->sessionHandler->speechFragment(message);
}

void MSSpeechSession::msspeechResult(ms_speech_result_message_t *message)
{
    this->sessionHandler->speechResult(message);
}

void MSSpeechSession::msspeechTurnStart(ms_speech_turn_start_message_t *message)
{

}

void MSSpeechSession::msspeechTurnEnd(ms_speech_turn_end_message_t *message)
{
    this->completionLock.unlock();
}

int MSSpeechSession::msspeech_audio_callback(ms_speech_connection_t connection, unsigned char *buffer, int buffer_len, void *user_data)
{
    auto session = static_cast<MSSpeechSession *>(user_data);
    auto size = session->audioQueue.get((char *)buffer, buffer_len);
    if (size == -1) {
        return 0;
    }
    else if (size == 0) {
        session->pendingAudio = true;
        return -EAGAIN;
    }

    return size;
}
