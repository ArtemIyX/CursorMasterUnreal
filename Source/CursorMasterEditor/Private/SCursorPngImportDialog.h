#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SWindow;

class SCursorPngImportDialog final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCursorPngImportDialog) {}
		SLATE_ARGUMENT(FIntPoint, SourceSize)
		SLATE_ARGUMENT(TArray<int32>, ExistingSizes)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Window)
		SLATE_ARGUMENT(TFunction<void(int32, FIntPoint)>, OnImport)
	SLATE_END_ARGS()

	static void Open(FIntPoint InSourceSize, TArray<int32> InExistingSizes, TFunction<void(int32, FIntPoint)> InOnImport);
	void Construct(const FArguments& InArgs);

private:
	TOptional<int32> GetTargetSize() const;
	TOptional<int32> GetHotspotX() const;
	TOptional<int32> GetHotspotY() const;
	void SetTargetSize(int32 InValue);
	void SetHotspotX(int32 InValue);
	void SetHotspotY(int32 InValue);
	bool CanImport() const;
	FReply Import();
	FReply Cancel();
	FText GetWarnings() const;

	FIntPoint SourceSize = FIntPoint::ZeroValue;
	TArray<int32> ExistingSizes;
	int32 TargetSize = 1;
	int32 HotspotX = 0;
	int32 HotspotY = 0;
	TSharedPtr<SWindow> Window;
	TFunction<void(int32, FIntPoint)> OnImport;
};