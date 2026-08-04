#pragma once

#include "IDetailCustomization.h"

class FHardwareCursorAssetDetails final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;
};

class FHardwareCursorCollectionAssetDetails final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;
};