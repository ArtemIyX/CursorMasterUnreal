#include "CursorMasterAssetEditor.h"

#include "Assets/HardwareCursorAsset.h"
#include "Assets/HardwareCursorCollectionAsset.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "IDetailsView.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"
#include "Libs/CursorMasterLib.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SCursorPngImportDialog.h"
#include "ScopedTransaction.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	TSharedRef<SWidget> MakeEditorBody(const FText& InText, const FLinearColor& InColor)
	{
		return SNew(SBorder).BorderBackgroundColor(InColor).Padding(24.0f)
		[
			SNew(STextBlock).Text(InText).ColorAndOpacity(FLinearColor::White)
		];
	}
}

const FName FHardwareCursorAssetEditor::EditorTabId(TEXT("HardwareCursorAssetEditor_Main"));
const FName FHardwareCursorCollectionAssetEditor::EditorTabId(TEXT("HardwareCursorCollectionAssetEditor_Main"));

void SHardwareCursorAssetEditor::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;
	RefreshEntries();
	ChildSlot[
		SNew(SBorder).Padding(12.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(INVTEXT("Import PNG")).OnClicked(this, &SHardwareCursorAssetEditor::ImportPng)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)[SNew(SButton).Text(INVTEXT("Delete selected")).IsEnabled(this, &SHardwareCursorAssetEditor::HasSelection).OnClicked(this, &SHardwareCursorAssetEditor::DeleteSelected)]
			]
			+ SVerticalBox::Slot().FillHeight(1).Padding(0, 12, 0, 0)[
				SAssignNew(ListView, SListView<TSharedPtr<FCursorPreviewEntry>>)
				.ListItemsSource(&Entries)
				.OnGenerateRow(this, &SHardwareCursorAssetEditor::MakeEntryRow)
				.OnSelectionChanged(this, &SHardwareCursorAssetEditor::SelectEntry)
			]
		]
	];
}

void SHardwareCursorAssetEditor::RefreshEntries()
{
	Entries.Reset();
	if (const UHardwareCursorAsset* asset = Asset.Get())
	{
		for (const FHardwareCursorSize& entry : asset->Sizes)
		{
			TSharedPtr<FCursorPreviewEntry> preview = MakeShared<FCursorPreviewEntry>();
			preview->Cursor = entry;
			preview->Texture = CreatePreviewTexture(entry);
			preview->Brush.SetResourceObject(preview->Texture.Get());
			preview->Brush.ImageSize = FVector2D(48.0f, 48.0f);
			Entries.Add(MoveTemp(preview));
		}
	}
	SelectedEntry.Reset();
	if (ListView.IsValid()) { ListView->RequestListRefresh(); }
}

TSharedRef<ITableRow> SHardwareCursorAssetEditor::MakeEntryRow(TSharedPtr<FCursorPreviewEntry> InEntry, const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(STableRow<TSharedPtr<FCursorPreviewEntry>>, InOwnerTable)[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SImage).Image(&InEntry->Brush)]
		+ SHorizontalBox::Slot().FillWidth(1).Padding(10, 0, 0, 0)[SNew(STextBlock).Text(FText::Format(INVTEXT("{0} x {0}    Hotspot: ({1}, {2})    CUR: {3} bytes"), InEntry->Cursor.Size, InEntry->Cursor.Hotspot.X, InEntry->Cursor.Hotspot.Y, InEntry->Cursor.CurBytes.Num()))]
	];
}

void SHardwareCursorAssetEditor::SelectEntry(TSharedPtr<FCursorPreviewEntry> InEntry, ESelectInfo::Type InSelectInfo) { SelectedEntry = InEntry; }
bool SHardwareCursorAssetEditor::HasSelection() const { return SelectedEntry.IsValid(); }

TStrongObjectPtr<UTexture2D> SHardwareCursorAssetEditor::CreatePreviewTexture(const FHardwareCursorSize& InEntry) const
{
	if (InEntry.CurBytes.Num() < 22) { return nullptr; }
	const uint32 pngSize = InEntry.CurBytes[14] | (InEntry.CurBytes[15] << 8) | (InEntry.CurBytes[16] << 16) | (InEntry.CurBytes[17] << 24);
	const uint32 pngOffset = InEntry.CurBytes[18] | (InEntry.CurBytes[19] << 8) | (InEntry.CurBytes[20] << 16) | (InEntry.CurBytes[21] << 24);
	if (pngOffset > static_cast<uint32>(InEntry.CurBytes.Num()) || pngSize > static_cast<uint32>(InEntry.CurBytes.Num()) - pngOffset) { return nullptr; }
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	const TSharedPtr<IImageWrapper> image = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	TArray<uint8> rawBytes;
	if (!image.IsValid() || !image->SetCompressed(InEntry.CurBytes.GetData() + pngOffset, pngSize) || !image->GetRaw(ERGBFormat::BGRA, 8, rawBytes)) { return nullptr; }
	const int32 width = image->GetWidth();
	const int32 height = image->GetHeight();
	if (width < 1 || height < 1 || rawBytes.Num() != width * height * sizeof(FColor)) { return nullptr; }
	TStrongObjectPtr<UTexture2D> texture(UTexture2D::CreateTransient(width, height, PF_B8G8R8A8));
	if (!texture.IsValid()) { return nullptr; }
	void* pixels = texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(pixels, rawBytes.GetData(), rawBytes.Num());
	texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	texture->NeverStream = true;
	texture->UpdateResource();
	return texture;
}

