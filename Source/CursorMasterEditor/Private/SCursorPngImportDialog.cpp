#include "SCursorPngImportDialog.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

void SCursorPngImportDialog::Open(FIntPoint InSourceSize, TArray<int32> InExistingSizes, TFunction<void(int32, FIntPoint)> InOnImport)
{
	const TSharedRef<SWindow> window = SNew(SWindow)
		.Title(INVTEXT("Import PNG Cursor"))
		.ClientSize(FVector2D(430.0f, 250.0f))
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
	TargetSize = FMath::Max(SourceSize.X, 1);
	Window = InArgs._Window;
	OnImport = InArgs._OnImport;

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
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock).Text(INVTEXT("Target size"))
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>).Value(this, &SCursorPngImportDialog::GetTargetSize).MinValue(1).MaxValue(SourceSize.X).OnValueChanged(this, &SCursorPngImportDialog::SetTargetSize)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock).Text(INVTEXT("Hotspot X"))
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>).Value(this, &SCursorPngImportDialog::GetHotspotX).MinValue(0).OnValueChanged(this, &SCursorPngImportDialog::SetHotspotX)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock).Text(INVTEXT("Hotspot Y"))
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(SNumericEntryBox<int32>).Value(this, &SCursorPngImportDialog::GetHotspotY).MinValue(0).OnValueChanged(this, &SCursorPngImportDialog::SetHotspotY)
				]
			]
			+ SVerticalBox::Slot().FillHeight(1).Padding(0, 12, 0, 0)
			[
				SNew(STextBlock).Text(this, &SCursorPngImportDialog::GetWarnings).AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SUniformGridPanel).SlotPadding(FMargin(4, 0))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton).Text(INVTEXT("Import")).IsEnabled(this, &SCursorPngImportDialog::CanImport).OnClicked(this, &SCursorPngImportDialog::Import)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton).Text(INVTEXT("Cancel")).OnClicked(this, &SCursorPngImportDialog::Cancel)
				]
			]
		]
	];
}

TOptional<int32> SCursorPngImportDialog::GetTargetSize() const { return TargetSize; }
TOptional<int32> SCursorPngImportDialog::GetHotspotX() const { return HotspotX; }
TOptional<int32> SCursorPngImportDialog::GetHotspotY() const { return HotspotY; }

void SCursorPngImportDialog::SetTargetSize(int32 InValue)
{
	TargetSize = InValue;
	HotspotX = FMath::Clamp(HotspotX, 0, FMath::Max(0, TargetSize - 1));
	HotspotY = FMath::Clamp(HotspotY, 0, FMath::Max(0, TargetSize - 1));
}

void SCursorPngImportDialog::SetHotspotX(int32 InValue) { HotspotX = InValue; }
void SCursorPngImportDialog::SetHotspotY(int32 InValue) { HotspotY = InValue; }
bool SCursorPngImportDialog::CanImport() const { return TargetSize >= 1 && TargetSize <= FMath::Min(SourceSize.X, 256) && HotspotX >= 0 && HotspotX < TargetSize && HotspotY >= 0 && HotspotY < TargetSize; }

FReply SCursorPngImportDialog::Import()
{
	OnImport(TargetSize, FIntPoint(HotspotX, HotspotY));
	Window->RequestDestroyWindow();
	return FReply::Handled();
}

FReply SCursorPngImportDialog::Cancel()
{
	Window->RequestDestroyWindow();
	return FReply::Handled();
}

FText SCursorPngImportDialog::GetWarnings() const
{
	FString warnings;
	if (SourceSize.X > 256) { warnings += TEXT("CUR supports at most 256x256.\n"); }
	if (TargetSize > 256) { warnings += TEXT("Target size must not exceed 256 for CUR.\n"); }
	if (TargetSize != SourceSize.X) { warnings += FString::Printf(TEXT("Image will be resized from %dx%d to %dx%d.\n"), SourceSize.X, SourceSize.Y, TargetSize, TargetSize); }
	if (ExistingSizes.Contains(TargetSize)) { warnings += TEXT("This size already exists and will be replaced."); }
	return FText::FromString(warnings);
}
