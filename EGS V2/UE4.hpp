#pragma once

inline TUObjectArray* GlobalObjects;
inline UObject* (*SpawnActor)(UObject* World, UObject* Class, FVector* Position, FRotator* Rotation, const FActorSpawnParameters& SpawnParameters);
static UObject* (*StaticConstructObjectInternal)(void*, void*, void*, int, unsigned int, void*, bool, void*, bool);

namespace UE4
{
	static UObject* FindObjectById(uint32_t Id)
	{
		auto Offset = 24 * Id;
		return *(UObject**)(GlobalObjects->Objects + Offset);
	}

	static UObject* FindObject(const char* name)
	{
		for (int i = 0; i < GlobalObjects->NumElements; i++)
		{
			auto object = FindObjectById(i);

			if (!object)
				continue;

			if (object->GetFullName().find(name) != std::string::npos)
				return object;
		}

		return nullptr;
	}

	static DWORD FindOffset(std::string OffsetToFind)
	{
		UObject* Object = nullptr;

		for (int i = 0; i < GlobalObjects->NumElements; i++)
		{
			auto object = FindObjectById(i);

			if (object == nullptr)
				continue;

			if (object->GetFullName() == OffsetToFind)
				Object = object;
		}

		if (Object)
		{
			return *(uint32_t*)(__int64(Object) + 0x44);
		}

		return 0;
	}

	static UObject* GetWorld()
	{
		static auto FortEngine = FindObject("FortEngine_");
		static auto GameViewportOffset = FindOffset("ObjectProperty /Script/Engine.Engine.GameViewport");
		static auto WorldOffset = FindOffset("ObjectProperty /Script/Engine.GameViewportClient.World");

		UObject* GameViewport = *reinterpret_cast<UObject**>(__int64(FortEngine) + __int64(GameViewportOffset));
		UObject** World = reinterpret_cast<UObject**>(__int64(GameViewport) + __int64(WorldOffset));

		return *World;
	}

	static UObject* StaticConstructObject(UObject* Object, UObject* Outer)
	{
		return StaticConstructObjectInternal(Object, Outer, 0, 0, 0, 0, 0, 0, 0);
	}
}