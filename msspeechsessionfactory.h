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

#ifndef __MSSPEECHSESSIONFACTORY_H__
#define __MSSPEECHSESSIONFACTORY_H__

#include <map>
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <ms_speech/ms_speech.h>
#include <ctime>

class MSSpeechSession;
class RecognitionSession;

/**
 * \brief Factory class to create and manage the lifetime of MSSpeechSession.
 * 
 * This class creates and and monitors instances of MSSpeechSession as they are used.
 * If a connection is closed, the corresponding MSSpeechSession object is removed. It
 * maintains a list per request URL since it encodes the session parameters. Instances
 * are then reused as long as their connections are open.
 */
class MSSpeechSessionFactory
{
public:
    /**
     * \brief Construct a new session factory.
     * 
     * \param msspeechSubscriptionKey Microsoft Speech API subscription key to be used.
     * \param maxSession Maximum number of simultaneos sessions.
     */
    MSSpeechSessionFactory(const std::string &msspeechSubscriptionKey, int maxSessions);
    ~MSSpeechSessionFactory();

    /**
     * \brief Obtain a new session instance for a given URL.
     * 
     * This call will block until a session is available. If maxSessions was not
     * exceeded, a new session is created and its connection established. If no 
     * session available and maxSession reached, the call blocks until one is avaialble.
     * If there is a free session then it is returned immediately.
     * 
     * \param uri Full request URL.
     * \return a speech session object.
     */ 
    MSSpeechSession * getSession(const std::string &uri);
    /**
     * \brief Returns back a session after usage.
     * 
     * \param session session object
     */
    void putSession(MSSpeechSession *session);

    /**
     * \brief Start object main run loop. Must be called before calling get().
     */
    void start();
    /**
     * \brief Stop object main run loop.
     */
    void stop();

private:
    std::mutex lock;
    std::condition_variable sessionCondition;

    std::string msspeechSubscriptionKey;

    std::map<std::string, std::set<RecognitionSession *>> availableSessionsByUri;
    std::set<RecognitionSession *> pendingSessions;
    std::set<RecognitionSession *> busySesssions;
    std::set<RecognitionSession *> sessions;
    std::map<MSSpeechSession *, RecognitionSession *> sessionsBySession;
    ms_speech_context_t msspeechContext;
    std::thread runloopThread;
    bool terminateRunloop;
    bool runloopStarted;
    int maxSessions;

    RecognitionSession * createNewSession(const std::string &uri);
    void processPendingSessions();

    static void log(ms_speech_log_level_t level, const std::string &message);

 	static void msspeech_connection_established(ms_speech_connection_t connection, void *user_data);
	static void msspeech_connection_error(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data);
	static void msspeech_connection_closed(ms_speech_connection_t connection, void *user_data);
	static void msspeech_client_ready(ms_speech_connection_t connection, void *user_data);
	static const char * msspeech_provide_authentication_header(ms_speech_connection_t connection, void *user_data, size_t max_len);
	static void msspeech_message_overlay(ms_speech_connection_t connection, ms_speech_user_message_type type, json_object *body, void *user_data);
	static void msspeech_speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data);
	static void msspeech_speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data);
	static void msspeech_speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data);
	static void msspeech_speech_fragment(ms_speech_connection_t connection, ms_speech_fragment_message_t *message, void *user_data);
	static void msspeech_speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data);
	static void msspeech_turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data);
	static void msspeech_turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data);
	static void msspeech_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message);

    static void msspeech_global_log(ms_speech_log_level_t level, const char *message);
};

class RecognitionSession
{
public:
RecognitionSession();

    MSSpeechSessionFactory *factory;

    std::string uri;
    ms_speech_connection_t connection;
    MSSpeechSession *session;

    bool checkedOut;
    std::time_t checkoutTime;
};

#endif
