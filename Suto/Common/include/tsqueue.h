#ifndef TSQUEUE_H
#define TSQUEUE_H

#pragma once
#include <mutex>
#include <deque>

namespace sto 
{
    template<typename T>
    struct tsqueue
    {
    public:
        tsqueue(const tsqueue&) = delete;
        tsqueue& operator=(const tsqueue&) = delete;
        tsqueue() = default;

        ~tsqueue() { deqQueue.clear();}

    public:
        const T front()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            return deqQueue.front();
        }

        const T back()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            return deqQueue.back();
        }

        void push_back(const T& item)
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.emplace_back(std::move(item));
        }

        void push_front(const T& item)
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.emplace_front(std::move(item));
        }

        // clears queue
        void empty()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.clear();
        }

        // returns size of queue
        size_t count()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.size();
        }

        // checks if empty
        bool empty() const
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.empty();
        }

        T pop_front()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            auto t = std::move(deqQueue.front());
            deqQueue.pop_front();
            return t;
        }

        T pop_back()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            auto t = std::move(deqQueue.front());
            deqQueue.pop_back();
            return t;
        }
    protected:
        std::mutex muxQueue;
        std::deque<T> deqQueue;
    };
}

#endif