#include "Packer.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <iostream>

Packer::Packer(Buffer& buffer, Sender& sender, DiskStorage& storage, const std::string& agentId)
    : buffer(buffer),
    sender(sender),
    storage(storage),
    agentId(agentId),
    lastPackTime(std::chrono::steady_clock::now())
{
}

void Packer::start()
{
    if (running)
    {
        return;
    }

    running = true;
    worker = std::thread(&Packer::packLoop, this);
}

void Packer::stop()
{
    if (!running)
    {
        return;
    }

    running = false;
    stopCondition.notify_one();

    if (worker.joinable())
    {
        worker.join();
    }

    flushBuffer();
}

std::string Packer::formatTime(time_t timestamp) const
{
    std::tm localTime{};

    localtime_s(&localTime, &timestamp);

    std::ostringstream stream;

    stream << std::put_time(
        &localTime,
        "%Y-%m-%d %H:%M:%S"
    );

    return stream.str();
}

std::string Packer::escapeJson(const std::string& value) const
{
    std::string result;

    for (char character : value)
    {
        switch (character)
        {
        case '"':
            result += "\\\"";
            break;

        case '\\':
            result += "\\\\";
            break;

        case '\n':
            result += "\\n";
            break;

        case '\r':
            result += "\\r";
            break;

        case '\t':
            result += "\\t";
            break;

        default:
            result += character;
            break;
        }
    }

    return result;
}

void Packer::packLoop()
{
    while (running)
    {
        std::unique_lock<std::mutex> lock(stopMutex);

        stopCondition.wait_for(
            lock,
            std::chrono::seconds(5),
            [this]()
            {
                return !running;
            }
        );

        if (!running)
        {
            break;
        }

        bool available = sender.isAvailable();

        if (available && !serverAvailable)
        {
            serverAvailable = true;

            sendStoredPackets();
        }
        else if (!available)
        {
            serverAvailable = false;
        }

        if (serverAvailable)
        {
            auto now = std::chrono::steady_clock::now();

            auto elapsed =
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - lastPackTime
                );

            if (elapsed.count() >= 30)
            {
                if (buffer.size() > 0)
                {
                    pack(buffer.size());
                }

                lastPackTime = now;
            }
        }
        else
        {
            if (buffer.size() >= 10)
            {
                pack(10);
            }
        }
    }
}

void Packer::sendStoredPackets()
{
    std::vector<std::string> packets =
        storage.getPackets();

    for (const std::string& filePath : packets)
    {
        std::string packet =
            storage.readPacket(filePath);

        if (packet.empty())
        {
            continue;
        }

        if (!sender.send(packet))
        {
            serverAvailable = false;
            break;
        }

        if (storage.removePacket(filePath))
        {
            buffer.setStoredRecords(storage.getStoredRecordCount());
            std::wcout << L"[Упаковщик] Сохранённый пакет отправлен: записи освобождены\n";
        }
    }

    buffer.setStoredRecords(storage.getStoredRecordCount());
}

void Packer::flushBuffer()
{
    std::size_t count = buffer.size();

    if (count == 0)
    {
        return;
    }

    pack(count);
}

std::string Packer::pack(std::size_t count)
{
    std::vector<Metric> metrics = buffer.getBatch(count);

    if (metrics.empty())
    {
        return {};
    }

    std::time_t packetTimestamp = std::time(nullptr);

    std::ostringstream json;

    json << "{";

    json << "\"agent_id\":\""
        << escapeJson(agentId)
        << "\",";

    json << "\"timestamp\":"
        << packetTimestamp
        << ",";

    json << "\"payload\":[";

    for (std::size_t i = 0; i < metrics.size(); ++i)
    {
        const Metric& metric = metrics[i];

        if (i > 0)
        {
            json << ",";
        }

        json << "{";

        json << "\"time\":\""
            << formatTime(metric.timestamp)
            << "\",";

        json << "\"process_name\":\""
            << escapeJson(metric.processName)
            << "\",";

        json << "\"window_title\":\""
            << escapeJson(metric.windowTitle)
            << "\",";

        json << "\"user_active\":"
            << (metric.userActive ? "true" : "false");

        json << "}";
    }

    json << "]";
    json << "}";

    std::string packet = json.str();

    std::wcout << L"[Упаковщик] Пакет сформирован: " << metrics.size() << L" записей\n";

    if (serverAvailable)
    {
        if (sender.send(packet))
        {
            buffer.removeFirst(metrics.size());

            std::wcout << L"[Упаковщик] Пакет отправлен: " << metrics.size() << L" записей\n";

            return packet;
        }

        std::wcout << L"[Упаковщик] Не удалось отправить пакет\n";
        serverAvailable = false;
    }

    std::size_t storedRecords = storage.getStoredRecordCount();
    buffer.setStoredRecords(storedRecords + metrics.size());

    if (storage.save(packet))
    {
        buffer.removeFirst(metrics.size());

        buffer.setStoredRecords(storage.getStoredRecordCount());

        std::wcout << L"[Упаковщик] Пакет сохранён на диск: " << metrics.size() << L" записей\n";
    }
    else
    {
        buffer.setStoredRecords(storedRecords);
    }

    return packet;
}