#pragma once

#include "AssetTypeActions_Base.h"

class FCursorMasterAssetActions final : public FAssetTypeActions_Base
{
public:
	FCursorMasterAssetActions(EAssetTypeCategories::Type InCategory, bool bInCollection) : Category(InCategory), bCollection(bInCollection) {}
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return Category; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> InToolkitHost) override;

private:
	EAssetTypeCategories::Type Category;
	bool bCollection;
};
