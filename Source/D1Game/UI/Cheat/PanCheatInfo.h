#pragma once

#include "Blueprint/UserWidget.h"
#include "PanCheatInfo.generated.h"

class UTextBlock;

UCLASS()
class UPanCheatInfo : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPanCheatInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_AnimationName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_AnimationSpeed;
};
