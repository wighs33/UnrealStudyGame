#include "PanNumberPopWidgetDefault.h"

#include "LyraLogChannels.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PanNumberPopWidgetDefault)

UPanNumberPopWidgetDefault::UPanNumberPopWidgetDefault(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UPanNumberPopWidgetDefault::InitializeUI(int32 InNumberToDisplay, FLinearColor InColor)
{
	Super::InitializeUI(InNumberToDisplay, InColor);
	
	Text_Number->SetColorAndOpacity(InColor);
	Text_Number->SetText(FText::AsNumber(InNumberToDisplay));
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		PlayAnimation(Animation_NumberPop);
	});
}
