#pragma once

#include "CoreMinimal.h"

namespace CursorMasterImageImport
{
	bool ResizePng(const TArray<uint8>& InPngBytes, FIntPoint InSourceSize, int32 InTargetSize, TArray<uint8>& OutPngBytes);
}
