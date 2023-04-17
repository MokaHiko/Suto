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

            // Enabled on passive run
            std::scoped_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();

        }

        void push_front(const T& item)
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            deqQueue.emplace_front(std::move(item));

            // Enabled on passive run
            std::scoped_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }

        // clears queue
        void clear()
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
        bool empty()
        {
            std::scoped_lock<std::mutex> lock(muxQueue);
            return deqQueue.empty();
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


        void wait()
        {
            // Enabled on passive run
            while(empty())
            {
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul);
            }
        }
    protected:
        std::mutex muxQueue;
        std::deque<T> deqQueue;

        std::condition_variable cvBlocking;
        std::mutex muxBlocking;
    };
}

#endif