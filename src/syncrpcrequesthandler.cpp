#include "syncrpcrequesthandler.h"
#include "msspeechsession.h"

SyncRpcRequestHandler::SyncRpcRequestHandler(
    std::shared_ptr<MSSpeechSession> &session, 
    const MSSpeechRecoConfig &msspeechRecoConfig)
{
    this->session = session;
    this->msspeechRecoConfig = msspeechRecoConfig;
}

SyncRpcRequestHandler::~SyncRpcRequestHandler()
{

}

::grpc::Status SyncRpcRequestHandler::recognize(
    const std::string &audio,
    ::google::cloud::speech::v1::RecognizeResponse* response)
{
    this->response = response;
    
    this->session->recognizeStream(this);
    this->session->sendAudio(audio.data(), audio.size());
    this->session->endAudio();
    this->session->waitForCompletion();

    return ::grpc::Status::OK;
}

// MSSpeechSessinHandler callbacks
void SyncRpcRequestHandler::speechStartdetected(ms_speech_startdetected_message_t *message)
{
}

void SyncRpcRequestHandler::speechEnddetected(ms_speech_enddetected_message_t *message)
{
}

void SyncRpcRequestHandler::speechHypothesis(ms_speech_hypothesis_message_t *message)
{
}

void SyncRpcRequestHandler::speechFragment(ms_speech_fragment_message_t *message)
{
}

void SyncRpcRequestHandler::speechResult(ms_speech_result_message_t *message)
{
        // TODO: handle speech status
    ::google::cloud::speech::v1::StreamingRecognizeResponse response;
    switch(message->status) {
        case MS_SPEECH_RECO_SUCCESS:
        {
            auto *result = this->response->add_results();
            for (int i=0; i<message->num_phrase_results; i++) {
                auto phraseResult = &message->phrase_results[i];
                auto alt = result->add_alternatives();
                alt->set_transcript(phraseResult->display);
                alt->set_confidence(phraseResult->confidence);
            }
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
}

