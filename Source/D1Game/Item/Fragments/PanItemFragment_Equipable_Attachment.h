#pragma once

#include "PanItemFragment_Equipable.h"
#include "PanItemFragment_Equipable_Attachment.generated.h"

class APanEquipmentBase;

// 무기 부착 정보
USTRUCT(BlueprintType)
struct FPanWeaponAttachInfo
{
	GENERATED_BODY()

public:
	// 스폰 무기 클래스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APanEquipmentBase> SpawnWeaponClass;

	// 부착 소켓 이름
	UPROPERTY(EditDefaultsOnly)
	FName AttachSocket;

	// 부착 트랜스폼
	UPROPERTY(EditDefaultsOnly)
	FTransform AttachTransform;
};

// 장착가능한 아이템 프래그먼트 (부착)
UCLASS(Abstract, Const)
class UPanItemFragment_Equipable_Attachment : public UPanItemFragment_Equipable
{
	GENERATED_BODY()
	
public:
	UPanItemFragment_Equipable_Attachment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	// 무기 핸드 타입
	UPROPERTY(EditDefaultsOnly)
	EWeaponHandType WeaponHandType = EWeaponHandType::Count;

	// 무기 부착 정보
	UPROPERTY(EditDefaultsOnly)
	FPanWeaponAttachInfo WeaponAttachInfo;

	// 장착하는 몽타주
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> FrontHitMontage;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> BackHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> LeftHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> RightHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> BlockHitMontage;

	// 포켓 월드 아이들 몽타주
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> PocketWorldIdleMontage;
	
	// 애님 인스턴스 클래스
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> AnimInstanceClass;
};
