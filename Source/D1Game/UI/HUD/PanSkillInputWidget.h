#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PanSkillInputWidget.generated.h"

USTRUCT(BlueprintType)
struct FSkillInputInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bShouldShow = false;
};

UCLASS()
class UPanSkillInputWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanSkillInputWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void ConstructUI(FGameplayTag Channel, const FSkillInputInitializeMessage& Message);

protected:
	UPROPERTY(EditAnywhere, meta=(Categories="Message"))
	FGameplayTag MessageChannelTag;
	
private:
	FGameplayMessageListenerHandle ListenerHandle;
};
