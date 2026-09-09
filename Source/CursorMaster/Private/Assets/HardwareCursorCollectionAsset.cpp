// Developed by Wellsaik

#include "Assets/HardwareCursorCollectionAsset.h"

#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/ICursor.h"
#include "Libs/CursorMasterLib.h"

bool UHardwareCursorCollectionAsset::Apply(const int32 InSize, FString& OutError) const
{
	OutError.Reset();
	if (!FSlateApplication::IsInitialized())
	{
		OutError = TEXT("Slate is not initialized.");
		return false;
	}

	const TSharedPtr<ICursor> platformCursor = FSlateApplication::Get().GetPlatformCursor();
	if (!platformCursor.IsValid() || !platformCursor->IsCreateCursorFromRGBABufferSupported())
	{
		OutError = TEXT("The platform does not support hardware cursors from image data.");
		return false;
	}

	for (const TPair<TEnumAsByte<EMouseCursor::Type>, TObjectPtr<UHardwareCursorAsset>>& pair : Cursors)
	{
		const UHardwareCursorAsset* asset = pair.Value;
		const FHardwareCursorSize* cursorSize = asset ? asset->FindBestSize(InSize) : nullptr;
		if (!cursorSize)
		{
			OutError = TEXT("A cursor mapping has no valid cursor data.");
			return false;
		}

		if (!UCursorMasterLib::SetHardwareCursor(*cursorSize, pair.Key, OutError))
		{
			return false;
		}
	}

	return true;
}
