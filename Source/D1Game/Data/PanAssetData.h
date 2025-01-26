#pragma once

#include "PanAssetData.generated.h"

// 애셋 항목
USTRUCT()
struct FAssetEntry
{
	GENERATED_BODY()

public:
	// 애셋 이름
	UPROPERTY(EditDefaultsOnly)
	FName AssetName;
	
	// 애셋 경로
	UPROPERTY(EditDefaultsOnly)
	FSoftObjectPath AssetPath;
	
	// 애셋 라벨
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> AssetLabels;
};

// 애셋 모음
USTRUCT()
struct FAssetSet
{
	GENERATED_BODY()

public:
	// 애셋들
	UPROPERTY(EditDefaultsOnly)
	TArray<FAssetEntry> AssetEntries;
};

// 애셋 데이터
UCLASS(Const, CollapseCategories, meta=(DisplayName="Pan Asset Data"))
class UPanAssetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UPanAssetData& Get();
	
protected:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	
public:
	FSoftObjectPath GetAssetPathByName(const FName& AssetName) const;
	const FAssetSet& GetAssetSetByLabel(const FName& Label) const;
	
private:
	// [애셋 이름]-[애셋 모음] 리스트
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FAssetSet> AssetGroupNameToSet;
	
	// [애셋 이름]-[애셋 경로] 리스트
	UPROPERTY()
	TMap<FName, FSoftObjectPath> AssetNameToPath;

	// [애셋 라벨]-[애셋 모음] 리스트
	UPROPERTY()
	TMap<FName, FAssetSet> AssetLabelToSet;
};