FReply SHardwareCursorAssetEditor::ImportPng()
{
	IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
	if (!desktopPlatform) { return FReply::Handled(); }
	TArray<FString> files;
	if (!desktopPlatform->OpenFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), TEXT("Import PNG Cursor"), FString(), FString(), TEXT("PNG files (*.png)|*.png"), EFileDialogFlags::None, files) || files.IsEmpty()) { return FReply::Handled(); }
	TArray<uint8> pngBytes;
	if (!FFileHelper::LoadFileToArray(pngBytes, *files[0])) { FMessageDialog::Open(EAppMsgType::Ok, INVTEXT("Could not read the selected PNG.")); return FReply::Handled(); }
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	const TSharedPtr<IImageWrapper> image = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!image.IsValid() || !image->SetCompressed(pngBytes.GetData(), pngBytes.Num())) { FMessageDialog::Open(EAppMsgType::Ok, INVTEXT("The selected file is not a valid PNG.")); return FReply::Handled(); }
	const FIntPoint sourceSize(image->GetWidth(), image->GetHeight());
	if (sourceSize.X < 1 || sourceSize.Y < 1 || sourceSize.X != sourceSize.Y) { FMessageDialog::Open(EAppMsgType::Ok, INVTEXT("Cursor PNGs must be square.")); return FReply::Handled(); }
	TArray<int32> existingSizes;
	if (const UHardwareCursorAsset* asset = Asset.Get()) { for (const FHardwareCursorSize& entry : asset->Sizes) { existingSizes.Add(entry.Size); } }
	SCursorPngImportDialog::Open(sourceSize, MoveTemp(existingSizes), [this, pngBytes = MoveTemp(pngBytes), sourceSize](int32 targetSize, FIntPoint hotspot) mutable { ImportPngBytes(MoveTemp(pngBytes), sourceSize, targetSize, hotspot); });
	return FReply::Handled();
}

void SHardwareCursorAssetEditor::ImportPngBytes(TArray<uint8> InPngBytes, FIntPoint InSourceSize, int32 InTargetSize, FIntPoint InHotspot)
{
	if (!Asset.IsValid()) { return; }
	TArray<uint8> pngBytes = MoveTemp(InPngBytes);
	if (InSourceSize.X != InTargetSize)
	{
		IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> decoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		TArray<uint8> rawBytes;
		if (!decoder.IsValid() || !decoder->SetCompressed(pngBytes.GetData(), pngBytes.Num()) || !decoder->GetRaw(ERGBFormat::BGRA, 8, rawBytes)) { FMessageDialog::Open(EAppMsgType::Ok, INVTEXT("Could not decode the selected PNG.")); return; }
		TArray<FColor> sourcePixels;
		sourcePixels.SetNumUninitialized(InSourceSize.X * InSourceSize.Y);
		FMemory::Memcpy(sourcePixels.GetData(), rawBytes.GetData(), rawBytes.Num());
		TArray<FColor> resizedPixels;
		FImageUtils::ImageResize(InSourceSize.X, InSourceSize.Y, sourcePixels, InTargetSize, InTargetSize, resizedPixels, false);
		const TSharedPtr<IImageWrapper> encoder = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (!encoder.IsValid() || !encoder->SetRaw(resizedPixels.GetData(), resizedPixels.Num() * sizeof(FColor), InTargetSize, InTargetSize, ERGBFormat::BGRA, 8)) { FMessageDialog::Open(EAppMsgType::Ok, INVTEXT("Could not resize the selected PNG.")); return; }
		const TArray64<uint8> encodedBytes = encoder->GetCompressed();
		pngBytes.Reset();
		pngBytes.Append(encodedBytes.GetData(), encodedBytes.Num());
	}
	TArray<uint8> curBytes;
	FString error;
	if (!UCursorMasterLib::ConvertPngBytesToCur(pngBytes, InHotspot, curBytes, error)) { FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(error)); return; }
	const FScopedTransaction transaction(INVTEXT("Import Hardware Cursor"));
	UHardwareCursorAsset* asset = Asset.Get();
	asset->Modify();
	FHardwareCursorSize* entry = asset->Sizes.FindByPredicate([InTargetSize](const FHardwareCursorSize& candidate) { return candidate.Size == InTargetSize; });
	if (!entry) { entry = &asset->Sizes.AddDefaulted_GetRef(); entry->Size = InTargetSize; }
	entry->Hotspot = InHotspot;
	entry->CurBytes = MoveTemp(curBytes);
	asset->Sizes.Sort([](const FHardwareCursorSize& left, const FHardwareCursorSize& right) { return left.Size < right.Size; });
	asset->PostEditChange();
	asset->MarkPackageDirty();
	RefreshEntries();
}

