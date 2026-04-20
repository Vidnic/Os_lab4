#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "common.h"

int main()
{
    std::string fileName;
    int recordsCount;
    int senderCount;

    std::cout << "Input binary file name: ";
    std::cin >> fileName;

    std::cout << "Input records count: ";
    std::cin >> recordsCount;

    std::ofstream file(fileName, std::ios::binary | std::ios::trunc);

    Message emptyMessage{};

    for (int i = 0; i < recordsCount; ++i)
    {
        file.write(reinterpret_cast<char*>(&emptyMessage), sizeof(Message));
    }

    file.close();

    HANDLE mutex = CreateMutexA(
        nullptr,
        FALSE,
        "FileMutex"
    );

    HANDLE emptySemaphore = CreateSemaphoreA(
        nullptr,
        recordsCount,
        recordsCount,
        "EmptySemaphore"
    );

    HANDLE fullSemaphore = CreateSemaphoreA(
        nullptr,
        0,
        recordsCount,
        "FullSemaphore"
    );

    HANDLE readyEvent = CreateEventA(
        nullptr,
        TRUE,
        FALSE,
        "ReadyEvent"
    );

    int writeIndex = 0;
    int readIndex = 0;

    std::cout << "Input sender count: ";
    std::cin >> senderCount;

    std::vector<PROCESS_INFORMATION> senders(senderCount);

    for (int i = 0; i < senderCount; ++i)
    {
        STARTUPINFOA si{};
        si.cb = sizeof(STARTUPINFOA);

        PROCESS_INFORMATION pi{};

        char commandLine[256];

        sprintf_s(
            commandLine,
            "sender.exe %s",
            fileName.c_str()
        );

        CreateProcessA(
            nullptr,
            commandLine,
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        senders[i] = pi;
    }

    SetEvent(readyEvent);

    while (true)
    {
        int command;

        std::cout << "\n1 - read message\n";
        std::cout << "0 - exit\n";
        std::cin >> command;

        if (command == 0)
        {
            break;
        }

        WaitForSingleObject(fullSemaphore, INFINITE);

        WaitForSingleObject(mutex, INFINITE);

        std::fstream binaryFile(
            fileName,
            std::ios::binary | std::ios::in | std::ios::out
        );

        binaryFile.seekg(
            readIndex * sizeof(Message),
            std::ios::beg
        );

        Message message{};

        binaryFile.read(
            reinterpret_cast<char*>(&message),
            sizeof(Message)
        );

        Message empty{};

        binaryFile.seekp(
            readIndex * sizeof(Message),
            std::ios::beg
        );

        binaryFile.write(
            reinterpret_cast<char*>(&empty),
            sizeof(Message)
        );

        binaryFile.close();

        std::cout << "Message: " << message.text << "\n";

        readIndex = (readIndex + 1) % recordsCount;

        ReleaseMutex(mutex);

        ReleaseSemaphore(emptySemaphore, 1, nullptr);
    }

    for (int i = 0; i < senderCount; ++i)
    {
        WaitForSingleObject(
            senders[i].hProcess,
            INFINITE
        );

        CloseHandle(senders[i].hProcess);
        CloseHandle(senders[i].hThread);
    }

    CloseHandle(mutex);
    CloseHandle(emptySemaphore);
    CloseHandle(fullSemaphore);
    CloseHandle(readyEvent);

    return 0;
}