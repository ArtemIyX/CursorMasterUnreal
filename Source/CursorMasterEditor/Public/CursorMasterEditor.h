#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCursorMasterEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<class FSlateStyleSet> Style;
	TArray<TSharedPtr<class IAssetTypeActions>> AssetActions;
	uint32 CursorCategory = 0;
};
