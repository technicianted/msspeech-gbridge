/*

Copyright 2017 technicianted

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

*/

#ifndef __MSSPEECHSESSION_H__
#define __MSSPEECHSESSION_H__

#include <string>
#include <ms_speech/ms_speech.h>

#include "msspeechsessionhandler.h"
#include "audioqueue.h"

class MSSpeechSession
{
public:
    class CallStatus
    {
    public:
        int httpStatus;
        std::string errorMessage;
    };

    MSSpeechSession(ms_speech_connection_t connection);
    ~MSSpeechSession();

    void recognizeStream(MSSpeechSessionHandler *sessionHandler);

    void sendAudio(const char *buffer, int buffer_len);
    void endAudio();

    CallStatus waitForCompletion();

    void msspeechConnectionClosed(unsigned int httpStatus, const std::string &message);
    void msspeechStartDetected(ms_speech_startdetected_message_t *message);
    void msspeechEndDetected(ms_speech_enddetected_message_t *message);
    void msspeechHypothesis(ms_speech_hypothesis_message_t *message);
    void msspeechFragment(ms_speech_fragment_message_t *message);
    void msspeechResult(ms_speech_result_message_t *message);
    void msspeechTurnStart(ms_speech_turn_start_message_t *message);
    void msspeechTurnEnd(ms_speech_turn_end_message_t *message);

private:
    ms_speech_connection_t connection;
    AudioQueue audioQueue;
    MSSpeechSessionHandler *sessionHandler;
    bool recognitionStarted;
    bool pendingAudio;
    std::mutex completionLock;

    static int msspeech_audio_callback(ms_speech_connection_t connection, unsigned char *buffer, int buffer_len, void *user_data);
};

#endif
