// Developed by Wellsaik


#include "Libs/CursorMasterLib.h"

#include "CursorMasterCursorImage.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/ICursor.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

UTexture2D* UCursorMasterLib::HardwareCursorSizeToTexture(const FHardwareCursorSize& InCursorSize)
{
	if (InCursorSize.CurBytes.Num() < 22)
	{
		return nullptr;
	}

	const uint16 reserved = static_cast<uint16>(InCursorSize.CurBytes[0]) | (static_cast<uint16>(InCursorSize.CurBytes[1]) << 8);
	const uint16 type = static_cast<uint16>(InCursorSize.CurBytes[2]) | (static_cast<uint16>(InCursorSize.CurBytes[3]) << 8);
	const uint16 imageCount = static_cast<uint16>(InCursorSize.CurBytes[4]) | (static_cast<uint16>(InCursorSize.CurBytes[5]) << 8);
	if (reserved != 0 || type != 2 || imageCount < 1)
	{
		return nullptr;
	}

	const uint32 pngSize = static_cast<uint32>(InCursorSize.CurBytes[14]) |
		(static_cast<uint32>(InCursorSize.CurBytes[15]) << 8) |
		(static_cast<uint32>(InCursorSize.CurBytes[16]) << 16) |
		(static_cast<uint32>(InCursorSize.CurBytes[17]) << 24);
	const uint32 pngOffset = static_cast<uint32>(InCursorSize.CurBytes[18]) |
		(static_cast<uint32>(InCursorSize.CurBytes[19]) << 8) |
		(static_cast<uint32>(InCursorSize.CurBytes[20]) << 16) |
		(static_cast<uint32>(InCursorSize.CurBytes[21]) << 24);
	if (pngSize == 0 || pngOffset < 22 || pngOffset > static_cast<uint32>(InCursorSize.CurBytes.Num()) || pngSize > static_cast<uint32>(InCursorSize.CurBytes.Num()) - pngOffset)
	{
		return nullptr;
	}

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	const TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	TArray<uint8> rawBytes;
	if (!imageWrapper.IsValid() || !imageWrapper->SetCompressed(InCursorSize.CurBytes.GetData() + pngOffset, pngSize) || !imageWrapper->GetRaw(ERGBFormat::BGRA, 8, rawBytes))
	{
		return nullptr;
	}

	const int32 width = imageWrapper->GetWidth();
	const int32 height = imageWrapper->GetHeight();
	const int64 expectedByteCount = static_cast<int64>(width) * height * sizeof(FColor);
	if (width < 1 || height < 1 || expectedByteCount != rawBytes.Num())
	{
		return nullptr;
	}

	UTexture2D* texture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);
	if (!texture || !texture->GetPlatformData() || texture->GetPlatformData()->Mips.IsEmpty())
	{
		return nullptr;
	}

	FTexture2DMipMap& mip = texture->GetPlatformData()->Mips[0];
	void* pixels = mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!pixels)
	{
		mip.BulkData.Unlock();
		return nullptr;
	}
	FMemory::Memcpy(pixels, rawBytes.GetData(), rawBytes.Num());
	mip.BulkData.Unlock();
	texture->NeverStream = true;
	texture->UpdateResource();
	return texture;
}

bool UCursorMasterLib::SetHardwareCursor(const FHardwareCursorSize& InCursorSize, const TEnumAsByte<EMouseCursor::Type> InCursorType, FString& OutError)
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

	if (InCursorSize.CurBytes.Num() < 22)
	{
		OutError = TEXT("Cursor data is truncated.");
		return false;
	}

	const uint16 reserved = static_cast<uint16>(InCursorSize.CurBytes[0]) | (static_cast<uint16>(InCursorSize.CurBytes[1]) << 8);
	const uint16 type = static_cast<uint16>(InCursorSize.CurBytes[2]) | (static_cast<uint16>(InCursorSize.CurBytes[3]) << 8);
	const uint16 imageCount = static_cast<uint16>(InCursorSize.CurBytes[4]) | (static_cast<uint16>(InCursorSize.CurBytes[5]) << 8);
	if (reserved != 0 || type != 2 || imageCount < 1)
	{
		OutError = TEXT("Cursor data is not a valid CUR payload.");
		return false;
	}

	const uint32 pngSize = static_cast<uint32>(InCursorSize.CurBytes[14]) |
		(static_cast<uint32>(InCursorSize.CurBytes[15]) << 8) |
		(static_cast<uint32>(InCursorSize.CurBytes[16]) << 16) |
		(static_cast<uint32>(InCursorSize.CurBytes[17]) << 24);
	const uint32 pngOffset = static_cast<uint32>(InCursorSize.CurBytes[18]) |
		(static_cast<uint32>(InCursorSize.CurBytes[19]) << 8) |
		(static_cast<uint32>(InCursorSize.CurBytes[20]) << 16) |
		(static_cast<uint32>(InCursorSize.CurBytes[21]) << 24);
	if (pngSize == 0 || pngOffset < 22 || pngOffset > static_cast<uint32>(InCursorSize.CurBytes.Num()) || pngSize > static_cast<uint32>(InCursorSize.CurBytes.Num()) - pngOffset)
	{
		OutError = TEXT("Cursor data contains an invalid PNG range.");
		return false;
	}

	int32 width = 0;
	int32 height = 0;
	TArray<uint8> rawBytes;
	if (!CursorMasterCursorImage::DecodePngToRgba(InCursorSize.CurBytes.GetData() + pngOffset, pngSize, width, height, rawBytes))
	{
		OutError = TEXT("The embedded PNG could not be decoded.");
		return false;
	}

	const int64 expectedByteCount = static_cast<int64>(width) * height * 4;
	if (width < 1 || height < 1 || expectedByteCount != rawBytes.Num())
	{
		OutError = TEXT("The embedded PNG has invalid pixel data.");
		return false;
	}

	void* cursorHandle = platformCursor->CreateCursorFromRGBABuffer(reinterpret_cast<const FColor*>(rawBytes.GetData()), width, height, FVector2D(InCursorSize.Hotspot.X, InCursorSize.Hotspot.Y));
	if (!cursorHandle)
	{
		OutError = TEXT("A hardware cursor could not be created.");
		return false;
	}

	platformCursor->SetTypeShape(InCursorType, cursorHandle);
	return true;
}

