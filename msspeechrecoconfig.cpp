#include "msspeechrecoconfig.h"

#include <fmt/format.h>

MSSpeechRecoConfig::MSSpeechRecoConfig()
{
    this->_recognitionMode = MSSpeechRecognitionMode::INTERACTIVE;
    this->_profanityFilterMode = MSSpeechProfanityFilterMode::MASK;
    this->_enableIntermediateResults = false;
}

::grpc::Status MSSpeechRecoConfig::load(const ::google::cloud::speech::v1::StreamingRecognitionConfig &googleSpeechConfig)
{
    auto recoConfig = googleSpeechConfig.config();
 
    // TODO: Transcode?
    auto encoding = recoConfig.encoding();
    if (encoding != ::google::cloud::speech::v1::RecognitionConfig::LINEAR16) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "only LEANEAR16 is supported");
    }

    // check for supported fields
    if (recoConfig.max_alternatives() > 5) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "max_alternatives cannot be greater than 5");
    }
    if (recoConfig.speech_contexts().size() > 0) {
        return ::grpc::Status(::grpc::INVALID_ARGUMENT, "speech_contexts not supported");
    }
    if (recoConfig.enable_word_time_offsets()) {
        // TODO: let it slide. default is on.
        // return ::grpc::Status(::grpc::INVALID_ARGUMENT, "word time offsets not supported");
    }

    this->_recognitionMode = MSSpeechRecognitionMode::INTERACTIVE;
    if (!googleSpeechConfig.single_utterance())
        this->_recognitionMode = MSSpeechRecognitionMode::DICTATION;
    this->_enableIntermediateResults = googleSpeechConfig.interim_results();

    this->_language = recoConfig.language_code();
    this->_profanityFilterMode = MSSpeechProfanityFilterMode::NONE;
    if (recoConfig.profanity_filter())
        this->_profanityFilterMode = MSSpeechProfanityFilterMode::MASK;

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
