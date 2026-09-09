// Developed by Wellsaik

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HardwareCursorAsset.generated.h"

USTRUCT(BlueprintType)
struct CURSORMASTER_API FHardwareCursorSize
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Cursor")
	int32 Size = 0;

	UPROPERTY(VisibleAnywhere, Category = "Cursor")
	FIntPoint Hotspot = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, Category = "Cursor")
	TArray<uint8> CurBytes;
};

UCLASS(Blueprintable, BlueprintType)
class CURSORMASTER_API UHardwareCursorAsset : public UObject
{
	GENERATED_BODY()

public:
	UHardwareCursorAsset() {}
public:
	UPROPERTY(VisibleAnywhere, Category = "Cursor")
	TArray<FHardwareCursorSize> Sizes;

	const FHardwareCursorSize* FindBestSize(int32 InSize) const;
};
