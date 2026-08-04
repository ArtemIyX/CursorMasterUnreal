// Developed by Wellsaik


#include "Assets/HardwareCursorAsset.h"

const FHardwareCursorSize* UHardwareCursorAsset::FindBestSize(int32 InSize) const
{
	for (const FHardwareCursorSize& cursorSize : Sizes)
	{
		if (cursorSize.Size >= InSize)
		{
			return &cursorSize;
		}
	}

	return Sizes.IsEmpty() ? nullptr : &Sizes.Last();
}
