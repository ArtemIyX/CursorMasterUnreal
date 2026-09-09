#include "CursorMasterImageImport.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Modules/ModuleManager.h"

namespace CursorMasterImageImport
{
	bool ResizePng(const TArray<uint8>& InPngBytes, const FIntPoint InSourceSize, const int32 InTargetSize, TArray<uint8>& OutPngBytes)
	{
		OutPngBytes.Reset();
		if (InPngBytes.IsEmpty() || InSourceSize.X < 1 || InSourceSize.Y < 1 || InTargetSize < 1)
		{
			return false;
		}

		const int64 sourcePixelCount = static_cast<int64>(InSourceSize.X) * InSourceSize.Y;
		const int64 targetPixelCount = static_cast<int64>(InTargetSize) * InTargetSize;
		if (sourcePixelCount > MAX_int32 || targetPixelCount > MAX_int32)
		{
			return false;
		}

		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> decoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		TArray<uint8> rawBytes;
		const int64 sourceByteCount = sourcePixelCount * sizeof(FColor);
		if (!decoder.IsValid() || !decoder->SetCompressed(InPngBytes.GetData(), InPngBytes.Num()) || decoder->GetWidth() != InSourceSize.X || decoder->GetHeight() != InSourceSize.Y || !decoder->GetRaw(ERGBFormat::BGRA, 8, rawBytes) || rawBytes.Num() != sourceByteCount)
		{
			return false;
		}

		TArray<FColor> sourcePixels;
		sourcePixels.SetNumUninitialized(static_cast<int32>(sourcePixelCount));
		FMemory::Memcpy(sourcePixels.GetData(), rawBytes.GetData(), rawBytes.Num());

		TArray<FColor> resizedPixels;
		FImageUtils::ImageResize(InSourceSize.X, InSourceSize.Y, sourcePixels, InTargetSize, InTargetSize, resizedPixels, false, false);
		if (resizedPixels.Num() != targetPixelCount)
		{
			return false;
		}

		const TSharedPtr<IImageWrapper> encoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!encoder.IsValid() || !encoder->SetRaw(resizedPixels.GetData(), resizedPixels.Num() * sizeof(FColor), InTargetSize, InTargetSize, ERGBFormat::BGRA, 8))
		{
			return false;
		}

		const TArray64<uint8> encodedBytes = encoder->GetCompressed();
		if (encodedBytes.IsEmpty())
		{
			return false;
		}

		OutPngBytes.Append(encodedBytes.GetData(), encodedBytes.Num());
		return true;
	}
}
