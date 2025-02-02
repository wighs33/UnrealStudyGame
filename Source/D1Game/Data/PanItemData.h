﻿#pragma once

#include "PanItemData.generated.h"

class UPanItemTemplate;

// 아이템 데이터 (싱글턴)
UCLASS(BlueprintType, Const, meta=(DisplayName="Pan Item Data"))
class UPanItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UPanItemData& Get();

public:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITORONLY_DATA
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
	
public:
	const UPanItemTemplate& FindItemTemplateByID(int32 ItemTemplateID) const;
	int32 FindItemTemplateIDByClass(TSubclassOf<UPanItemTemplate> ItemTemplateClass) const;
	
	void GetAllItemTemplateClasses(TArray<TSubclassOf<UPanItemTemplate>>& OutItemTemplateClasses) const;
	const TArray<TSubclassOf<UPanItemTemplate>>& GetWeaponItemTemplateClasses() const { return WeaponItemTemplateClasses; }
	const TArray<TSubclassOf<UPanItemTemplate>>& GetArmorItemTemplateClasses() const { return ArmorItemTemplateClasses; }
	
private:
	// [아이템 아이디] - [아이템 템플릿 클래스] 리스트
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, TSubclassOf<UPanItemTemplate>> ItemTemplateIDToClass;

	// [아이템 템플릿 클래스] - [아이템 아이디] 리스트
	UPROPERTY()
	TMap<TSubclassOf<UPanItemTemplate>, int32> ItemTemplateClassToID;

private:
	// 무기 모음
	UPROPERTY()
	TArray<TSubclassOf<UPanItemTemplate>> WeaponItemTemplateClasses;

	// 갑옷 모음
	UPROPERTY()
	TArray<TSubclassOf<UPanItemTemplate>> ArmorItemTemplateClasses;
};
