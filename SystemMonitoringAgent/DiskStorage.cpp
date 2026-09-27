#include "DiskStorage.h"

#include <Windows.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstddef>

DiskStorage::DiskStorage(const std::string& directory)
    : directory(directory)
{
    CreateDirectoryA(directory.c_str(), nullptr);
}

int DiskStorage::getNextPacketNumber() const
{
    int maxNumber = 0;

    WIN32_FIND_DATAA findData{};

    std::string searchPath = directory + "\\packet_*.json";

    HANDLE findHandle = FindFirstFileA(
        searchPath.c_str(),
        &findData
    );

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    do
    {
        std::string fileName = findData.cFileName;

        std::size_t start = fileName.find("packet_");
        std::size_t end = fileName.find(".json");

        if (start != std::string::npos &&
            end != std::string::npos)
        {
            std::string numberString =
                fileName.substr(
                    start + 7,
                    end - (start + 7)
                );

            try
            {
                int number = std::stoi(numberString);

                if (number > maxNumber)
                {
                    maxNumber = number;
                }
            }
            catch (...)
            {
                // Игнорируем файлы с некорректным номером.
            }
        }

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);

    return maxNumber + 1;
}

bool DiskStorage::save(const std::string& packet)
{
    CreateDirectoryA(directory.c_str(), nullptr);

    int packetNumber = getNextPacketNumber();

    std::ostringstream fileName;

    fileName << directory
        << "\\packet_";

    if (packetNumber < 10)
    {
        fileName << "00000";
    }
    else if (packetNumber < 100)
    {
        fileName << "0000";
    }
    else if (packetNumber < 1000)
    {
        fileName << "000";
    }
    else if (packetNumber < 10000)
    {
        fileName << "00";
    }
    else if (packetNumber < 100000)
    {
        fileName << "0";
    }

    fileName << packetNumber << ".json";

    std::ofstream file(fileName.str());

    if (!file.is_open())
    {
        return false;
    }

    file << packet;

    return file.good();
}

std::vector<std::string> DiskStorage::getPackets() const
{
    std::vector<std::string> packets;

    WIN32_FIND_DATAA findData{};

    std::string searchPath =
        directory + "\\packet_*.json";

    HANDLE findHandle = FindFirstFileA(
        searchPath.c_str(),
        &findData
    );

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return packets;
    }

    do
    {
        if (!(findData.dwFileAttributes &
            FILE_ATTRIBUTE_DIRECTORY))
        {
            packets.push_back(
                directory + "\\" + findData.cFileName
            );
        }

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);

    std::sort(
        packets.begin(),
        packets.end()
    );

    return packets;
}

std::size_t DiskStorage::getStoredRecordCount() const
{
    std::size_t count = 0;

    std::vector<std::string> packets = getPackets();

    for (const std::string& filePath : packets)
    {
        std::string packet = readPacket(filePath);

        if (packet.empty())
        {
            continue;
        }

        const std::string marker = "\"time\":\"";

        std::size_t position = 0;

        while ((position = packet.find(marker, position))
            != std::string::npos)
        {
            ++count;
            position += marker.size();
        }
    }

    return count;
}

std::string DiskStorage::readPacket(
    const std::string& filePath) const
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        return {};
    }

    std::ostringstream content;
    content << file.rdbuf();

    return content.str();
}

bool DiskStorage::removePacket(
    const std::string& filePath) const
{
    return DeleteFileA(filePath.c_str());
}