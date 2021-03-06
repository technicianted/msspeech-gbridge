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

#ifndef __MSSPEECHSESSIONHANDLER_H__
#define __MSSPEECHSESSIONHANDLER_H__

#include <ms_speech/ms_speech.h>

/**
 * \brief Definition of a speech session handler. Used by 
 * MSSpeechSession class.
 */
class MSSpeechSessionHandler
{
public:
    /**
     * \brief Called when speech.start received.
     * 
     * \param message message payload.
     */
 	virtual void speechStartdetected(ms_speech_startdetected_message_t *message) {};
    /**
     * \brief Called when speech.end received.
     * 
     * \param message message payload.
     */
	virtual void speechEnddetected(ms_speech_enddetected_message_t *message) {};
    /**
     * \brief Called when speech.hypothesis received.
     * 
     * \param message message payload.
     */
    virtual void speechHypothesis(ms_speech_hypothesis_message_t *message) = 0;
    /**
     * \brief Called when speech.fragment received.
     * 
     * \param message message payload.
     */
    virtual void speechFragment(ms_speech_fragment_message_t *message) = 0;
    /**
     * \brief Called when speech.phrase received.
     * 
     * \param message message payload.
     */
    virtual void speechResult(ms_speech_result_message_t *message) = 0;
};

#endif
