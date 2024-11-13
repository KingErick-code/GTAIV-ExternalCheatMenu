#include "Memory.h"
#include <TlHelp32.h>
#include <tchar.h>
#include <iostream>

Memory::Memory()
{
    // Find the game window
    gameWindow = FindWindow(NULL, "GTAIV");
    if (gameWindow == NULL)
    {
        MessageBoxA(NULL, "Ensure the game is running!", "Couldn't find game window", MB_OK);
        return;
    }

    // Get the process ID of the game
    GetWindowThreadProcessId(gameWindow, &procID);

    // Open the process
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (processHandle == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Failed to open process", "Error", MB_OK);
        return;
    }

    // Retrieve the base address of the game module (GTAIV.exe)
    gameBaseAddress = GetModuleBaseAddress("GTAIV.exe", procID);
    if (gameBaseAddress == 0)
    {
        MessageBoxA(NULL, "Failed to get base address", "Error", MB_OK);
    }

    Health = new Address{ 0xEBD0D4, {0xE9C}, *this };
    Armor = new Address{ 0xEBD0D4, {0xB84}, *this };
    Money = new Address{ 0xDA8808, {0x5C4}, *this };
    WantedLevel = new Address{ 0xDA8808, {0x3F4}, *this };
}

DWORD Memory::GetModuleBaseAddress(const char* moduleName, DWORD processID)
{
    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Failed to create snapshot", "Error", MB_OK);
        return 0;
    }

    MODULEENTRY32 moduleEntry32 = { 0 };
    moduleEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry32))
    {
        do
        {
            if (strcmp(moduleEntry32.szModule, moduleName) == 0)
            {
                dwModuleBaseAddress = (DWORD)moduleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &moduleEntry32));
    }

    CloseHandle(hSnapshot);

    return dwModuleBaseAddress;
}

Memory::~Memory()
{
    // Close the process handle to avoid memory leak
    if (processHandle != NULL)
    {
        CloseHandle(processHandle);
    }

    if (Health)
        delete Health;
    if (Armor)
        delete Armor;
    if (Money)
        delete Money;
    if (WantedLevel)
        delete WantedLevel;
}
