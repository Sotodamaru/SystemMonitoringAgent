#include "Collector.h"

#include <Windows.h>

#include <chrono>
#include <ctime>
#include <thread>
#include <iostream>

#pragma comment(lib, "User32.lib")

namespace
{
    std::string wideToUtf8(const std::wstring& text)
    {
        if (text.empty())
        {
            return {};
        }

        int size = WideCharToMultiByte(
            CP_UTF8,
            0,
            text.c_str(),
            static_cast<int>(text.size()),
            nullptr,
            0,
            nullptr,
            nullptr
        );

        if (size <= 0)
        {
            return {};
        }

        std::string result(size, '\0');

        WideCharToMultiByte(
            CP_UTF8,
            0,
            text.c_str(),
            static_cast<int>(text.size()),
            &result[0],
            size,
            nullptr,
            nullptr
        );

        return result;
    }
}

Collector::Collector(Buffer& buffer)
    : buffer(buffer)
{
}

void Collector::start()
{
    if (running)
    {
        return;
    }

    running = true;
    worker = std::thread(&Collector::collectLoop, this);
}

void Collector::stop()
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
}

void Collector::collectLoop()
{
    while (running)
    {
        Metric metric = collectMetric();

        if (buffer.add(metric))
        {
            std::wcout << L"[Сборщик] Запись добавлена в буфер\n";
        }
        else
        {
            std::wcout << L"[Сборщик] Буфер достиг лимита\n";
        }

        std::unique_lock<std::mutex> lock(stopMutex);

        stopCondition.wait_for(
            lock,
            std::chrono::seconds(5),
            [this]()
            {
                return !running;
            }
        );
    }
}

Metric Collector::collectMetric()
{
    Metric metric{};

    metric.timestamp = std::time(nullptr);

    HWND foregroundWindow = GetForegroundWindow();

    if (foregroundWindow == nullptr)
    {
        return metric;
    }

    wchar_t windowTitle[512]{};

    GetWindowTextW(
        foregroundWindow,
        windowTitle,
        512
    );

    metric.windowTitle = wideToUtf8(windowTitle);

    DWORD processId = 0;

    GetWindowThreadProcessId(
        foregroundWindow,
        &processId
    );

    HANDLE processHandle = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        processId
    );

    if (processHandle != nullptr)
    {
        wchar_t processPath[MAX_PATH]{};
        DWORD pathSize = MAX_PATH;

        if (QueryFullProcessImageNameW(
            processHandle,
            0,
            processPath,
            &pathSize))
        {
            const wchar_t* fileName = processPath;

            for (DWORD i = 0; i < pathSize; ++i)
            {
                if (processPath[i] == L'\\')
                {
                    fileName = &processPath[i + 1];
                }
            }

            metric.processName = wideToUtf8(fileName);
        }

        CloseHandle(processHandle);
    }

    LASTINPUTINFO lastInputInfo{};
    lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

    if (GetLastInputInfo(&lastInputInfo))
    {
        DWORD currentTick = GetTickCount();
        DWORD idleTime = currentTick - lastInputInfo.dwTime;

        metric.userActive = idleTime <= 5000;
    }

    return metric;
}