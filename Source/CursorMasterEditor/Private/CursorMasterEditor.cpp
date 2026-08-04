#include "CursorMasterEditor.h"

#include "AssetToolsModule.h"
#include "CursorMasterAssetActions.h"
#include "CursorMasterDetails.h"
#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "FCursorMasterEditorModule"

void FCursorMasterEditorModule::StartupModule()
{
	Style = MakeShared<FSlateStyleSet>(TEXT("CursorMasterEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("CursorMaster"))->GetBaseDir() / TEXT("Resources"));
	Style->Set(TEXT("ClassIcon.HardwareCursorAsset"), new FSlateImageBrush(Style->RootToContentDir(TEXT("CursorAsset16"), TEXT(".png")), FVector2D(16.0f)));
	Style->Set(TEXT("ClassThumbnail.HardwareCursorAsset"), new FSlateImageBrush(Style->RootToContentDir(TEXT("CursorAsset128"), TEXT(".png")), FVector2D(128.0f)));
	Style->Set(TEXT("ClassIcon.HardwareCursorCollectionAsset"), new FSlateImageBrush(Style->RootToContentDir(TEXT("CursorCollectionAsset16"), TEXT(".png")), FVector2D(16.0f)));
	Style->Set(TEXT("ClassThumbnail.HardwareCursorCollectionAsset"), new FSlateImageBrush(Style->RootToContentDir(TEXT("CursorCollectionAsset128"), TEXT(".png")), FVector2D(128.0f)));
	FSlateStyleRegistry::RegisterSlateStyle(*Style);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	CursorCategory = AssetToolsModule.Get().RegisterAdvancedAssetCategory(TEXT("Cursors"), LOCTEXT("CursorsCategory", "Cursors"));
	AssetActions.Add(MakeShared<FCursorMasterAssetActions>(static_cast<EAssetTypeCategories::Type>(CursorCategory), false));
	AssetActions.Add(MakeShared<FCursorMasterAssetActions>(static_cast<EAssetTypeCategories::Type>(CursorCategory), true));
	for (const TSharedPtr<IAssetTypeActions>& Actions : AssetActions)
		AssetToolsModule.Get().RegisterAssetTypeActions(Actions.ToSharedRef());

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	PropertyEditor.RegisterCustomClassLayout(TEXT("HardwareCursorAsset"), FOnGetDetailCustomizationInstance::CreateStatic(&FHardwareCursorAssetDetails::MakeInstance));
	PropertyEditor.RegisterCustomClassLayout(TEXT("HardwareCursorCollectionAsset"), FOnGetDetailCustomizationInstance::CreateStatic(&FHardwareCursorCollectionAssetDetails::MakeInstance));
}

void FCursorMasterEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyEditor = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyEditor.UnregisterCustomClassLayout(TEXT("HardwareCursorAsset"));
		PropertyEditor.UnregisterCustomClassLayout(TEXT("HardwareCursorCollectionAsset"));
	}

	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		for (const TSharedPtr<IAssetTypeActions>& Actions : AssetActions)
			AssetTools.UnregisterAssetTypeActions(Actions.ToSharedRef());
	}
	AssetActions.Empty();

	if (Style.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Style);
		Style.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCursorMasterEditorModule, CursorMasterEditor)
