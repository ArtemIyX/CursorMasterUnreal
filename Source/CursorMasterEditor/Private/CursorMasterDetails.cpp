#include "CursorMasterDetails.h"

#include "Assets/HardwareCursorAsset.h"
#include "Assets/HardwareCursorCollectionAsset.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	void AddHeader(IDetailLayoutBuilder& InDetailLayout, const FText& InText, const FLinearColor& InColor)
	{
		InDetailLayout.EditCategory(TEXT("Cursor"), InText, ECategoryPriority::Important).AddCustomRow(InText).WholeRowContent()
		[
			SNew(SBorder).BorderBackgroundColor(InColor).Padding(8.0f)
			[
				SNew(STextBlock).Text(InText).ColorAndOpacity(FLinearColor::White)
			]
		];
	}
}

TSharedRef<IDetailCustomization> FHardwareCursorAssetDetails::MakeInstance()
{
	return MakeShared<FHardwareCursorAssetDetails>();
}

void FHardwareCursorAssetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	AddHeader(InDetailLayout, NSLOCTEXT("CursorMaster", "CursorAssetDetails", "Cursor Asset"), FLinearColor(0.08f, 0.36f, 0.18f));
}

TSharedRef<IDetailCustomization> FHardwareCursorCollectionAssetDetails::MakeInstance()
{
	return MakeShared<FHardwareCursorCollectionAssetDetails>();
}

void FHardwareCursorCollectionAssetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	AddHeader(InDetailLayout, NSLOCTEXT("CursorMaster", "CursorCollectionAssetDetails", "Cursor Collection Asset"), FLinearColor(0.22f, 0.48f, 0.12f));
	TArray<TWeakObjectPtr<UObject>> objects;
	InDetailLayout.GetObjectsBeingCustomized(objects);
	const UHardwareCursorCollectionAsset* collection = objects.IsEmpty() ? nullptr : Cast<UHardwareCursorCollectionAsset>(objects[0].Get());
	if (!collection) { return; }

	FString errors;
	FString warnings;
	const UHardwareCursorAsset* reference = nullptr;
	for (const TPair<TEnumAsByte<EMouseCursor::Type>, TObjectPtr<UHardwareCursorAsset>>& pair : collection->Cursors)
	{
		if (!pair.Value)
		{
			errors += FString::Printf(TEXT("%s has no cursor asset.\n"), *StaticEnum<EMouseCursor::Type>()->GetNameStringByValue(pair.Key.GetValue()));
			continue;
		}
		if (pair.Value->Sizes.IsEmpty())
		{
			errors += FString::Printf(TEXT("%s has no cursor sizes.\n"), *pair.Value->GetName());
			continue;
		}
		if (!reference)
		{
			reference = pair.Value;
			continue;
		}
		bool bMatchesReference = pair.Value->Sizes.Num() == reference->Sizes.Num();
		for (const FHardwareCursorSize& size : pair.Value->Sizes)
		{
			bMatchesReference &= reference->Sizes.ContainsByPredicate([&size](const FHardwareCursorSize& other) { return other.Size == size.Size; });
		}
		if (!bMatchesReference)
		{
			warnings += FString::Printf(TEXT("%s does not have the same cursor sizes as %s.\n"), *pair.Value->GetName(), *reference->GetName());
		}
	}

	IDetailCategoryBuilder& validation = InDetailLayout.EditCategory(TEXT("Validation"), INVTEXT("Validation"), ECategoryPriority::Important);
	if (!errors.IsEmpty())
	{
		validation.AddCustomRow(INVTEXT("Errors")).WholeRowContent()[SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.05f, 0.05f)).Padding(8.0f)[SNew(STextBlock).Text(FText::FromString(errors)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true)]];
	}
	if (!warnings.IsEmpty())
	{
		validation.AddCustomRow(INVTEXT("Warnings")).WholeRowContent()[SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.3f, 0.02f)).Padding(8.0f)[SNew(STextBlock).Text(FText::FromString(warnings)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true)]];
	}
}
