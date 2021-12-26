#include <Windows.h>
#include <iostream>
#include "SDK.hpp"

using namespace SDK;

#include "MinHook/MinHook.h"
#pragma comment(lib, "MinHook/minhook.lib")

#include "Offsets.hpp"
#include "Globals.hpp"
#include "Beacons.hpp"

enum EBeaconState
{
    AllowRequests,
    DenyRequests
};

unsigned long __stdcall Main(void*)
{
    AllocConsole();

    FILE* pFile;
    freopen_s(&pFile, "CONIN$", "r", stdin);
    freopen_s(&pFile, "CONOUT$", "w", stderr);
    freopen_s(&pFile, "CONOUT$", "w", stdout);

    MH_Initialize();
    Globals::Init();
    /*auto Console = UConsole::StaticClass()->CreateDefaultObject<UConsole>();
    Console->Outer = Globals::LocalPlayer->ViewportClient;

    Globals::LocalPlayer->ViewportClient->ViewportConsole = Console;

    MessageBoxA(NULL, "Press okay when the loading bar is ready", "EGS V2", MB_OK);*/
    Beacons::Init();

    auto Beacon = SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass(), { 0, 0, 10000 }, {});
    Beacon->ListenPort = 7777;

    Beacons::InitHost(Beacon);
    Beacon->BeaconState = EBeaconState::AllowRequests;

    TArray<AActor*> OutActors;
    Globals::GameplayStatics->STATIC_GetAllActorsOfClass(Globals::World, APlayerPawn_Athena_C::StaticClass(), &OutActors);
    Globals::Pawn = static_cast<APlayerPawn_Athena_C*>(OutActors[0]);

    if (Globals::Pawn)
    {
        Globals::PlayerController->Possess(Globals::Pawn);

        Sleep(2000);

        Globals::PlayerController->ServerReadyToStartMatch();
        static_cast<AGameMode*>(Globals::World->AuthorityGameMode)->StartMatch();
    }
    else
        MessageBoxA(NULL, "Invalid PlayerPawn", "EGS V2", MB_OK);

    return 0;
}

int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved)
{
    if (dwReason == 1)
        CreateThread(0, 0, Main, 0, 0, 0);

    return 1;
}
