#if WITH_DEV_AUTOMATION_TESTS && PLATFORM_WINDOWS

#include "Misc/AutomationTest.h"

#include "CursorMasterImageImport.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Libs/CursorMasterLib.h"
#include "Modules/ModuleManager.h"

namespace
{
	TArray<uint8> MakeTestPng()
	{
		constexpr int32 size = 64;
		TArray<FColor> pixels;
		pixels.SetNumUninitialized(size * size);
		for (int32 y = 0; y < size; ++y)
		{
			for (int32 x = 0; x < size; ++x)
			{
				FColor& pixel = pixels[y * size + x];
				pixel = x < size / 2
					? (y < size / 2 ? FColor(255, 1, 2, 0) : FColor(5, 6, 255, 255))
					: (y < size / 2 ? FColor(3, 255, 4, 128) : FColor(7, 8, 9, 255));
			}
		}

		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> encoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!encoder.IsValid() || !encoder->SetRaw(pixels.GetData(), pixels.Num() * sizeof(FColor), size, size, ERGBFormat::BGRA, 8))
		{
			return {};
		}

		const TArray64<uint8> compressedBytes = encoder->GetCompressed();
		TArray<uint8> result;
		result.Append(compressedBytes.GetData(), compressedBytes.Num());
		return result;
	}

	bool DecodePng(const TArray<uint8>& InPngBytes, int32& OutWidth, int32& OutHeight, TArray<uint8>& OutRawBytes)
	{
		OutRawBytes.Reset();
		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> decoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		return decoder.IsValid() && decoder->SetCompressed(InPngBytes.GetData(), InPngBytes.Num())
			&& decoder->GetRaw(ERGBFormat::BGRA, 8, OutRawBytes)
			&& (OutWidth = decoder->GetWidth()) > 0
			&& (OutHeight = decoder->GetHeight()) > 0;
	}

	uint32 ReadUInt32(const TArray<uint8>& InBytes, int32 InOffset)
	{
		return static_cast<uint32>(InBytes[InOffset])
			| (static_cast<uint32>(InBytes[InOffset + 1]) << 8)
			| (static_cast<uint32>(InBytes[InOffset + 2]) << 16)
			| (static_cast<uint32>(InBytes[InOffset + 3]) << 24);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCursorMasterImageImportAlphaTest, "CursorMaster.Editor.ImageImport.RetainsAlpha",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCursorMasterImageImportAlphaTest::RunTest(const FString&)
{
	const TArray<uint8> sourcePng = MakeTestPng();
	TestTrue(TEXT("Test PNG is generated"), !sourcePng.IsEmpty());
	int32 sourceWidth = 0;
	int32 sourceHeight = 0;
	TArray<uint8> sourceRawBytes;
	TestTrue(TEXT("Same-size control PNG decodes"), DecodePng(sourcePng, sourceWidth, sourceHeight, sourceRawBytes));
	TestEqual(TEXT("Same-size control width"), sourceWidth, 64);
	TestEqual(TEXT("Same-size control height"), sourceHeight, 64);

	TArray<uint8> resizedPng;
	TestTrue(TEXT("PNG resize succeeds"), CursorMasterImageImport::ResizePng(sourcePng, FIntPoint(64, 64), 32, resizedPng));

	int32 width = 0;
	int32 height = 0;
	TArray<uint8> rawBytes;
	TestTrue(TEXT("Resized PNG decodes"), DecodePng(resizedPng, width, height, rawBytes));
	TestEqual(TEXT("Resized PNG width"), width, 32);
	TestEqual(TEXT("Resized PNG height"), height, 32);

	int32 transparentPixels = 0;
	int32 partialPixels = 0;
	int32 opaquePixels = 0;
	for (int32 index = 3; index < rawBytes.Num(); index += sizeof(FColor))
	{
		const uint8 alpha = rawBytes[index];
		transparentPixels += alpha == 0;
		partialPixels += alpha > 0 && alpha < 255;
		opaquePixels += alpha == 255;
	}
	TestTrue(TEXT("Transparent pixels remain transparent"), transparentPixels > 0);
	TestTrue(TEXT("Partial alpha remains non-opaque"), partialPixels > 0);
	TestTrue(TEXT("Opaque pixels remain opaque"), opaquePixels > 0);

	TArray<uint8> curBytes;
	FString error;
	TestTrue(TEXT("Resized PNG converts to CUR"), UCursorMasterLib::ConvertPngBytesToCur(resizedPng, FIntPoint(4, 4), curBytes, error));
	TestTrue(TEXT("CUR contains a PNG payload"), curBytes.Num() >= 22);
	if (curBytes.Num() >= 22)
	{
		const uint32 pngSize = ReadUInt32(curBytes, 14);
		const uint32 pngOffset = ReadUInt32(curBytes, 18);
		TestTrue(TEXT("CUR PNG range is valid"), pngOffset <= static_cast<uint32>(curBytes.Num()) && pngSize <= static_cast<uint32>(curBytes.Num()) - pngOffset);
		if (pngOffset <= static_cast<uint32>(curBytes.Num()) && pngSize <= static_cast<uint32>(curBytes.Num()) - pngOffset)
		{
			TArray<uint8> embeddedPng;
			embeddedPng.Append(curBytes.GetData() + pngOffset, pngSize);
			int32 embeddedWidth = 0;
			int32 embeddedHeight = 0;
			TArray<uint8> embeddedRawBytes;
			TestTrue(TEXT("Embedded PNG decodes"), DecodePng(embeddedPng, embeddedWidth, embeddedHeight, embeddedRawBytes));
			TestEqual(TEXT("Embedded PNG width"), embeddedWidth, 32);
			TestEqual(TEXT("Embedded PNG height"), embeddedHeight, 32);
			bool hasPartialAlpha = false;
			for (int32 index = 3; index < embeddedRawBytes.Num(); index += sizeof(FColor))
			{
				if (embeddedRawBytes[index] > 0 && embeddedRawBytes[index] < 255)
				{
					hasPartialAlpha = true;
					break;
				}
			}
			TestTrue(TEXT("Embedded PNG retains partial alpha"), hasPartialAlpha);
		}
	}

	return true;
}

#endif
