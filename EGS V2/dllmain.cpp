#include <Windows.h>
#include <string>
#include <locale>
#include <algorithm>
#include "Types.hpp"
#include "UE4.hpp"
#include "Util.h"
#include <iostream>
#include <fstream>

#include "minhook/minhook.h"
#pragma comment(lib, "minhook/minhook.lib")

bool (*InitHost)(void*);
void* (*NotifyControlMessage)(UObject* World, UObject* NetConnection, uint8_t MessageType, void* FInBunch); //UWorld::NotifyControlMessage
void* (*PauseBeaconRequest)(UObject* Beacon, bool); //AOnlineBeacon::PauseBeaconRequest
void* (*WelcomePlayer)(UObject* World, UObject* NetConnection); //UWorld::WelcomePlayer

void* NotifyControlMessageHook(UObject* Beacon, UObject* NetConnection, uint8_t MessageType, void* FInBunch) //AOnlineBeaconHost::NotifyControlMessage
{
<<<<<<< Updated upstream
    std::cout << "NotifyControlMessage Hook: " << std::to_string(MessageType) << std::endl;
=======
<<<<<<< HEAD
    std::cout << "NotifyControlMessage Hook: " << std::to_string(MessageType) << std::endl;
=======
    // MessageType Numbers https://github.com/EpicGames/UnrealEngine/blob/c3caf7b6bf12ae4c8e09b606f10a09776b4d1f38/Engine/Source/Runtime/Engine/Public/Net/DataChannel.h#L148
    std::cout << "MessageType: " << std::to_string(MessageType) << std::endl;
>>>>>>> 92df12a4288e25d197428f5253edec15c4ca1971
>>>>>>> Stashed changes
    if (std::stoi(std::to_string(MessageType)) == 5)
    {
        //WelcomePlayer(UE4::GetWorld(), NetConnection);
        return NULL;
    }
    return NotifyControlMessage(UE4::GetWorld(), NetConnection, MessageType, FInBunch);
}

void* WelcomePlayerHook(UObject* World, UObject* NetConnection) //UWorld::WelcomePlayer
{
    std::cout << "WelcomePlayer Hook: " << NetConnection->GetFullName() << std::endl;
    return WelcomePlayer(UE4::GetWorld(), NetConnection);
}

unsigned long __stdcall Main(void*)
{
    Util::SetupConsole();

    auto BaseAddr = (uintptr_t)GetModuleHandle(0);

    auto GObjectsAddr = Util::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30");
    auto ObjectsOffset = *(int32_t*)(GObjectsAddr + 3);
    auto FinalObjectsAddress = GObjectsAddr + 7 + ObjectsOffset;
    GlobalObjects = decltype(GlobalObjects)(FinalObjectsAddress);

    auto FNameToStringAddr = Util::FindPattern("48 89 5C 24 ? 57 48 83 EC 40 83 79 04 00 48 8B DA 48 8B F9");
    FNameToString = decltype(FNameToString)(FNameToStringAddr);

    auto FreeMemoryAddr = Util::FindPattern("48 85 C9 74 1D 4C 8B 05 ? ? ? ? 4D 85 C0");
    FreeInternal = decltype(FreeInternal)(FreeMemoryAddr);

    auto SpawnActorAddr = Util::FindPattern("40 53 56 57 48 83 EC 70 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 0F 28 1D ? ? ? ? 0F 57 D2 48 8B B4 24 ? ? ? ? 0F 28 CB");
    SpawnActor = decltype(SpawnActor)(SpawnActorAddr);

    auto InitHostAddr = BaseAddr + 0x340670;
    InitHost = decltype(InitHost)(InitHostAddr);

    auto NotiControlMessageAddr = BaseAddr + 0x343AE0; //AOnlineBeaconHost::NotifyControlMessage

    NotifyControlMessage = decltype(NotifyControlMessage)(BaseAddr + 0x254BB00); //UWorld::NotifyControlMessage

    auto PauseBeaconRequestAddr = BaseAddr + 0xD6DFA0;
    PauseBeaconRequest = decltype(PauseBeaconRequest)(PauseBeaconRequestAddr);

    auto WelcomePlayerAddr = BaseAddr + 0x2557260;
    WelcomePlayer = decltype(WelcomePlayer)(WelcomePlayerAddr);

    MH_Initialize();
    MH_CreateHook(reinterpret_cast<void*>(NotiControlMessageAddr), NotifyControlMessageHook, nullptr);
    MH_EnableHook(reinterpret_cast<void*>(NotiControlMessageAddr));
    MH_CreateHook(reinterpret_cast<void*>(WelcomePlayerAddr), WelcomePlayerHook, reinterpret_cast<void**>(WelcomePlayer));
    MH_EnableHook(reinterpret_cast<void*>(WelcomePlayerAddr));

    auto PortOffset = UE4::FindOffset("IntProperty /Script/OnlineSubsystemUtils.OnlineBeaconHost.ListenPort");

    auto Location = FVector{0, 0, 10000};
    auto Rotation = FRotator();
    auto Beacon = SpawnActor(UE4::GetWorld(), UE4::FindObject("Class /Script/OnlineSubsystemUtils.OnlineBeaconHost"), &Location, &Rotation, FActorSpawnParameters());
    auto Port = reinterpret_cast<int*>(__int64(Beacon) + __int64(PortOffset));
    *Port = 7777;
    InitHost(Beacon);
    PauseBeaconRequest(Beacon, false);

    return 0;
}

int __stdcall DllMain(void* hModule, unsigned long dwReason, void* lpReserved)
{
    if (dwReason == 1)
        CreateThread(0, 0, Main, 0, 0, 0);

    return 1;
}
