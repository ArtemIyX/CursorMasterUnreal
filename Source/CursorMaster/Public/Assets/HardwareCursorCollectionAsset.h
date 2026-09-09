// Developed by Wellsaik

#pragma once

#include "CoreMinimal.h"
#include "Assets/HardwareCursorAsset.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "HardwareCursorCollectionAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CURSORMASTER_API UHardwareCursorCollectionAsset : public UObject
{
	GENERATED_BODY()

public:
	UHardwareCursorCollectionAsset() {}
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor")
	TMap<TEnumAsByte<EMouseCursor::Type>, TObjectPtr<UHardwareCursorAsset>> Cursors;

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	bool Apply(int32 InSize, FString& OutError) const;
};
