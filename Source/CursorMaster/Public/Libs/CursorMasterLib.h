// Developed by Wellsaik

#pragma once

#include "CoreMinimal.h"
#include "Assets/HardwareCursorAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CursorMasterLib.generated.h"

class UTexture2D;

/**
 * 
 */
UCLASS()
class CURSORMASTER_API UCursorMasterLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CursorMaster|Cursor", meta = (DisplayName = "Find Best Cursor Size", 
		ToolTip = "Finds the smallest cursor size at least as large as the requested size, or the largest available size as a fallback."))
	static bool FindBestSize(const UHardwareCursorAsset* InAsset, int32 InSize, FHardwareCursorSize& OutCursorSize);

	UFUNCTION(BlueprintCallable, Category = "CursorMaster|Cursor", meta = (DisplayName = "Hardware Cursor Size to Texture", 
		ToolTip = "Creates a transient texture from the PNG embedded in a hardware cursor size."))
	static UTexture2D* HardwareCursorSizeToTexture(const FHardwareCursorSize& InCursorSize);

	UFUNCTION(BlueprintCallable, Category = "CursorMaster|Cursor", meta = (DisplayName = "Set Hardware Cursor", 
		ToolTip = "Applies the PNG embedded in a hardware cursor size to the specified hardware cursor type."))
	static bool SetHardwareCursor(const FHardwareCursorSize& InCursorSize, TEnumAsByte<EMouseCursor::Type> InCursorType, FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "CursorMaster|Cursor")
	static bool ConvertPngBytesToCur(
		const TArray<uint8>& InPngBytes,
		FIntPoint InHotspot,
		TArray<uint8>& OutCurBytes,
		FString& OutError);
};
