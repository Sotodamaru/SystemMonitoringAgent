cmake_minimum_required(VERSION 3.20)

project(SystemMonitoringAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SystemMonitoringAgent
    SystemMonitoringAgent.cpp
    Buffer.cpp
    Collector.cpp
    DiskStorage.cpp
    Packer.cpp
    Sender.cpp
)

target_link_libraries(SystemMonitoringAgent
    PRIVATE
    user32
    winhttp
)