#include "Buffer.h"

bool Buffer::add(const Metric& metric)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (metrics.size() + storedRecords >= MAX_RECORDS)
    {
        return false;
    }

    metrics.push_back(metric);
    return true;
}

std::size_t Buffer::size() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return metrics.size();
}

void Buffer::setStoredRecords(std::size_t count)
{
    std::lock_guard<std::mutex> lock(mutex);

    storedRecords = count > MAX_RECORDS
        ? MAX_RECORDS
        : count;
}

std::size_t Buffer::totalSize() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return metrics.size() + storedRecords;
}

std::vector<Metric> Buffer::getBatch(std::size_t count) const
{
    std::lock_guard<std::mutex> lock(mutex);

    if (count > metrics.size())
    {
        count = metrics.size();
    }

    std::vector<Metric> batch;
    batch.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        batch.push_back(metrics[i]);
    }

    return batch;
}

void Buffer::removeFirst(std::size_t count)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (count > metrics.size())
    {
        count = metrics.size();
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        metrics.pop_front();
    }
}