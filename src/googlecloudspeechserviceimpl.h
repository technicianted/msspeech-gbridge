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

#ifndef __GOOGLECLOUDSPEECHSERVICEIPLE_H__
#define __GOOGLECLOUDSPEECHSERVICEIPLE_H__

#include "google/cloud/speech/v1/cloud_speech.grpc.pb.h"

class MSSpeechSessionFactory;

class GoogleCloudSpeechServiceImpl final : public google::cloud::speech::v1::Speech::Service
{
public:
    GoogleCloudSpeechServiceImpl(MSSpeechSessionFactory *sessionFactory);

    // Performs synchronous speech recognition: receive results after all audio
    // has been sent and processed.
    virtual ::grpc::Status Recognize(::grpc::ServerContext* context, const ::google::cloud::speech::v1::RecognizeRequest* request, ::google::cloud::speech::v1::RecognizeResponse* response);
    // Performs asynchronous speech recognition: receive results via the
    // google.longrunning.Operations interface. Returns either an
    // `Operation.error` or an `Operation.response` which contains
    // a `LongRunningRecognizeResponse` message.
    virtual ::grpc::Status LongRunningRecognize(::grpc::ServerContext* context, const ::google::cloud::speech::v1::LongRunningRecognizeRequest* request, ::google::longrunning::Operation* response);
    // Performs bidirectional streaming speech recognition: receive results while
    // sending audio. This method is only available via the gRPC API (not REST).
    virtual ::grpc::Status StreamingRecognize(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::google::cloud::speech::v1::StreamingRecognizeResponse, ::google::cloud::speech::v1::StreamingRecognizeRequest>* stream);

private:
    MSSpeechSessionFactory *sessionFactory;

    std::string getRequestId(::grpc::ServerContext *context);
    std::string dumpMetadata(::grpc::ServerContext *context);
};

#endif

