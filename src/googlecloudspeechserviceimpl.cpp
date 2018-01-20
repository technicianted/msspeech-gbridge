#include "googlecloudspeechserviceimpl.h"

#include <uuid/uuid.h>

#include "msspeechsessionfactory.h"
#include "msspeechsession.h"
#include "streamingrpcrequesthandler.h"
#include "syncrpcrequesthandler.h"

GoogleCloudSpeechServiceImpl::GoogleCloudSpeechServiceImpl(MSSpeechSessionFactory *sessionFactory)
{
    this->sessionFactory = sessionFactory;
}

// Performs synchronous speech recognition: receive results after all audio
// has been sent and processed.
::grpc::Status GoogleCloudSpeechServiceImpl::Recognize(
    ::grpc::ServerContext* context, 
    const ::google::cloud::speech::v1::RecognizeRequest* request, 
    ::google::cloud::speech::v1::RecognizeResponse* response)
{
   auto requestId = this->getRequestId(context);
    auto logger = spdlog::stderr_logger_mt(requestId);
    logger->trace("request metadata: {0}", this->dumpMetadata(context));
    logger->debug("Recognize request start");

    MSSpeechRecoConfig msspeechRecoConfig;
    auto r = msspeechRecoConfig.load(request->config());
    if (!r.ok())
        return r;

    if (request->audio().audio_source_case() != ::google::cloud::speech::v1::RecognitionAudio::kContent)
       return ::grpc::Status(::grpc::INVALID_ARGUMENT, "audio.content not provided");
    
    auto session = this->sessionFactory->getSession(msspeechRecoConfig.getMSSpeechUrl(), requestId);
    SyncRpcRequestHandler requestHandler(session, msspeechRecoConfig);
    auto recoResult = requestHandler.recognize(request->audio().content(), response);
    sessionFactory->putSession(session);

    return recoResult;
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
    auto requestId = this->getRequestId(context);
    auto logger = spdlog::stderr_logger_mt(requestId);
    logger->trace("request metadata: {0}", this->dumpMetadata(context));
    logger->debug("StreamingRecognize request start");

    ::google::cloud::speech::v1::StreamingRecognizeRequest request; 
    stream->Read(&request);

    if (!request.has_streaming_config()) {
        logger->notice("streaming_config not provided");
        return ::grpc::Status(::grpc::FAILED_PRECONDITION, "streaming_config not provided");
    }
 
    logger->debug("streaming_config: {0}", request.streaming_config().ShortDebugString());
    MSSpeechRecoConfig msspeechRecoConfig;
    auto r = msspeechRecoConfig.load(request.streaming_config());
    if (!r.ok()) {
        logger->notice("config mapping failed: {0}: {1}", r.error_code(), r.error_message());
        return r;
    }
    logger->debug("reco_config: {0}", msspeechRecoConfig.str());

    auto session = this->sessionFactory->getSession(msspeechRecoConfig.getMSSpeechUrl(), requestId);
    StreamingRpcRequestHandler requestHandler(logger, session, msspeechRecoConfig, stream);
    session->recognizeStream(&requestHandler);

    while (stream->Read(&request)) {
        // TODO: check we don't get configs
        auto payload = request.audio_content();
        auto payloadSize = payload.length();
        session->sendAudio(payload.data(), payloadSize);
    }
    session->endAudio();
    // TODO: handle errors properly
    session->waitForCompletion();

    sessionFactory->putSession(session);

    logger->debug("request end");

    return ::grpc::Status::OK;
}

std::string GoogleCloudSpeechServiceImpl::getRequestId(::grpc::ServerContext *context)
{
    uuid_t requestIdUuid;
    uuid_generate(requestIdUuid);
    char buffer[64];
    uuid_unparse(requestIdUuid, buffer);
    return std::string(buffer);
}

std::string GoogleCloudSpeechServiceImpl::dumpMetadata(::grpc::ServerContext *context)
{
    auto metadata = context->client_metadata();
    fmt::MemoryWriter writer;
    for(auto md : metadata) {
        writer.write("({0}: {1}) ", md.first, md.second);
    }

    return writer.c_str();
}