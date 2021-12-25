#pragma once

template<typename T>
__forceinline static T* ReadPtr(uintptr_t offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(GetModuleHandle(0)) + offset);
}

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

namespace Globals
{
	UEngine* GEngine;
	UGameplayStatics* GameplayStatics;
	UWorld* World;
	APlayerPawn_Athena_C* Pawn;

	static void Init()
	{
		Globals::GEngine = *reinterpret_cast<UFortEngine**>(ReadPtr<uintptr_t**>(Offsets::GEngineOffset));
		FName::GNames = *reinterpret_cast<TNameEntryArray**>(ReadPtr<uintptr_t**>(Offsets::GNamesOffset));
		UObject::GObjects = reinterpret_cast<FUObjectArray*>(ReadPtr<uintptr_t*>(Offsets::GUObjectArrayOffset));

		World = Globals::GEngine->GameViewport->World;
		Globals::GameplayStatics = reinterpret_cast<UGameplayStatics*>(UGameplayStatics::StaticClass());
	}
}

template <class T>
T* SpawnActor(UClass* ActorClass, FVector Location, FRotator Rotation)
{
	FQuat Quat;
	FTransform Transform;
	Quat.W = 0;
	Quat.X = Rotation.Pitch;
	Quat.Y = Rotation.Roll;
	Quat.Z = Rotation.Yaw;

	Transform.Rotation = Quat;
	Transform.Scale3D = FVector{ 1,1,1 };
	Transform.Translation = Location;

	auto Actor = Globals::GameplayStatics->STATIC_BeginSpawningActorFromClass(Globals::World, ActorClass, Transform, false, nullptr);
	Globals::GameplayStatics->STATIC_FinishSpawningActor(Actor, Transform);
	return static_cast<T*>(Actor);
}