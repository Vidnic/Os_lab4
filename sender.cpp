#include <windows.h>
#include <iostream>
#include <fstream>

#include "common.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    std::string fileName = argv[1];

    HANDLE mutex = OpenMutexA(
        MUTEX_ALL_ACCESS,
        FALSE,
        "FileMutex"
    );

    HANDLE emptySemaphore = OpenSemaphoreA(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        "EmptySemaphore"
    );

    HANDLE fullSemaphore = OpenSemaphoreA(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        "FullSemaphore"
    );

    HANDLE readyEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "ReadyEvent"
    );

    WaitForSingleObject(readyEvent, INFINITE);

    int writeIndex = 0;

    while (true)
    {
        int command;

        std::cout << "\n1 - send message\n";
        std::cout << "0 - exit\n";
        std::cin >> command;

        if (command == 0)
        {
            break;
        }

        WaitForSingleObject(emptySemaphore, INFINITE);

        WaitForSingleObject(mutex, INFINITE);

        Message message{};

        std::cout << "Input message: ";
        std::cin >> message.text;

        std::fstream file(
            fileName,
            std::ios::binary | std::ios::in | std::ios::out
        );

        file.seekp(
            writeIndex * sizeof(Message),
            std::ios::beg
        );

        file.write(
            reinterpret_cast<char*>(&message),
            sizeof(Message)
        );

        file.close();

        writeIndex++;

        ReleaseMutex(mutex);

        ReleaseSemaphore(fullSemaphore, 1, nullptr);
    }

    CloseHandle(mutex);
    CloseHandle(emptySemaphore);
    CloseHandle(fullSemaphore);
    CloseHandle(readyEvent);

    return 0;
}