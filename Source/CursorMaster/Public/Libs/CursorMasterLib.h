// Developed by Wellsaik

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CursorMasterLib.generated.h"

/**
 * 
 */
UCLASS()
class CURSORMASTER_API UCursorMasterLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CursorMaster|Cursor")
	static bool ConvertPngBytesToCur(
		const TArray<uint8>& InPngBytes,
		FIntPoint InHotspot,
		TArray<uint8>& OutCurBytes,
		FString& OutError);
};
