#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SWindow;

struct FCursorPngImportRequest
{
	int32 TargetSize = 0;
	FIntPoint Hotspot = FIntPoint::ZeroValue;
};

enum class ECursorPngImportMode : uint8
{
	Single,
	Multiple
};

class SCursorPngImportDialog final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCursorPngImportDialog) {}
		SLATE_ARGUMENT(FIntPoint, SourceSize)
		SLATE_ARGUMENT(TArray<int32>, ExistingSizes)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Window)
		SLATE_ARGUMENT(TFunction<void(TArray<FCursorPngImportRequest>)>, OnImport)
	SLATE_END_ARGS()

	static void Open(FIntPoint InSourceSize, TArray<int32> InExistingSizes, TFunction<void(TArray<FCursorPngImportRequest>)> InOnImport);
	void Construct(const FArguments& InArgs);

private:
	ECheckBoxState GetModeState(ECursorPngImportMode InMode) const;
	void SetMode(ECheckBoxState InState, ECursorPngImportMode InMode);
	EVisibility GetSingleSizeVisibility() const;
	EVisibility GetMultipleSizesVisibility() const;
	TSharedRef<SWidget> MakeSizePicker();
	ECheckBoxState GetSizeState(int32 InSize) const;
	void SetSizeState(ECheckBoxState InState, int32 InSize);
	bool IsSizeEnabled(int32 InSize) const;
	FText GetSelectedSizesText() const;
	TArray<FCursorPngImportRequest> BuildRequests() const;
	FText GetHotspotLabelX() const;
	FText GetHotspotLabelY() const;
	FText GetImportText() const;
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
	ECursorPngImportMode Mode = ECursorPngImportMode::Multiple;
	TSet<int32> SelectedSizes;
	int32 TargetSize = 1;
	int32 HotspotX = 0;
	int32 HotspotY = 0;
	TSharedPtr<SWindow> Window;
	TFunction<void(TArray<FCursorPngImportRequest>)> OnImport;
};
