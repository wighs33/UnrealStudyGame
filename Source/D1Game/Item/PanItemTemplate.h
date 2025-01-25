#pragma once

#include "PanItemTemplate.generated.h"

class UPanItemInstance;

// 아이템 프래그먼트
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class UPanItemFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void OnInstanceCreated(UPanItemInstance* Instance) const { }
};

// 아이템 양식
UCLASS(Blueprintable, Const, Abstract)
class UPanItemTemplate : public UObject
{
	GENERATED_BODY()
	
public:
	UPanItemTemplate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure="false", meta=(DeterminesOutputType="FragmentClass"))
	const UPanItemFragment* FindFragmentByClass(TSubclassOf<UPanItemFragment> FragmentClass) const;

	template <typename FragmentClass>
	const FragmentClass* FindFragmentByClass() const
	{
		return (FragmentClass*)FindFragmentByClass(FragmentClass::StaticClass());
	}
	
public:
	// 슬롯 개수
	UPROPERTY(EditDefaultsOnly)
	FIntPoint SlotCount = FIntPoint::ZeroValue;

	// 최대 스택
	UPROPERTY(EditDefaultsOnly)
	int32 MaxStackCount = 1;
	
	// 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
	
	// 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;
	
	// 아이콘 텍스처
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconTexture;

	// 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> PickupableMesh;

	// 프래그먼트 모음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<UPanItemFragment>> Fragments;
};