FReply SHardwareCursorAssetEditor::DeleteSelected()
{
	if (!Asset.IsValid() || !SelectedEntry.IsValid()) { return FReply::Handled(); }
	const int32 size = SelectedEntry->Cursor.Size;
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(INVTEXT("Delete the {0} x {0} cursor?"), size)) != EAppReturnType::Yes) { return FReply::Handled(); }
	const FScopedTransaction transaction(INVTEXT("Delete Hardware Cursor"));
	UHardwareCursorAsset* asset = Asset.Get();
	asset->Modify();
	asset->Sizes.RemoveAll([size](const FHardwareCursorSize& entry) { return entry.Size == size; });
	asset->PostEditChange();
	asset->MarkPackageDirty();
	RefreshEntries();
	return FReply::Handled();
}
void SHardwareCursorCollectionAssetEditor::Construct(const FArguments& InArgs)
{
	FDetailsViewArgs args;
	args.bAllowSearch = false;
	args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	const TSharedRef<IDetailsView> details = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor")).CreateDetailView(args);
	details->SetObject(InArgs._Asset);
	ChildSlot[details];
}

void FHardwareCursorAssetEditor::Init(UHardwareCursorAsset* InAsset, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	Asset = InAsset;
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TEXT("HardwareCursorAssetEditor_Layout"))->AddArea(
		FTabManager::NewPrimaryArea()->Split(FTabManager::NewStack()->AddTab(EditorTabId, ETabState::OpenedTab)));
	InitAssetEditor(EToolkitMode::Standalone, InToolkitHost, GetToolkitFName(), Layout, true, true, InAsset);
}

FText FHardwareCursorAssetEditor::GetBaseToolkitName() const { return NSLOCTEXT("CursorMaster", "CursorAssetEditorName", "Cursor Asset"); }

void FHardwareCursorAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	InTabManager->RegisterTabSpawner(EditorTabId, FOnSpawnTab::CreateSP(this, &FHardwareCursorAssetEditor::SpawnEditorTab)).SetDisplayName(GetBaseToolkitName());
}

void FHardwareCursorAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(EditorTabId);
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

TSharedRef<SDockTab> FHardwareCursorAssetEditor::SpawnEditorTab(const FSpawnTabArgs& InArgs)
{
	return SNew(SDockTab).Label(GetBaseToolkitName())[
		SNew(SHardwareCursorAssetEditor).Asset(Asset.Get())
	];
}

void FHardwareCursorCollectionAssetEditor::Init(UHardwareCursorCollectionAsset* InAsset, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	Asset = InAsset;
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TEXT("HardwareCursorCollectionAssetEditor_Layout"))->AddArea(
		FTabManager::NewPrimaryArea()->Split(FTabManager::NewStack()->AddTab(EditorTabId, ETabState::OpenedTab)));
	InitAssetEditor(EToolkitMode::Standalone, InToolkitHost, GetToolkitFName(), Layout, true, true, InAsset);
}

FText FHardwareCursorCollectionAssetEditor::GetBaseToolkitName() const { return NSLOCTEXT("CursorMaster", "CursorCollectionAssetEditorName", "Cursor Collection Asset"); }

void FHardwareCursorCollectionAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	InTabManager->RegisterTabSpawner(EditorTabId, FOnSpawnTab::CreateSP(this, &FHardwareCursorCollectionAssetEditor::SpawnEditorTab)).SetDisplayName(GetBaseToolkitName());
}

void FHardwareCursorCollectionAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(EditorTabId);
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

TSharedRef<SDockTab> FHardwareCursorCollectionAssetEditor::SpawnEditorTab(const FSpawnTabArgs& InArgs)
{
	return SNew(SDockTab).Label(GetBaseToolkitName())[
		SNew(SHardwareCursorCollectionAssetEditor).Asset(Asset.Get())
	];
}
