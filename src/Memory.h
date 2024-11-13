#pragma once

#include <Windows.h>
#include <cstddef>
#include <vector>

class Memory
{
private:
	HANDLE processHandle = NULL;
	DWORD procID = NULL;
	DWORD gameBaseAddress = NULL;
	HWND gameWindow = FindWindow(NULL, "GTAIV");

    struct Address
    {
    public:
        DWORD OffsetToBaseAddress;
        std::vector<DWORD> offsets;
        DWORD BaseAddress = NULL;
        DWORD pointsAddress = NULL;

        Address(DWORD offsetToBaseAddress, const std::vector<DWORD>& offsets, Memory& memory)
            : OffsetToBaseAddress(offsetToBaseAddress), offsets(offsets)
        {
            if (!ReadProcessMemory(memory.processHandle, (LPVOID)(memory.gameBaseAddress + OffsetToBaseAddress), &BaseAddress, sizeof(BaseAddress), NULL))
            {
                MessageBoxA(NULL, "Failed to read base address", "Error", MB_OK);
                return;
            }

            pointsAddress = BaseAddress;
            for (int i = 0; i < offsets.size() - 1; i++)
            {
                if (!ReadProcessMemory(memory.processHandle, (LPVOID)(pointsAddress + offsets[i]), &pointsAddress, sizeof(pointsAddress), NULL))
                {
                    MessageBoxA(NULL, "Failed to read memory with offsets", "Error", MB_OK);
                    return;
                }
            }

            pointsAddress += offsets.at(offsets.size() - 1);
        }
    };

public:
    Address* Health;
    Address* Armor;
    Address* Money;
    Address* WantedLevel;
public:
	Memory();
	~Memory();

	DWORD GetModuleBaseAddress(const char* lpszModuleName, DWORD processID);
	HANDLE GetProcessHandle() const { return processHandle; }

    template <typename T>
    bool ReadMemory(DWORD address, T& outputVal) { return ReadProcessMemory(processHandle, (LPCVOID)address, &outputVal, sizeof(T), nullptr); }

    template <typename T>
    bool WriteMemory(DWORD address, const T& inputVal){ return WriteProcessMemory(processHandle, (LPVOID)address, &inputVal, sizeof(T), nullptr); }

    template <typename T>
    void Freeze(DWORD address, const T& value)
    {
        T currentValue;
        
        ReadMemory(address, currentValue);

        if (currentValue < value)
            WriteMemory(address, value);
    }
};

