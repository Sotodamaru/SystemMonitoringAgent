#pragma once
#include "Metrics.h"
#include <cstddef>
#include <mutex>
#include <deque>
#include <vector>

class Buffer
{
private:
    static constexpr std::size_t MAX_RECORDS = 100;
    std::size_t storedRecords = 0;

    std::deque<Metric> metrics;
    mutable std::mutex mutex;
public:
    bool add(const Metric& metric);
    std::size_t size() const;
    void setStoredRecords(std::size_t count);
    std::size_t totalSize() const;
    std::vector<Metric> getBatch(std::size_t count) const;
    void removeFirst(std::size_t count);
};