#pragma once

enum EObjectFlags
{
	RF_NoFlags = 0x00000000,
	RF_Public = 0x00000001,
	RF_Standalone = 0x00000002,
	RF_MarkAsNative = 0x00000004,
	RF_Transactional = 0x00000008,
	RF_ClassDefaultObject = 0x00000010,
	RF_ArchetypeObject = 0x00000020,
	RF_Transient = 0x00000040,
	RF_MarkAsRootSet = 0x00000080,
	RF_TagGarbageTemp = 0x00000100,
	RF_NeedInitialization = 0x00000200,
	RF_NeedLoad = 0x00000400,
	RF_KeepForCooker = 0x00000800,
	RF_NeedPostLoad = 0x00001000,
	RF_NeedPostLoadSubobjects = 0x00002000,
	RF_NewerVersionExists = 0x00004000,
	RF_BeginDestroyed = 0x00008000,
	RF_FinishDestroyed = 0x00010000,
	RF_BeingRegenerated = 0x00020000,
	RF_DefaultSubObject = 0x00040000,
	RF_WasLoaded = 0x00080000,
	RF_TextExportTransient = 0x00100000,
	RF_LoadCompleted = 0x00200000,
	RF_InheritableComponentTemplate = 0x00400000,
	RF_DuplicateTransient = 0x00800000,
	RF_StrongRefOnFrame = 0x01000000,
	RF_NonPIEDuplicateTransient = 0x02000000,
	RF_Dynamic = 0x04000000,
	RF_WillBeLoaded = 0x08000000,
};

enum class ESpawnActorNameMode : uint8_t
{
	Required_Fatal,
	Required_ErrorAndReturnNull,
	Required_ReturnNull,
	Requested
};

enum class ESpawnActorCollisionHandlingMethod : uint8_t
{
	Undefined = 0,
	AlwaysSpawn = 1,
	AdjustIfPossibleButAlwaysSpawn = 2,
	AdjustIfPossibleButDontSpawnIfColliding = 3,
	DontSpawnIfColliding = 4,
	ESpawnActorCollisionHandlingMethod_MAX = 5
};

enum class EAthenaGamePhase : uint8_t
{
	None = 0,
	Setup = 1,
	Warmup = 2,
	Aircraft = 3,
	SafeZones = 4,
	EndGame = 5,
	Count = 6,
	EAthenaGamePhase_MAX = 7
};

enum class EFortCustomPartType : uint8_t
{
	Head = 0,
	Body = 1,
	Hat = 2,
	Backpack = 3,
	Charm = 4,
	Face = 5,
	NumTypes = 6,
	EFortCustomPartType_MAX = 7
};

enum class ENetRole : uint8_t
{
	ROLE_None = 0,
	ROLE_SimulatedProxy = 1,
	ROLE_AutonomousProxy = 2,
	ROLE_Authority = 3,
	ROLE_MAX = 4
};

template <class T>
struct TArray
{
	friend struct FString;

public:
	T* Data;
	int32_t Count;
	int32_t Max;

	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline const T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}

	inline void Add(T InputData)
	{
		Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
		Data[Count++] = InputData;
		Max = Count;
	};
};

struct FString : private TArray<wchar_t>
{
	FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	const wchar_t* ToWString() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

struct FGuid
{
	int A;
	int B;
	int C;
	int D;
};

struct FName;


inline void (*FreeInternal)(void*);
void (*FNameToString)(FName* pThis, FString& out);

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;

	FName() = default;

	explicit FName(int64_t name)
	{
		DisplayIndex = (name & 0xFFFFFFFF00000000LL) >> 32;
		ComparisonIndex = (name & 0xFFFFFFFFLL);
	};

	FName(uint32_t comparisonIndex, uint32_t displayIndex) : ComparisonIndex(comparisonIndex),
		DisplayIndex(displayIndex)
	{
	}

	auto ToString()
	{
		FString temp;
		FNameToString(this, temp);

		std::string ret(temp.ToString());

		FreeInternal((void*)temp.ToWString());

		return ret;
	}
};

void* (*ProcessEvent)(void* Object, void* Function, void* Params);

