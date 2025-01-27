#include "PanEquipmentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Data/PanItemData.h"
#include "Item/Fragments/PanItemFragment_Equipable_Weapon.h"
#include "Item/Managers/PanEquipManagerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanEquipmentBase)

APanEquipmentBase::APanEquipmentBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
    bReplicates = true;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
	ArrowComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	SetRootComponent(ArrowComponent);
	
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	MeshComponent->SetCollisionProfileName("Weapon");
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	
	TraceDebugCollision = CreateDefaultSubobject<UBoxComponent>("TraceDebugCollision");
	TraceDebugCollision->SetCollisionProfileName("NoCollision");
	TraceDebugCollision->SetGenerateOverlapEvents(false);
	TraceDebugCollision->SetupAttachment(GetRootComponent());
	TraceDebugCollision->PrimaryComponentTick.bStartWithTickEnabled = false;
}

void APanEquipmentBase::BeginPlay()
{
	Super::BeginPlay();

	if (bOnlyUseForLocal)
		return;
	
	if (HasAuthority())
	{
		OnRep_EquipmentSlotType();
	}
}

void APanEquipmentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (bOnlyUseForLocal)
		return;
	
	DOREPLIFETIME(ThisClass, TemplateID);
	DOREPLIFETIME(ThisClass, EquipmentSlotType);
	DOREPLIFETIME(ThisClass, bCanBlock);
}

void APanEquipmentBase::Destroyed()
{
	if (bOnlyUseForLocal)
		return;
	
	if (ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwner()))
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(Character->GetAbilitySystemComponent()))
		{
			SkillAbilitySetHandles.TakeFromAbilitySystem(ASC);
		}

		if (UPanEquipManagerComponent* EquipManager = Character->FindComponentByClass<UPanEquipManagerComponent>())
		{
			TArray<FPanEquipEntry>& Entries = EquipManager->GetAllEntries();
			if (Entries[(int32)EquipmentSlotType].GetEquipmentActor() == this)
			{
				Entries[(int32)EquipmentSlotType].SetEquipmentActor(nullptr);
			}
		}
	}
	
	Super::Destroyed();
}

void APanEquipmentBase::Init(int32 InTemplateID, EEquipmentSlotType InEquipmentSlotType)
{
	if (bOnlyUseForLocal)
		return;
	
	TemplateID = InTemplateID;
	EquipmentSlotType = InEquipmentSlotType;
}

void APanEquipmentBase::ChangeBlockState(bool bShouldBlock)
{
	if (bOnlyUseForLocal)
		return;
	
	if (HasAuthority())
	{
		bCanBlock = bShouldBlock;
		OnRep_CanBlock();
	}
}

void APanEquipmentBase::OnRep_CanBlock()
{
	if (bOnlyUseForLocal)
		return;
	
	MeshComponent->SetCollisionResponseToChannel(Pan_ObjectChannel_Projectile, bCanBlock ? ECR_Block : ECR_Ignore);
}

void APanEquipmentBase::OnRep_EquipmentSlotType()
{
	if (bOnlyUseForLocal)
		return;
	
	if (GetOwner() && GetOwner()->FindComponentByClass<UPanEquipManagerComponent>())
	{
		if (ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwner()))
		{
			if (UPanEquipManagerComponent* EquipManager = Character->FindComponentByClass<UPanEquipManagerComponent>())
			{
				TArray<FPanEquipEntry>& Entries = EquipManager->GetAllEntries();
				Entries[(int32)EquipmentSlotType].SetEquipmentActor(this);
			}
		}
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::OnRep_EquipmentSlotType);
	}
}

UAbilitySystemComponent* APanEquipmentBase::GetAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = nullptr;
	if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwner()))
	{
		ASC = LyraCharacter->GetAbilitySystemComponent();
	}
	return ASC;
}

UAnimMontage* APanEquipmentBase::GetEquipMontage()
{
	UAnimMontage* EquipMontage = nullptr;
	
	if (TemplateID > 0)
	{
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(TemplateID);
		if (const UPanItemFragment_Equipable_Attachment* AttachmentFragment = ItemTemplate.FindFragmentByClass<UPanItemFragment_Equipable_Attachment>())
		{
			EquipMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->EquipMontage);
		}
	}
	
	return EquipMontage;
}

UAnimMontage* APanEquipmentBase::GetHitMontage(AActor* InstigatorActor, const FVector& HitLocation, bool IsBlocked)
{
	UAnimMontage* SelectedMontage = nullptr;
	
	if (InstigatorActor && TemplateID > 0)
	{
		const UPanItemTemplate& ItemTemplate = UPanItemData::Get().FindItemTemplateByID(TemplateID);
		if (const UPanItemFragment_Equipable_Attachment* AttachmentFragment = ItemTemplate.FindFragmentByClass<UPanItemFragment_Equipable_Attachment>())
		{
			if (IsBlocked)
			{
				SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->BlockHitMontage);
			}
			else
			{
				AActor* CharacterActor = GetOwner();
				const FVector& CharacterLocation = CharacterActor->GetActorLocation();
				const FVector& CharacterDirection = CharacterActor->GetActorForwardVector();
			
				const FRotator& FacingRotator = UKismetMathLibrary::Conv_VectorToRotator(CharacterDirection);
				const FRotator& CharacterToHitRotator = UKismetMathLibrary::Conv_VectorToRotator((HitLocation - CharacterLocation).GetSafeNormal());
			
				const FRotator& DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(CharacterToHitRotator, FacingRotator);
				float YawAbs = FMath::Abs(DeltaRotator.Yaw);

				if (YawAbs < 60.f)
				{
					SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->FrontHitMontage);
				}
				else if (YawAbs > 120.f)
				{
					SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->BackHitMontage);
				}
				else if (DeltaRotator.Yaw < 0.f)
				{
					SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->LeftHitMontage);
				}
				else
				{
					SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->RightHitMontage);
				}
			}
		}
	}
	
	return SelectedMontage;
}
