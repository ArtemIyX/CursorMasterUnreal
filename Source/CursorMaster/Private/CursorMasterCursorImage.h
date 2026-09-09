#pragma once

#include "CoreMinimal.h"

namespace CursorMasterCursorImage
{
	bool DecodePngToRgba(const uint8* InPngData, int64 InPngSize, int32& OutWidth, int32& OutHeight, TArray<uint8>& OutRgbaBytes);
}