// This stays the same throughout all Fortnite builds so far, no need to change.
struct UObject
{
	void** VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	UObject* Class;
	FName NamePrivate;
	UObject* Outer;

	bool IsA(UObject* cmp) const
	{
		if (this->Class == cmp)
		{
			return true;
		}
		return false;
	}

	std::string GetName()
	{
		return NamePrivate.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Outer; outer; outer = outer->Outer)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(Class)->GetName() + " " + temp + this->GetName();
		return temp;
	}

	FName GetFName() const
	{
		return *reinterpret_cast<const FName*>(this + 0x18);
	}
};

struct UField : UObject
{
	UField* Next;
};

struct UProperty : UField
{
	uint32_t ArrayDim; // 0x30
	uint32_t ElementSize; // 0x34
	uint64_t PropertyFlags; // 0x38
	char pad_40[4]; // 0x40
	uint32_t Offset; // 0x44
	char pad_48[0x70 - 0x30 - 0x18];
};

struct UStruct : UField
{
	UStruct* Super; // 0x30
	struct UProperty* Children; // 0x38
	uint32_t Size; // 0x40
	char pad_44[0x88 - 0x30 - 0x14];
};
struct UFunction : UStruct
{
	uint32_t FunctionFlags;
	char pad[28]; // 0x8C
	void* Func; // 0xB0
};

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;
};

struct FVector
{
	float X;
	float Y;
	float Z;

	auto operator-(FVector A)
	{
		return FVector{ this->X - A.X, this->Y - A.Y, this->Z - A.Z };
	}

	auto operator+(FVector A)
	{
		return FVector{ this->X + A.X, this->Y + A.Y, this->Z + A.Z };
	}

	auto operator==(FVector A)
	{
		return (this->X == A.X && this->Y == A.Y && this->Z == A.Z);
	}

	auto operator!=(FVector A)
	{
		return (this->X != A.X && this->Y != A.Y && this->Z != A.Z);
	}
};

struct FVector2D
{
	float X;
	float Y;

	inline FVector2D()
		: X(0), Y(0)
	{
	}

	inline FVector2D(float x, float y)
		: X(x),
		Y(y)
	{
	}
};

struct TUObjectArray
{
	uint8_t* Objects;
	uint32_t MaxElements;
	uint32_t NumElements;
};

struct FKey {
	struct FName KeyName; // 0x00(0x08)
	char UnknownData_8[0x10]; // 0x08(0x10)
};

struct FText
{
	char data[0x18];
};

struct FActorSpawnParameters
{
	FActorSpawnParameters() : Name(), Template(nullptr), Owner(nullptr), Instigator(nullptr), OverrideLevel(nullptr),
		SpawnCollisionHandlingOverride(), bRemoteOwned(0), bNoFail(0),
		bDeferConstruction(0),
		bAllowDuringConstructionScript(0),
		NameMode(),
		ObjectFlags()
	{
	}
	;

	FName Name;
	UObject* Template;
	UObject* Owner;
	UObject* Instigator;
	UObject* OverrideLevel;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

private:
	uint8_t bRemoteOwned : 1;

public:
	bool IsRemoteOwned() const { return bRemoteOwned; }

	uint8_t bNoFail : 1;
	uint8_t bDeferConstruction : 1;
	uint8_t bAllowDuringConstructionScript : 1;
	ESpawnActorNameMode NameMode;
	EObjectFlags ObjectFlags;
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

struct FActiveGameplayEffectHandle
{
	int Handle; // 0x00(0x04)
	bool bPassedFiltersAndWasExecuted; // 0x04(0x01)
	char UnknownData_5[0x3]; // 0x05(0x03)
};

struct FGameplayEffectContextHandle
{
	char UnknownData_0[0x30]; // 0x00(0x18)
};

struct FGameplayAbilitySpecDef
{
	UObject* Ability;
	unsigned char Unk00[0x90];
};

struct FLinearColor {
	float R;
	float G;
	float B;
	float A;
};

enum class EFortResourceType : uint8_t
{
	Wood = 0,
	Stone = 1,
	Metal = 2,
	Permanite = 3,
	None = 4,
	EFortResourceType_MAX = 5
};