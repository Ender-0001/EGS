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
    MH_Initialize();
    Globals::Init();
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
        MessageBoxA(NULL, "If this crashes your gay", "Test", MB_OK);
        auto LocalPlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(Globals::GEngine->GameViewport->World->OwningGameInstance->LocalPlayers[0]->PlayerController);
        LocalPlayerController->Possess(Globals::Pawn);

        Sleep(3000);

        MessageBoxA(NULL, "No gay", "Test", MB_OK);
        LocalPlayerController->ServerReadyToStartMatch();
        static_cast<AGameMode*>(Globals::World->AuthorityGameMode)->StartMatch();
    }
    else
    {
        MessageBoxA(NULL, "Invalid Pawn", "Test", MB_OK);
    }

    return 0;
}

int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved)
{
    if (dwReason == 1)
        CreateThread(0, 0, Main, 0, 0, 0);

    return 1;
}
