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

#ifndef __AUDIOQUEUE_H__
#define __AUDIOQUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>

class AudioQueueEntry;

/**
 * \brief Implementation of produce-consumer thraded blocking audio queue.
 * 
 * Impelementation of audio queue capable of blocking get operations
 * until requrested size becomes available. It also supports produces
 * and consumers calling from different threads.
 * It also supports marking the queue as completed and notifying consumers.
 */
class AudioQueue
{
public:
    /**
     * \brief Construct a new instance of AudioQueue.
     */
    AudioQueue();
    ~AudioQueue();

    /**
     *  \brief Non-blocking queue the given bytes.
     * 
     * \param buffer Byte input to queue.
     * \param length Byte input size.
     */
    int put(const char *buffer, int length);
    /**
     * \brief Non-blocking dequeue.
     * 
     * This method attempts to deuque a maximum of given byte size 
     * without blocking.
     * 
     * \param buffer pointer to buffer to be filled.
     * \param requestedLength maximum bytes to dequeue.
     * \return Dequeued size, 0 if no bytes available, -1 if queue ended
     * and drained, or terminated.
     */
    int get(char *buffer, int requestedLength);
    /**
     * \brief Blocking dequeue.
     * 
     * This method blocks until requestedLength is available
     * or queue has ended.
     * 
     * \param buffer pointer to buffer to be filled.
     * \param requestedLength byte size to provide.
     * \return requestedLength if available, or less if queue has ended
     * and drained, or terminated.
     */
    int blockingGet(char *buffer, int requestedLength);
    /**
     * \brief Mark queue as ended and notify consumers.
     */
    void markEnd();
    /**
     * \brief Set queue as terminated notifying consumers.
     * 
     * Terminating a queue informs consumers immediately even 
     * if queue was not drawined.
     */
    void terminate();

private:
    std::mutex queueLock;
    std::condition_variable queueCondition;
    int dataSize;
    bool busy;
    bool ended;
    bool terminating;
    int requestedSize;
    int processedBytes;

    std::queue<AudioQueueEntry *> audioQueue;

    void getSize(char *buffer, int requestedLength);
};

class AudioQueueEntry
{
public:
    char *buffer;
    int offset;
    int length;
};

#endif
