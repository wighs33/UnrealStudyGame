#include "PanSkillInputWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanSkillInputWidget)

UPanSkillInputWidget::UPanSkillInputWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanSkillInputWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = MessageSubsystem.RegisterListener(MessageChannelTag, this, &ThisClass::ConstructUI);
}

void UPanSkillInputWidget::NativeDestruct()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(ListenerHandle);
	
	Super::NativeDestruct();
}

void UPanSkillInputWidget::ConstructUI(FGameplayTag Channel, const FSkillInputInitializeMessage& Message)
{
	Message.bShouldShow ? SetVisibility(ESlateVisibility::Visible) : SetVisibility(ESlateVisibility::Hidden);
}
