#pragma once

#include "Factories/Factory.h"
#include "AssetFactories.generated.h"

UCLASS()
class UHardwareCursorAssetFactory final : public UFactory
{
	GENERATED_BODY()

public:
	UHardwareCursorAssetFactory();
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags InFlags, UObject* InContext, FFeedbackContext* InWarn) override;
};

UCLASS()
class UHardwareCursorCollectionAssetFactory final : public UFactory
{
	GENERATED_BODY()

public:
	UHardwareCursorCollectionAssetFactory();
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags InFlags, UObject* InContext, FFeedbackContext* InWarn) override;
};
