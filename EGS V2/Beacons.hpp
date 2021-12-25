#pragma once

namespace Beacons
{
	bool (*InitHost)(AOnlineBeaconHost*); //AOnlineBeaconHost::InitHost
	void* (*NotifyControlMessage)(UWorld* World, UNetConnection* Connection, uint8 MessageType, void* InBunch); //UWorld::NotifyControlMessage
	APlayerController* (*SpawnPlayActor)(UWorld* World, UPlayer* Player, ENetRole Role, FURL* InUrl, void* UniqueId, FString& Error, uint8 InNetPlayerIndex);

	namespace Hooks
	{
		void* NotifyControlMessageHook(AOnlineBeaconHost* Beacon, UNetConnection* Connection, uint8 MessageType, void* InBunch)
		{
			if (std::to_string(MessageType) == "4")
			{
				Connection->CurrentNetSpeed = 10000;
				return NULL;
			}
			return NotifyControlMessage(Globals::World, Connection, MessageType, InBunch);
		}

		APlayerController* SpawnPlayActorHook(UWorld* World, UPlayer* Player, ENetRole Role, FURL* InUrl, void* UniqueId, FString& Error, uint8 InNetPlayerIndex)
		{
			auto PlayerController = static_cast<AFortPlayerControllerAthena*>(SpawnPlayActor(Globals::World, Player, Role, InUrl, UniqueId, Error, InNetPlayerIndex));
			auto FortPlayerState = static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState);
			FortPlayerState->TeamIndex = static_cast<AFortPlayerStateAthena*>(Globals::Pawn->PlayerState)->TeamIndex;
			FortPlayerState->OnRep_TeamIndex();

			auto Pawn = SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), { 0, 0, 10000 }, {});
			PlayerController->Possess(static_cast<APawn*>(Pawn));
			auto Location = Globals::Pawn->K2_GetActorLocation();
			Location.Z += 500;
			Pawn->K2_SetActorLocation(Location, false, true, new FHitResult());
			Pawn->Mesh->SetSkeletalMesh(UObject::FindObject<USkeletalMesh>("SkeletalMesh F_SML_Starter_Epic.F_SML_Starter_Epic"), true);

			Sleep(2000);

			PlayerController->ServerReadyToStartMatch();

			Sleep(500);
			Location = Pawn->K2_GetActorLocation();
			Location.Z += 500;
			Globals::Pawn->K2_SetActorLocation(Location, false, true, new FHitResult());

			PlayerController->SwitchLevel(L"Athena_Terrain?game=athena");

			return PlayerController;
		}

		void* ClientTravelHook(FString& URL, ETravelType TravelType, bool bSeamless, FGuid& MapPackage)
		{
			MH_DisableHook(reinterpret_cast<void*>((uintptr_t)GetModuleHandle(0) + 0x2315350));
			return NULL;
		}
	}

	static void Init()
	{
		auto ModuleBaseAddr = (uintptr_t)GetModuleHandle(0);
		InitHost = decltype(InitHost)(ModuleBaseAddr + 0x391E000);
		NotifyControlMessage = decltype(NotifyControlMessage)(ModuleBaseAddr + 0x249DE10);
		SpawnPlayActor = decltype(SpawnPlayActor)(ModuleBaseAddr + 0x21D8700);

		MH_CreateHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x3920F90), Hooks::NotifyControlMessageHook, nullptr);
		MH_EnableHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x3920F90));
		MH_CreateHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x21D8700), Hooks::SpawnPlayActorHook, reinterpret_cast<void**>(&SpawnPlayActor));
		MH_EnableHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x21D8700));
		MH_CreateHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x2315350), Hooks::ClientTravelHook, nullptr);
		MH_EnableHook(reinterpret_cast<void*>(ModuleBaseAddr + 0x2315350));
	}
}
