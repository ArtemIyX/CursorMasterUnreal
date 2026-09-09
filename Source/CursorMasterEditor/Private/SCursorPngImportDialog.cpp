#include "SCursorPngImportDialog.h"

#include "Framework/Application/SlateApplication.h"
#include "Misc/ConfigCacheIni.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr TCHAR ModeConfigSection[] = TEXT("CursorMaster");
	constexpr TCHAR ModeConfigKey[] = TEXT("CursorPngImportMode");
	constexpr int32 SupportedSizes[] = {8, 16, 32, 48, 64, 96, 128, 256};

	int32 ScaleHotspotAxis(const int32 InCoord, const int32 InSourceExtent, const int32 InTargetExtent)
	{
		if (InSourceExtent <= 1 || InTargetExtent <= 1) { return 0; }
		const double scale = static_cast<double>(InTargetExtent - 1) / (InSourceExtent - 1);
		return FMath::Clamp(FMath::RoundToInt(InCoord * scale), 0, InTargetExtent - 1);
	}
}

void SCursorPngImportDialog::Open(FIntPoint InSourceSize, TArray<int32> InExistingSizes, TFunction<void(TArray<FCursorPngImportRequest>)> InOnImport)
{
	const TSharedRef<SWindow> window = SNew(SWindow)
		.Title(INVTEXT("Import PNG Cursor"))
		.ClientSize(FVector2D(500.0f, 410.0f))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	window->SetContent(SNew(SCursorPngImportDialog)
		.SourceSize(InSourceSize)
		.ExistingSizes(MoveTemp(InExistingSizes))
		.Window(window)
		.OnImport(MoveTemp(InOnImport)));
	FSlateApplication::Get().AddModalWindow(window, nullptr);
}

void SCursorPngImportDialog::Construct(const FArguments& InArgs)
{
	SourceSize = InArgs._SourceSize;
	ExistingSizes = InArgs._ExistingSizes;
	TargetSize = FMath::Clamp(SourceSize.X, 1, 256);
	Window = InArgs._Window;
	OnImport = InArgs._OnImport;
	int32 savedMode = static_cast<int32>(Mode);
	if (GConfig)
	{
		GConfig->GetInt(ModeConfigSection, ModeConfigKey, savedMode, GEditorPerProjectIni);
	}
	Mode = savedMode == static_cast<int32>(ECursorPngImportMode::Single) ? ECursorPngImportMode::Single : ECursorPngImportMode::Multiple;

	for (const int32 size : SupportedSizes)
	{
		if (size != 8 && IsSizeEnabled(size)) { SelectedSizes.Add(size); }
	}

	ChildSlot
	[
		SNew(SBox).Padding(16.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(FText::Format(INVTEXT("Source PNG: {0} x {1}"), SourceSize.X, SourceSize.Y))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(STextBlock).Text(INVTEXT("Mode"))]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16, 0, 0, 0)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked_Lambda([this] { return GetModeState(ECursorPngImportMode::Single); })
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState state) { SetMode(state, ECursorPngImportMode::Single); })
					[SNew(STextBlock).Text(INVTEXT("Single size"))]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16, 0, 0, 0)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked_Lambda([this] { return GetModeState(ECursorPngImportMode::Multiple); })
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState state) { SetMode(state, ECursorPngImportMode::Multiple); })
					[SNew(STextBlock).Text(INVTEXT("Multiple sizes"))]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox).Visibility(this, &SCursorPngImportDialog::GetSingleSizeVisibility)
				+ SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(INVTEXT("Target size"))]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>)
					.Value(this, &SCursorPngImportDialog::GetTargetSize)
					.MinValue(1)
					.MaxValue(FMath::Min(SourceSize.X, 256))
					.OnValueChanged(this, &SCursorPngImportDialog::SetTargetSize)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox).Visibility(this, &SCursorPngImportDialog::GetMultipleSizesVisibility)
				+ SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(INVTEXT("Target sizes"))]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SComboButton)
					.ButtonContent()[SNew(STextBlock).Text(this, &SCursorPngImportDialog::GetSelectedSizesText)]
					.MenuContent()[MakeSizePicker()]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(this, &SCursorPngImportDialog::GetHotspotLabelX)]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>)
					.Value(this, &SCursorPngImportDialog::GetHotspotX)
					.MinValue(0)
					.MaxValue_Lambda([this] { return FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.X) - 1); })
					.OnValueChanged(this, &SCursorPngImportDialog::SetHotspotX)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(this, &SCursorPngImportDialog::GetHotspotLabelY)]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>)
					.Value(this, &SCursorPngImportDialog::GetHotspotY)
					.MinValue(0)
					.MaxValue_Lambda([this] { return FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.Y) - 1); })
					.OnValueChanged(this, &SCursorPngImportDialog::SetHotspotY)
				]
			]
			+ SVerticalBox::Slot().FillHeight(1).Padding(0, 12, 0, 0)
			[
				SNew(STextBlock).Text(this, &SCursorPngImportDialog::GetWarnings).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SUniformGridPanel).SlotPadding(FMargin(4, 0))
				+ SUniformGridPanel::Slot(0, 0)[SNew(SButton).Text(this, &SCursorPngImportDialog::GetImportText).IsEnabled(this, &SCursorPngImportDialog::CanImport).OnClicked(this, &SCursorPngImportDialog::Import)]
				+ SUniformGridPanel::Slot(1, 0)[SNew(SButton).Text(INVTEXT("Cancel")).OnClicked(this, &SCursorPngImportDialog::Cancel)]
			]
		]
	];
}

