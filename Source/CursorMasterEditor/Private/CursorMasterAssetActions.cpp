#include "CursorMasterAssetActions.h"

#include "Assets/HardwareCursorAsset.h"
#include "Assets/HardwareCursorCollectionAsset.h"
#include "CursorMasterAssetEditor.h"

FText FCursorMasterAssetActions::GetName() const
{
	return bCollection ? NSLOCTEXT("CursorMaster", "CursorCollectionAssetName", "Cursor Collection Asset") : NSLOCTEXT("CursorMaster", "CursorAssetName", "Cursor Asset");
}

FColor FCursorMasterAssetActions::GetTypeColor() const
{
	return bCollection ? FColor(126, 194, 67) : FColor(44, 142, 78);
}

UClass* FCursorMasterAssetActions::GetSupportedClass() const
{
	return bCollection ? UHardwareCursorCollectionAsset::StaticClass() : UHardwareCursorAsset::StaticClass();
}

void FCursorMasterAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> InToolkitHost)
{
	for (UObject* Object : InObjects)
	{
		if (bCollection)
		{
			const TSharedRef<FHardwareCursorCollectionAssetEditor> Editor = MakeShared<FHardwareCursorCollectionAssetEditor>();
			Editor->Init(CastChecked<UHardwareCursorCollectionAsset>(Object), InToolkitHost);
		}
		else
		{
			const TSharedRef<FHardwareCursorAssetEditor> Editor = MakeShared<FHardwareCursorAssetEditor>();
			Editor->Init(CastChecked<UHardwareCursorAsset>(Object), InToolkitHost);
		}
	}
}