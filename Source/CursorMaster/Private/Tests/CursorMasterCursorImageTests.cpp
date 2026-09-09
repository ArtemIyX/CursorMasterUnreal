#if WITH_DEV_AUTOMATION_TESTS && PLATFORM_WINDOWS

#include "Misc/AutomationTest.h"

#include "CursorMasterCursorImage.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

namespace
{
	TArray<uint8> MakeTestPng()
	{
		constexpr int32 width = 2;
		constexpr int32 height = 2;
		const TArray<uint8> rgbaBytes = {
			255, 64, 16, 255,
			16, 64, 255, 128,
			255, 128, 0, 0,
			32, 200, 64, 255
		};

		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> encoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!encoder.IsValid() || !encoder->SetRaw(rgbaBytes.GetData(), rgbaBytes.Num(), width, height, ERGBFormat::RGBA, 8))
		{
			return {};
		}

		const TArray64<uint8> compressedBytes = encoder->GetCompressed();
		TArray<uint8> result;
		result.Append(compressedBytes.GetData(), compressedBytes.Num());
		return result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCursorMasterCursorImageChannelOrderTest, "CursorMaster.Runtime.CursorImage.ChannelOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCursorMasterCursorImageChannelOrderTest::RunTest(const FString&)
{
	const TArray<uint8> pngBytes = MakeTestPng();
	TestTrue(TEXT("Known-color PNG is generated"), !pngBytes.IsEmpty());

	int32 width = 0;
	int32 height = 0;
	TArray<uint8> rgbaBytes;
	TestTrue(TEXT("PNG decodes for hardware cursor input"), CursorMasterCursorImage::DecodePngToRgba(pngBytes.GetData(), pngBytes.Num(), width, height, rgbaBytes));
	TestEqual(TEXT("Decoded width"), width, 2);
	TestEqual(TEXT("Decoded height"), height, 2);
	TestEqual(TEXT("Decoded byte count"), rgbaBytes.Num(), 16);

	if (rgbaBytes.Num() == 16)
	{
		TestEqual(TEXT("Orange red channel is preserved"), rgbaBytes[0], static_cast<uint8>(255));
		TestEqual(TEXT("Orange blue channel is preserved"), rgbaBytes[2], static_cast<uint8>(16));
		TestEqual(TEXT("Blue red channel is preserved"), rgbaBytes[4], static_cast<uint8>(16));
		TestEqual(TEXT("Blue blue channel is preserved"), rgbaBytes[6], static_cast<uint8>(255));
		TestEqual(TEXT("Partial alpha is preserved"), rgbaBytes[7], static_cast<uint8>(128));
		TestEqual(TEXT("Transparent alpha is preserved"), rgbaBytes[11], static_cast<uint8>(0));
	}

	return true;
}

#endif