ECheckBoxState SCursorPngImportDialog::GetModeState(const ECursorPngImportMode InMode) const { return Mode == InMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }

void SCursorPngImportDialog::SetMode(const ECheckBoxState InState, const ECursorPngImportMode InMode)
{
	if (InState != ECheckBoxState::Checked || Mode == InMode) { return; }
	if (InMode == ECursorPngImportMode::Multiple)
	{
		HotspotX = ScaleHotspotAxis(HotspotX, TargetSize, SourceSize.X);
		HotspotY = ScaleHotspotAxis(HotspotY, TargetSize, SourceSize.Y);
	}
	else
	{
		HotspotX = ScaleHotspotAxis(HotspotX, SourceSize.X, TargetSize);
		HotspotY = ScaleHotspotAxis(HotspotY, SourceSize.Y, TargetSize);
	}
	Mode = InMode;
	if (GConfig)
	{
		GConfig->SetInt(ModeConfigSection, ModeConfigKey, static_cast<int32>(Mode), GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}
	HotspotX = FMath::Clamp(HotspotX, 0, FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.X) - 1));
	HotspotY = FMath::Clamp(HotspotY, 0, FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.Y) - 1));
}

EVisibility SCursorPngImportDialog::GetSingleSizeVisibility() const { return Mode == ECursorPngImportMode::Single ? EVisibility::Visible : EVisibility::Collapsed; }
EVisibility SCursorPngImportDialog::GetMultipleSizesVisibility() const { return Mode == ECursorPngImportMode::Multiple ? EVisibility::Visible : EVisibility::Collapsed; }

TSharedRef<SWidget> SCursorPngImportDialog::MakeSizePicker()
{
	TSharedRef<SVerticalBox> picker = SNew(SVerticalBox);
	for (const int32 size : SupportedSizes)
	{
		const bool enabled = IsSizeEnabled(size);
		picker->AddSlot().AutoHeight()
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this, size] { return GetSizeState(size); })
			.IsEnabled(enabled)
			.ToolTipText(enabled ? FText::GetEmpty() : INVTEXT("This size is larger than the source image and would require upscaling."))
			.OnCheckStateChanged_Lambda([this, size](const ECheckBoxState state) { SetSizeState(state, size); })
			[SNew(STextBlock).Text(FText::Format(INVTEXT("{0} x {0}{1}"), size, ExistingSizes.Contains(size) ? INVTEXT(" (existing, will replace)") : FText::GetEmpty()))]
		];
	}
	return picker;
}

ECheckBoxState SCursorPngImportDialog::GetSizeState(const int32 InSize) const { return SelectedSizes.Contains(InSize) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }
void SCursorPngImportDialog::SetSizeState(const ECheckBoxState InState, const int32 InSize)
{
	if (!IsSizeEnabled(InSize)) { return; }
	if (InState == ECheckBoxState::Checked) { SelectedSizes.Add(InSize); }
	else if (InState == ECheckBoxState::Unchecked) { SelectedSizes.Remove(InSize); }
}
bool SCursorPngImportDialog::IsSizeEnabled(const int32 InSize) const { return InSize >= 1 && InSize <= FMath::Min(SourceSize.X, 256) && InSize <= FMath::Min(SourceSize.Y, 256); }

FText SCursorPngImportDialog::GetSelectedSizesText() const
{
	FString summary;
	for (const int32 size : SupportedSizes)
	{
		if (SelectedSizes.Contains(size))
		{
			if (!summary.IsEmpty()) { summary += TEXT(", "); }
			summary += FString::FromInt(size);
		}
	}
	return summary.IsEmpty() ? INVTEXT("No sizes selected") : FText::FromString(summary);
}

TArray<FCursorPngImportRequest> SCursorPngImportDialog::BuildRequests() const
{
	TArray<FCursorPngImportRequest> requests;
	if (Mode == ECursorPngImportMode::Single)
	{
		requests.Add({TargetSize, FIntPoint(HotspotX, HotspotY)});
		return requests;
	}
	for (const int32 size : SupportedSizes)
	{
		if (SelectedSizes.Contains(size))
		{
			requests.Add({size, FIntPoint(ScaleHotspotAxis(HotspotX, SourceSize.X, size), ScaleHotspotAxis(HotspotY, SourceSize.Y, size))});
		}
	}
	return requests;
}

