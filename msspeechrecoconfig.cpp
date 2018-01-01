#include "msspeechrecoconfig.h"

#include <fmt/format.h>

MSSpeechRecoConfig::MSSpeechRecoConfig()
{
    this->_recognitionMode = MSSpeechRecognitionMode::INTERACTIVE;
    this->_profanityFilterMode = MSSpeechProfanityFilterMode::MASK;
    this->_enableIntermediateResults = false;
}

::grpc::Status MSSpeechRecoConfig::load(const ::google::cloud::speech::v1::RecognitionConfig &googleRecoConfig)
{
    // TODO: Transcode?
    auto encoding = googleRecoConfig.encoding();
    if (encoding != ::google::cloud::speech::v1::RecognitionConfig::LINEAR16) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "only LEANEAR16 is supported");
    }

    // check for supported fields
    if (googleRecoConfig.max_alternatives() > 5) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "max_alternatives cannot be greater than 5");
    }
    if (googleRecoConfig.speech_contexts().size() > 0) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "speech_contexts not supported");
    }
    if (googleRecoConfig.enable_word_time_offsets()) {
        // TODO: let it slide
        // return ::grpc::Status(::grpc::INVALID_ARGUMENT, "word time offsets not supported");
    }

    this->_language = googleRecoConfig.language_code();
    this->_profanityFilterMode = MSSpeechProfanityFilterMode::NONE;
    if (googleRecoConfig.profanity_filter())
        this->_profanityFilterMode = MSSpeechProfanityFilterMode::MASK;

    return ::grpc::Status::OK;
} 

::grpc::Status MSSpeechRecoConfig::load(const ::google::cloud::speech::v1::StreamingRecognitionConfig &googleSpeechConfig)
{
    auto r = this->load(googleSpeechConfig.config());
    if (!r.ok())
        return r;
 
    this->_recognitionMode = MSSpeechRecognitionMode::INTERACTIVE;
    if (!googleSpeechConfig.single_utterance())
        this->_recognitionMode = MSSpeechRecognitionMode::DICTATION;
    this->_enableIntermediateResults = googleSpeechConfig.interim_results();

    return ::grpc::Status::OK;
}

std::string MSSpeechRecoConfig::getMSSpeechUrl() const
{
    std::string url = fmt::format(
        "wss://speech.platform.bing.com/speech/recognition/{0}/cognitiveservices/v1?language={1}&profanity={2}&format=detailed",
        this->_recognitionMode == MSSpeechRecognitionMode::INTERACTIVE ? "interactive" : "dictation",
        this->_language,
        this->_profanityFilterMode == MSSpeechProfanityFilterMode::MASK ? "mask" : "raw");
    return url;
}
