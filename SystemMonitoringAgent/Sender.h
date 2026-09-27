#pragma once

#include <string>

class Sender
{
private:
    std::string host;
    int port;
    std::string path;

public:
    Sender(
        const std::string& host,
        int port,
        const std::string& path
    );

    bool send(const std::string& json);

    bool isAvailable();
};