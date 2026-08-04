// Developed by Wellsaik


#include "Libs/CursorMasterLib.h"

#if PLATFORM_WINDOWS
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

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