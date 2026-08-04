// Developed by Wellsaik


#include "Assets/HardwareCursorCollectionAsset.h"

#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/ICursor.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

bool UHardwareCursorCollectionAsset::Apply(int32 InSize, FString& OutError) const
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

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	for (const TPair<TEnumAsByte<EMouseCursor::Type>, TObjectPtr<UHardwareCursorAsset>>& pair : Cursors)
	{
		const UHardwareCursorAsset* asset = pair.Value;
		const FHardwareCursorSize* cursorSize = asset ? asset->FindBestSize(InSize) : nullptr;
		if (!cursorSize || cursorSize->CurBytes.Num() < 22)
		{
			OutError = TEXT("A cursor mapping has no valid cursor data.");
			return false;
		}

		const uint32 pngSize = cursorSize->CurBytes[14] | (cursorSize->CurBytes[15] << 8) | (cursorSize->CurBytes[16] << 16) | (cursorSize->CurBytes[17] << 24);
		const uint32 pngOffset = cursorSize->CurBytes[18] | (cursorSize->CurBytes[19] << 8) | (cursorSize->CurBytes[20] << 16) | (cursorSize->CurBytes[21] << 24);
		if (pngOffset > static_cast<uint32>(cursorSize->CurBytes.Num()) || pngSize > static_cast<uint32>(cursorSize->CurBytes.Num()) - pngOffset)
		{
			OutError = TEXT("A cursor mapping has invalid CUR data.");
			return false;
		}

		const TSharedPtr<IImageWrapper> image = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		TArray<uint8> rawBytes;
		if (!image.IsValid() || !image->SetCompressed(cursorSize->CurBytes.GetData() + pngOffset, pngSize) || !image->GetRaw(ERGBFormat::BGRA, 8, rawBytes))
		{
			OutError = TEXT("A cursor mapping could not be decoded.");
			return false;
		}

		const int32 width = image->GetWidth();
		const int32 height = image->GetHeight();
		if (rawBytes.Num() != width * height * sizeof(FColor))
		{
			OutError = TEXT("A cursor mapping has invalid pixel data.");
			return false;
		}

		void* cursorHandle = platformCursor->CreateCursorFromRGBABuffer(reinterpret_cast<const FColor*>(rawBytes.GetData()), width, height, FVector2D(cursorSize->Hotspot.X, cursorSize->Hotspot.Y));
		if (!cursorHandle)
		{
			OutError = TEXT("A hardware cursor could not be created.");
			return false;
		}
		platformCursor->SetTypeShape(pair.Key, cursorHandle);
	}

	return true;
}
