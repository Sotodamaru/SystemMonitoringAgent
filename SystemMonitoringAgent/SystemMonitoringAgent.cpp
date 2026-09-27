#include "Buffer.h"
#include "Collector.h"
#include "Packer.h"
#include "DiskStorage.h"
#include "Sender.h"

#include <Windows.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>

#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

std::atomic<bool> shutdownRequested{ false };

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    Buffer buffer;
    Collector collector(buffer);
    Sender sender("localhost", 8080, "/");
    DiskStorage storage("storage");
    buffer.setStoredRecords(storage.getStoredRecordCount());
    Packer packer(buffer, sender, storage, "DESKTOP-MIDDLE-C");

    packer.start();
    collector.start();

    while (!shutdownRequested)
    {
        if (_kbhit())
        {
            int key = _getch();

            if (key == 27)
            {
                shutdownRequested = true;
            }
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }

    collector.stop();
    packer.stop();

    return 0;
}