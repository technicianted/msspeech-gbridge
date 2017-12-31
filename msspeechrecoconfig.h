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

#ifndef __MSSPEECHRECOCONFIG_H__
#define __MSSPEECHRECOCONFIG_H__

#include <string>

#include "google/cloud/speech/v1/cloud_speech.grpc.pb.h"


enum class MSSpeechRecognitionMode {
    INTERACTIVE,
    DICTATION,
    CONVERSATIONAL
};

enum class MSSpeechProfanityFilterMode {
    NONE,
    MASK,
    REMOVE
};

class MSSpeechRecoConfig
{
public:
    MSSpeechRecoConfig();

    ::grpc::Status load(const ::google::cloud::speech::v1::StreamingRecognitionConfig &googleSpeechConfig);
    std::string getMSSpeechUrl() const;

    MSSpeechRecognitionMode recognitionMode() const { return _recognitionMode; }
    MSSpeechProfanityFilterMode profanityFilterMode() const { return _profanityFilterMode; }
    std::string language() const { return _language; }
    bool enableIntermediateResults() const { return _enableIntermediateResults; }
 
private:
    MSSpeechRecognitionMode _recognitionMode;
    MSSpeechProfanityFilterMode _profanityFilterMode;
    std::string _language;
    bool _enableIntermediateResults;
};

#endif
