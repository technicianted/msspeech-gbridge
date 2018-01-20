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

#ifndef __STREAMINGRPCREQUESTHANDLER_H__
#define __STREAMINGRPCREQUESTHANDLER_H__

#include <grpc++/grpc++.h>
#include <spdlog/spdlog.h>

#include "msspeechsessionhandler.h"
#include "msspeechrecoconfig.h"
#include "google/cloud/speech/v1/cloud_speech.grpc.pb.h"

class MSSpeechSession;

/**
 * \brief Class handling the result side of the streaming call.
 * 
 * It handles the interaction with MSSpeechSession and mapping and sending
 * responses back to the client.
 */
class StreamingRpcRequestHandler : public MSSpeechSessionHandler
{
public:
    /**
     * \brief Construct a new handler. One is created per call.
     * 
     * \param session the MSSpeechSession object to use for this call.
     * \param msspeechRecoConfig speech recognition configurations for this call.
     * \param writer gRPC writer side of the streaming call.
     */
    StreamingRpcRequestHandler(
        std::shared_ptr<spdlog::logger> &logger,
        MSSpeechSession *session, 
        const MSSpeechRecoConfig &msspeechRecoConfig,
        ::grpc::internal::WriterInterface<::google::cloud::speech::v1::StreamingRecognizeResponse> *writer);

    /**
     * \brief Block the caller until speech recognition has been completed.
     */
    void waitForCompletion();

    // MSSpeechSessinHandler callbacks
    void speechStartdetected(ms_speech_startdetected_message_t *message);
	void speechEnddetected(ms_speech_enddetected_message_t *message);
    void speechHypothesis(ms_speech_hypothesis_message_t *message);
    void speechFragment(ms_speech_fragment_message_t *message);
    void speechResult(ms_speech_result_message_t *message);

private:
    std::shared_ptr<spdlog::logger> &logger;
    MSSpeechSession *session;
    MSSpeechRecoConfig msspeechRecoConfig;
    ::grpc::internal::WriterInterface<::google::cloud::speech::v1::StreamingRecognizeResponse> *writer;
    std::string currentDictationPhrase;
};

#endif
