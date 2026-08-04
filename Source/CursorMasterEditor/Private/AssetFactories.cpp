#include "AssetFactories.h"

#include "Assets/HardwareCursorAsset.h"
#include "Assets/HardwareCursorCollectionAsset.h"

UHardwareCursorAssetFactory::UHardwareCursorAssetFactory()
{
	SupportedClass = UHardwareCursorAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UHardwareCursorAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags InFlags, UObject* InContext, FFeedbackContext* InWarn)
{
	return NewObject<UHardwareCursorAsset>(InParent, InClass, InName, InFlags);
}

UHardwareCursorCollectionAssetFactory::UHardwareCursorCollectionAssetFactory()
{
	SupportedClass = UHardwareCursorCollectionAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UHardwareCursorCollectionAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags InFlags, UObject* InContext, FFeedbackContext* InWarn)
{
	return NewObject<UHardwareCursorCollectionAsset>(InParent, InClass, InName, InFlags);
}