#if PLATFORM_WINDOWS
namespace
{
	void WriteUInt16(TArray<uint8>& OutBytes, uint16 InValue)
	{
		OutBytes.Add(static_cast<uint8>(InValue));
		OutBytes.Add(static_cast<uint8>(InValue >> 8));
	}

	void WriteUInt32(TArray<uint8>& OutBytes, uint32 InValue)
	{
		OutBytes.Add(static_cast<uint8>(InValue));
		OutBytes.Add(static_cast<uint8>(InValue >> 8));
		OutBytes.Add(static_cast<uint8>(InValue >> 16));
		OutBytes.Add(static_cast<uint8>(InValue >> 24));
	}
}
#endif

bool UCursorMasterLib::FindBestSize(const UHardwareCursorAsset* InAsset, int32 InSize, FHardwareCursorSize& OutCursorSize)
{
	OutCursorSize = FHardwareCursorSize();
	if (!InAsset)
	{
		return false;
	}

	if (const FHardwareCursorSize* hardwareSize = InAsset->FindBestSize(InSize))
	{
		OutCursorSize = *hardwareSize;
		return true;
	}
	return false;
}

bool UCursorMasterLib::ConvertPngBytesToCur(
	const TArray<uint8>& InPngBytes,
	FIntPoint InHotspot,
	TArray<uint8>& OutCurBytes,
	FString& OutError)
{
	OutCurBytes.Reset();
	OutError.Reset();

	#if !PLATFORM_WINDOWS
	OutError = TEXT("CUR conversion is supported only on Windows.");
	return false;
	#else
	if (InPngBytes.IsEmpty())
	{
		OutError = TEXT("PNG bytes are empty.");
		return false;
	}

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	const TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!imageWrapper.IsValid() || !imageWrapper->SetCompressed(InPngBytes.GetData(), InPngBytes.Num()))
	{
		OutError = TEXT("PNG bytes are invalid.");
		return false;
	}

	const int32 width = imageWrapper->GetWidth();
	const int32 height = imageWrapper->GetHeight();
	if (width < 1 || width > 256 || height < 1 || height > 256)
	{
		OutError = TEXT("PNG dimensions must be between 1 and 256 pixels.");
		return false;
	}

	TArray<uint8> pixelBytes;
	if (!imageWrapper->GetRaw(ERGBFormat::BGRA, 8, pixelBytes))
	{
		OutError = TEXT("PNG pixel data is invalid.");
		return false;
	}

	if (InHotspot.X < 0 || InHotspot.X >= width || InHotspot.Y < 0 || InHotspot.Y >= height)
	{
		OutError = TEXT("Hotspot must be inside the PNG bounds.");
		return false;
	}

	OutCurBytes.Reserve(22 + InPngBytes.Num());
	WriteUInt16(OutCurBytes, 0);
	WriteUInt16(OutCurBytes, 2);
	WriteUInt16(OutCurBytes, 1);
	OutCurBytes.Add(width == 256 ? 0 : static_cast<uint8>(width));
	OutCurBytes.Add(height == 256 ? 0 : static_cast<uint8>(height));
	OutCurBytes.Add(0);
	OutCurBytes.Add(0);
	WriteUInt16(OutCurBytes, static_cast<uint16>(InHotspot.X));
	WriteUInt16(OutCurBytes, static_cast<uint16>(InHotspot.Y));
	WriteUInt32(OutCurBytes, static_cast<uint32>(InPngBytes.Num()));
	WriteUInt32(OutCurBytes, 22);
	OutCurBytes.Append(InPngBytes);
	return true;
	#endif
}
