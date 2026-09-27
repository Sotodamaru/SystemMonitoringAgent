#pragma once

#include "Buffer.h"
#include "Sender.h"
#include "DiskStorage.h"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>

class Packer
{
private:
    Buffer& buffer;
    DiskStorage& storage;
    Sender& sender;
    std::string agentId;

    std::atomic<bool> running{ false };
    std::thread worker;

    bool serverAvailable = false;
    std::chrono::steady_clock::time_point lastPackTime;

    std::condition_variable stopCondition;
    std::mutex stopMutex;

    std::string formatTime(time_t timestamp) const;
    std::string escapeJson(const std::string& value) const;

    void packLoop();
    void sendStoredPackets();
    void flushBuffer();

public:
    Packer(Buffer& buffer, Sender& sender, DiskStorage& storage, const std::string& agentId);

    void start();
    void stop();

    std::string pack(std::size_t count);
};