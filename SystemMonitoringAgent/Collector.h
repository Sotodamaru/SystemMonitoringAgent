#pragma once

#include "Buffer.h"
#include <atomic>
#include <thread>
#include <condition_variable>

class Collector
{
private:
    Buffer& buffer;
    std::atomic<bool> running{ false };
    std::thread worker;

    std::condition_variable stopCondition;
    std::mutex stopMutex;

    void collectLoop();
    Metric collectMetric();

public:
    explicit Collector(Buffer& buffer);

    void start();
    void stop();
};