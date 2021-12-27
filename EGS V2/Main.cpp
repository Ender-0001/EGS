#include <Windows.h>
#include <iostream>
#include "SDK.hpp"

#include "MinHook/MinHook.h"
#pragma comment(lib, "MinHook/minhook.lib")

#include "Offsets.hpp"
#include "Globals.hpp"
#include "Beacons.hpp"
#include "Gui.h"

DWORD MainThread(LPVOID lpParam)
{
    while (true)
    {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {

        }
        else if (GetAsyncKeyState(VK_F6) & 0x8000)
        {

        }

        Sleep(1000 / 60);
    }
}

unsigned long __stdcall Main(void*)
{
    AllocConsole();

    FILE* pFile;
    freopen_s(&pFile, "CONIN$", "r", stdin);
    freopen_s(&pFile, "CONOUT$", "w", stderr);
    freopen_s(&pFile, "CONOUT$", "w", stdout);

    MH_Initialize();
    Globals::Init();
    auto Console = UConsole::StaticClass()->CreateDefaultObject<UConsole>();
    Console->Outer = Globals::LocalPlayer->ViewportClient;

    Globals::LocalPlayer->ViewportClient->ViewportConsole = Console;

    CreateThread(0, 0, GuiThread, 0, 0, 0);

    // CreateThread(0, 0, MainThread, 0, 0, 0);

    return 0;
}

int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved)
{
    if (dwReason == 1)
        CreateThread(0, 0, Main, 0, 0, 0);

    return 1;
}
