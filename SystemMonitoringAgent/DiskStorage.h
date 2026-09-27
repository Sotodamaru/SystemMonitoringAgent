#pragma once

#include <string>
#include <vector>

class DiskStorage
{
private:
    std::string directory;

    int getNextPacketNumber() const;

public:
    explicit DiskStorage(const std::string& directory);

    bool save(const std::string& packet);

    std::vector<std::string> getPackets() const;

    std::size_t getStoredRecordCount() const;

    std::string readPacket(const std::string& filePath) const;

    bool removePacket(const std::string& filePath) const;
};