FText SCursorPngImportDialog::GetHotspotLabelX() const { return Mode == ECursorPngImportMode::Single ? INVTEXT("Hotspot X") : INVTEXT("Source hotspot X"); }
FText SCursorPngImportDialog::GetHotspotLabelY() const { return Mode == ECursorPngImportMode::Single ? INVTEXT("Hotspot Y") : INVTEXT("Source hotspot Y"); }
FText SCursorPngImportDialog::GetImportText() const { return Mode == ECursorPngImportMode::Multiple ? FText::Format(INVTEXT("Import {0} Sizes"), SelectedSizes.Num()) : INVTEXT("Import"); }

TOptional<int32> SCursorPngImportDialog::GetTargetSize() const { return TargetSize; }
TOptional<int32> SCursorPngImportDialog::GetHotspotX() const { return HotspotX; }
TOptional<int32> SCursorPngImportDialog::GetHotspotY() const { return HotspotY; }
void SCursorPngImportDialog::SetTargetSize(const int32 InValue)
{
	TargetSize = InValue;
	HotspotX = FMath::Clamp(HotspotX, 0, FMath::Max(0, TargetSize - 1));
	HotspotY = FMath::Clamp(HotspotY, 0, FMath::Max(0, TargetSize - 1));
}
void SCursorPngImportDialog::SetHotspotX(const int32 InValue) { HotspotX = FMath::Clamp(InValue, 0, FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.X) - 1)); }
void SCursorPngImportDialog::SetHotspotY(const int32 InValue) { HotspotY = FMath::Clamp(InValue, 0, FMath::Max(0, (Mode == ECursorPngImportMode::Single ? TargetSize : SourceSize.Y) - 1)); }

bool SCursorPngImportDialog::CanImport() const
{
	if (SourceSize.X < 1 || SourceSize.Y < 1 || SourceSize.X != SourceSize.Y) { return false; }
	if (Mode == ECursorPngImportMode::Single)
	{
		return TargetSize >= 1 && TargetSize <= FMath::Min(SourceSize.X, 256) && HotspotX >= 0 && HotspotX < TargetSize && HotspotY >= 0 && HotspotY < TargetSize;
	}
	return !SelectedSizes.IsEmpty() && HotspotX >= 0 && HotspotX < SourceSize.X && HotspotY >= 0 && HotspotY < SourceSize.Y;
}

FReply SCursorPngImportDialog::Import()
{
	if (CanImport() && OnImport)
	{
		OnImport(BuildRequests());
		if (Window.IsValid()) { Window->RequestDestroyWindow(); }
	}
	return FReply::Handled();
}
FReply SCursorPngImportDialog::Cancel()
{
	if (Window.IsValid()) { Window->RequestDestroyWindow(); }
	return FReply::Handled();
}

FText SCursorPngImportDialog::GetWarnings() const
{
	FString warnings;
	if (Mode == ECursorPngImportMode::Single)
	{
		if (SourceSize.X > 256) { warnings += TEXT("CUR supports at most 256x256; the target must be resized.\n"); }
		if (TargetSize != SourceSize.X) { warnings += FString::Printf(TEXT("Image will be resized from %dx%d to %dx%d.\n"), SourceSize.X, SourceSize.Y, TargetSize, TargetSize); }
		if (ExistingSizes.Contains(TargetSize)) { warnings += TEXT("This size already exists and will be replaced."); }
		return FText::FromString(warnings);
	}
	if (SelectedSizes.IsEmpty()) { return INVTEXT("Select at least one target size."); }
	int32 resizeCount = 0;
	FString replacedSizes;
	for (const int32 size : SupportedSizes)
	{
		if (!SelectedSizes.Contains(size)) { continue; }
		if (size != SourceSize.X) { ++resizeCount; }
		if (ExistingSizes.Contains(size))
		{
			if (!replacedSizes.IsEmpty()) { replacedSizes += TEXT(", "); }
			replacedSizes += FString::FromInt(size);
		}
	}
	if (resizeCount > 0) { warnings += FString::Printf(TEXT("%d selected size(s) will be resized from the source.\n"), resizeCount); }
	if (FMath::Min(SourceSize.X, SourceSize.Y) < 128) { warnings += TEXT("Larger presets are disabled to prevent upscaling.\n"); }
	if (!replacedSizes.IsEmpty()) { warnings += FString::Printf(TEXT("Existing sizes that will be replaced: %s."), *replacedSizes); }
	return FText::FromString(warnings);
}
