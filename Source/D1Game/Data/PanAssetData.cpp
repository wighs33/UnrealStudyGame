#include "PanAssetData.h"

#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanAssetData)

// 애셋 매니저에서 데이터 꺼내기
const UPanAssetData& UPanAssetData::Get()
{
	return ULyraAssetManager::Get().GetAssetData();
}

#if WITH_EDITOR
// 자동 저장 전
void UPanAssetData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	AssetNameToPath.Empty();
	AssetLabelToSet.Empty();

	AssetGroupNameToSet.KeySort([](const FName& A, const FName& B)
	{
		return (A.Compare(B) < 0);
	});
	
	for (const auto& Pair : AssetGroupNameToSet)
	{
		const FAssetSet& AssetSet = Pair.Value;
		for (FAssetEntry AssetEntry : AssetSet.AssetEntries)
		{
			// AssetNameToPath에 값 채우기
			AssetNameToPath.Emplace(AssetEntry.AssetName, AssetEntry.AssetPath);
			for (const FName& Label : AssetEntry.AssetLabels)
			{
				// AssetLabelToSet에 값 채우기
				AssetLabelToSet.FindOrAdd(Label).AssetEntries.Emplace(AssetEntry);
			}
		}
	}
}

// 유효성 체크
EDataValidationResult UPanAssetData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const auto& Pair : AssetGroupNameToSet)
	{
		const FAssetSet& AssetSet = Pair.Value;
		for (int32 i = 0; i < AssetSet.AssetEntries.Num(); i++)
		{
			const FAssetEntry& AssetEntry = AssetSet.AssetEntries[i];
			// 에셋 이름 있는 지 체크
			if (AssetEntry.AssetName.IsNone())
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Asset Name is None : [Group Name : %s] - [Entry Index : %d]"), *Pair.Key.ToString(), i)));
				Result = EDataValidationResult::Invalid;
			}
			// 올바른 경로인지 체크
			if (AssetEntry.AssetPath.IsValid() == false)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Asset Path is Invalid : [Group Name : %s] - [Entry Index : %d]"), *Pair.Key.ToString(), i)));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	return Result;
}
#endif // WITH_EDITOR

// 애셋 이름으로 경로 찾기
FSoftObjectPath UPanAssetData::GetAssetPathByName(const FName& AssetName) const
{
	const FSoftObjectPath* AssetPath = AssetNameToPath.Find(AssetName);
	ensureAlwaysMsgf(AssetPath, TEXT("Can't find Asset Path from Asset Name [%s]."), *AssetName.ToString());
	return *AssetPath;
}

// 애셋 라벨로 애셋 모음 찾기
const FAssetSet& UPanAssetData::GetAssetSetByLabel(const FName& Label) const
{
	const FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
	ensureAlwaysMsgf(AssetSet, TEXT("Can't find Asset Set from Label [%s]."), *Label.ToString());
	return *AssetSet;
}
