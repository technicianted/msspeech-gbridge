#include "googlecloudspeechserviceimpl.h"
#include "msspeechsessionfactory.h"
#include "msspeechsession.h"
#include "streamingrpcrequesthandler.h"

GoogleCloudSpeechServiceImpl::GoogleCloudSpeechServiceImpl(MSSpeechSessionFactory *sessionFactory)
{
    this->sessionFactory = sessionFactory;
}

// Performs synchronous speech recognition: receive results after all audio
// has been sent and processed.
::grpc::Status GoogleCloudSpeechServiceImpl::Recognize(::grpc::ServerContext* context, const ::google::cloud::speech::v1::RecognizeRequest* request, ::google::cloud::speech::v1::RecognizeResponse* response)
{
    return ::grpc::Status(::grpc::UNIMPLEMENTED, "not implemented");
}

// Performs asynchronous speech recognition: receive results via the
// google.longrunning.Operations interface. Returns either an
// `Operation.error` or an `Operation.response` which contains
// a `LongRunningRecognizeResponse` message.
::grpc::Status GoogleCloudSpeechServiceImpl::LongRunningRecognize(::grpc::ServerContext* context, const ::google::cloud::speech::v1::LongRunningRecognizeRequest* request, ::google::longrunning::Operation* response)
{
    return ::grpc::Status(::grpc::UNIMPLEMENTED, "not implemented");
}

// Performs bidirectional streaming speech recognition: receive results while
// sending audio. This method is only available via the gRPC API (not REST).
::grpc::Status GoogleCloudSpeechServiceImpl::StreamingRecognize(
    ::grpc::ServerContext* context, 
    ::grpc::ServerReaderWriter< 
        ::google::cloud::speech::v1::StreamingRecognizeResponse, 
        ::google::cloud::speech::v1::StreamingRecognizeRequest>* stream)
{
    ::google::cloud::speech::v1::StreamingRecognizeRequest request; 
    stream->Read(&request);

    if (!request.has_streaming_config()) {
        return ::grpc::Status(::grpc::FAILED_PRECONDITION, "streaming_config not provided");
    }
    auto streamingConfig = request.streaming_config();
    auto recoConfig = streamingConfig.config();
 
    // TODO: Transcode?
    auto encoding = recoConfig.encoding();
    if (encoding != ::google::cloud::speech::v1::RecognitionConfig::LINEAR16) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "only LEANEAR16 is supported");
    }

    MSSpeechRecoConfig msspeechRecoConfig;
    auto r = msspeechRecoConfig.load(streamingConfig);
    if (!r.ok())
        return r;

    auto session = this->sessionFactory->getSession(msspeechRecoConfig.getMSSpeechUrl());
    StreamingRpcRequestHandler requestHandler(session, msspeechRecoConfig, stream);
    session->recognizeStream(&requestHandler);

    while (stream->Read(&request)) {
        // TODO: check we don't get configs
        auto payload = request.audio_content();
        auto payloadSize = payload.length();
        session->sendAudio(payload.data(), payloadSize);
    }
    session->endAudio();
    session->waitForCompletion();

    sessionFactory->putSession(session);

    return ::grpc::Status::OK;
}
