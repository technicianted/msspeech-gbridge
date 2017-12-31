#include <string.h>
#include <assert>

#include "audioqueue.h"

AudioQueue::AudioQueue()
{
    this->dataSize = 0;
    this->ended = false;
    this->terminating = false;
    this->busy = false;
    this->requestedSize = 0;
    this->processedBytes = 0;
}

AudioQueue::~AudioQueue()
{
    std::lock_guard<std::mutex> lock(this->queueLock);

    while(this->audioQueue.size() > 0) {
        auto e = this->audioQueue.front();
        delete [] e->buffer;
        delete e;
        this->audioQueue.pop();
    }
 }

int AudioQueue::put(const char *buffer, int length)
{
    std::lock_guard<std::mutex> lock(this->queueLock);
 
    assert(!this->ended && !this->terminating);

    auto entry = new AudioQueueEntry();
    entry->buffer = new char[length];
    memcpy(entry->buffer, buffer, length);
    entry->offset = 0;
    entry->length = length;

    this->dataSize += length;
    this->audioQueue.push(entry);

    if (this->requestedSize > 0 && this->dataSize >= this->requestedSize)
        this->queueCondition.notify_one();

    return length;
}

int AudioQueue::get(char *buffer, int requestedLength)
{
    std::lock_guard<std::mutex> lock(this->queueLock);

    if (this->dataSize == 0) {
        if (this->ended)
            return -1;
        else
            return 0;
    }
    else {
        int size = std::min(requestedLength, this->dataSize);
        this->getSize(buffer, size);
        return size;
    }
}

int AudioQueue::blockingGet(char *buffer, int requestedLength)
{
    std::unique_lock<std::mutex> lock(this->queueLock);

    assert(!this->busy);

    this->busy = true;
    this->requestedSize = requestedLength;

    while(this->dataSize < requestedLength && !this->terminating && !this->ended) {
        this->queueCondition.wait(lock);
    }

    if (this->terminating) {
        return 0;
    }

    if (this->ended) {
        if (this->dataSize == 0) {
            return 0;
        }
        else {
            if (this->dataSize < requestedLength)
                requestedLength = this->dataSize;
         }
    }

    this->getSize(buffer, requestedLength);
    this->busy = false;
    this->requestedSize = 0;

    return requestedLength;
}

void AudioQueue::markEnd()
{
    this->ended = true;
    this->queueCondition.notify_all();
}

void AudioQueue::terminate()
{
    this->terminating = true;
    this->queueCondition.notify_all();
}

void AudioQueue::getSize(char *buffer, int requestedLength)
{
    int remaining = requestedLength;
    int offset = 0;
    while (remaining > 0) {
        int copiedLength = 0;
        auto entry = this->audioQueue.front();
        if (entry->length > remaining) {
            memcpy(buffer + offset, entry->buffer + entry->offset, remaining);
            entry->offset += remaining;
            entry->length -= remaining;
            this->dataSize -= remaining;

            copiedLength = remaining;
        }
        else {
            memcpy(buffer + offset, entry->buffer + entry->offset, entry->length);
            auto size = entry->length;
            delete [] entry->buffer;
            delete entry;
            this->audioQueue.pop();
            this->dataSize -= size;
        
            copiedLength = size;
        }

        offset += copiedLength;
        remaining -= copiedLength;
    }

    this->processedBytes += requestedLength;
}
