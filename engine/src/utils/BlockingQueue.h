#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class BlockingQueue
{
public:
    void push(T& item)
    {
        std::unique_lock lock{mut};
        blockingQueue.push(item);
        lock.unlock();
        cond.notify_one();
    }

    T pop()
    {
        // TODO: Specify order side in template
        std::unique_lock lock{mut};
        cond.wait(lock, [this]{ return !blockingQueue.empty(); });
        T t = std::move(blockingQueue.front());
        blockingQueue.pop();
        lock.unlock();
        return t;
    }

    bool empty()
    {
        return blockingQueue.empty();
    }

private:
    std::queue<T> blockingQueue;
    std::condition_variable cond;
    mutable std::mutex mut;
};

#endif  // BLOCKING_QUEUE_H