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

#ifndef __SYNCRPCREQUESTHANDLER_H__
#define __SYNCRPCREQUESTHANDLER_H__

#include "msspeechsessionhandler.h"
#include "msspeechrecoconfig.h"

class MSSpeechSession;

class SyncRpcRequestHandler : public MSSpeechSessionHandler
{
public:
    SyncRpcRequestHandler(
        MSSpeechSession *session, 
        const MSSpeechRecoConfig &msspeechRecoConfig);
    ~SyncRpcRequestHandler();

    ::grpc::Status recognize(
        const std::string &audio,
        ::google::cloud::speech::v1::RecognizeResponse* response);

    // MSSpeechSessinHandler callbacks
    void speechStartdetected(ms_speech_startdetected_message_t *message);
	void speechEnddetected(ms_speech_enddetected_message_t *message);
    void speechHypothesis(ms_speech_hypothesis_message_t *message);
    void speechFragment(ms_speech_fragment_message_t *message);
    void speechResult(ms_speech_result_message_t *message);

private:
    MSSpeechSession *session;
    MSSpeechRecoConfig msspeechRecoConfig;
    ::google::cloud::speech::v1::RecognizeResponse* response;
};

#endif

