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

class AudioQueue
{
public:
    AudioQueue();
    ~AudioQueue();

    int put(const char *buffer, int length);
    int get(char *buffer, int requestedLength);
    int blockingGet(char *buffer, int requestedLength);
    void markEnd();
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
