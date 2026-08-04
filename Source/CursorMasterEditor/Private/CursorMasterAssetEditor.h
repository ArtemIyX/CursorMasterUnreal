#pragma once

#include "Assets/HardwareCursorAsset.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UHardwareCursorAsset;
class UHardwareCursorCollectionAsset;
class SDockTab;
class FSpawnTabArgs;
class UTexture2D;

struct FCursorPreviewEntry
{
	FHardwareCursorSize Cursor;
	TStrongObjectPtr<UTexture2D> Texture;
	FSlateBrush Brush;
};

class SHardwareCursorAssetEditor final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHardwareCursorAssetEditor) {}
		SLATE_ARGUMENT(UHardwareCursorAsset*, Asset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void RefreshEntries();
	TSharedRef<ITableRow> MakeEntryRow(TSharedPtr<FCursorPreviewEntry> InEntry, const TSharedRef<STableViewBase>& InOwnerTable);
	void SelectEntry(TSharedPtr<FCursorPreviewEntry> InEntry, ESelectInfo::Type InSelectInfo);
	FReply ImportPng();
	FReply DeleteSelected();
	bool HasSelection() const;
	void ImportPngBytes(TArray<uint8> InPngBytes, FIntPoint InSourceSize, int32 InTargetSize, FIntPoint InHotspot);
	TStrongObjectPtr<UTexture2D> CreatePreviewTexture(const FHardwareCursorSize& InEntry) const;

	TWeakObjectPtr<UHardwareCursorAsset> Asset;
	TArray<TSharedPtr<FCursorPreviewEntry>> Entries;
	TSharedPtr<FCursorPreviewEntry> SelectedEntry;
	TSharedPtr<SListView<TSharedPtr<FCursorPreviewEntry>>> ListView;
};

class SHardwareCursorCollectionAssetEditor final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHardwareCursorCollectionAssetEditor) {}
		SLATE_ARGUMENT(UHardwareCursorCollectionAsset*, Asset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};

class FHardwareCursorAssetEditor final : public FAssetEditorToolkit
{
public:
	void Init(UHardwareCursorAsset* InAsset, const TSharedPtr<IToolkitHost>& InToolkitHost);
	virtual FName GetToolkitFName() const override { return TEXT("HardwareCursorAssetEditor"); }
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override { return TEXT("Hardware Cursor"); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.08f, 0.36f, 0.18f); }
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& InArgs);
	TWeakObjectPtr<UHardwareCursorAsset> Asset;
	static const FName EditorTabId;
};

class FHardwareCursorCollectionAssetEditor final : public FAssetEditorToolkit
{
public:
	void Init(UHardwareCursorCollectionAsset* InAsset, const TSharedPtr<IToolkitHost>& InToolkitHost);
	virtual FName GetToolkitFName() const override { return TEXT("HardwareCursorCollectionAssetEditor"); }
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override { return TEXT("Cursor Collection"); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.22f, 0.48f, 0.12f); }
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& InArgs);
	TWeakObjectPtr<UHardwareCursorCollectionAsset> Asset;
	static const FName EditorTabId;
};
