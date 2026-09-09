#include "CursorMasterCursorImage.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

namespace CursorMasterCursorImage
{
	bool DecodePngToRgba(const uint8* InPngData, const int64 InPngSize, int32& OutWidth, int32& OutHeight, TArray<uint8>& OutRgbaBytes)
	{
		OutWidth = 0;
		OutHeight = 0;
		OutRgbaBytes.Reset();
		if (!InPngData || InPngSize <= 0)
		{
			return false;
		}

		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> image = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!image.IsValid() || !image->SetCompressed(InPngData, InPngSize) || !image->GetRaw(ERGBFormat::RGBA, 8, OutRgbaBytes))
		{
			return false;
		}

		OutWidth = image->GetWidth();
		OutHeight = image->GetHeight();
		const int64 expectedByteCount = static_cast<int64>(OutWidth) * OutHeight * 4;
		return OutWidth > 0 && OutHeight > 0 && expectedByteCount == OutRgbaBytes.Num();
	}
}
