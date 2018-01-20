#include "streamingrpcrequesthandler.h"
#include "msspeechsession.h"

#include <fmt/format.h>

StreamingRpcRequestHandler::StreamingRpcRequestHandler(
    std::shared_ptr<spdlog::logger> &logger,
    MSSpeechSession *session, 
    const MSSpeechRecoConfig &msspeechRecoConfig,
    ::grpc::internal::WriterInterface<::google::cloud::speech::v1::StreamingRecognizeResponse> *writer)
    : logger(logger)
{
    this->session = session;
    this->msspeechRecoConfig = msspeechRecoConfig;
    this->writer = writer;
}

void StreamingRpcRequestHandler::waitForCompletion()
{
    session->waitForCompletion();
}

void StreamingRpcRequestHandler::speechStartdetected(ms_speech_startdetected_message_t *message)
{
}

void StreamingRpcRequestHandler::speechEnddetected(ms_speech_enddetected_message_t *message)
{
    if (this->msspeechRecoConfig.recognitionMode() == MSSpeechRecognitionMode::INTERACTIVE) {
        ::google::cloud::speech::v1::StreamingRecognizeResponse response;
        response.set_speech_event_type(::google::cloud::speech::v1::StreamingRecognizeResponse::END_OF_SINGLE_UTTERANCE);
        this->writer->Write(response);
    }
}

void StreamingRpcRequestHandler::speechHypothesis(ms_speech_hypothesis_message_t *message)
{
    if (this->msspeechRecoConfig.enableIntermediateResults()) {
        ::google::cloud::speech::v1::StreamingRecognizeResponse response;
        auto *result = response.add_results();
        result->set_is_final(false);
        auto alt = result->add_alternatives();
        alt->set_transcript(message->text);

        this->writer->Write(response);
    }
}

void StreamingRpcRequestHandler::speechFragment(ms_speech_fragment_message_t *message)
{
    if (this->msspeechRecoConfig.enableIntermediateResults()) {
        if (this->currentDictationPhrase.empty())
            this->currentDictationPhrase = message->text;
        else
            this->currentDictationPhrase = fmt::format("{0} {1}", this->currentDictationPhrase, message->text);

        ::google::cloud::speech::v1::StreamingRecognizeResponse response;
        auto *result = response.add_results();
        result->set_is_final(false);
        auto alt = result->add_alternatives();
        alt->set_transcript(this->currentDictationPhrase);

        this->writer->Write(response);
    }
}

void StreamingRpcRequestHandler::speechResult(ms_speech_result_message_t *message)
{
    // TODO: handle speech status
    ::google::cloud::speech::v1::StreamingRecognizeResponse response;
    switch(message->status) {
        case MS_SPEECH_RECO_SUCCESS:
        {
            auto *result = response.add_results();
            result->set_is_final(true);
            for (int i=0; i<message->num_phrase_results; i++) {
                auto phraseResult = &message->phrase_results[i];
                auto alt = result->add_alternatives();
                alt->set_transcript(phraseResult->display);
                alt->set_confidence(phraseResult->confidence);
            }
            this->writer->Write(response);
            break;
        }

        case MS_SPEECH_DICTATION_END:
            break;

        case MS_SPEECH_ERROR:
        case MS_SPEECH_NO_MATCH:
	    case MS_SPEECH_INITIAL_SILENCE_TIMEOUT:
	    case MS_SPEECH_BABBLE_TIMEOUT:
            break;
    }

    this->currentDictationPhrase.clear();
}
