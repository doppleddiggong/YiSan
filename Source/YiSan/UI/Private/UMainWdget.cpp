#include "UMainWdget.h"

#include "GameLogging.h"
#include "Components/EditableTextBox.h"
#include "LatteLibrary/Character/Public/APlayerActor.h"
#include "LatteLibrary/Character/Public/UVoiceCaptureComponent.h"

void UMainWdget::NativeConstruct()
{
	Super::NativeConstruct();

	if (VoiceInputBox)
	{
		VoiceInputBox->OnTextCommitted.AddDynamic(this, &UMainWdget::HandleTextCommitted);
	}

	PRINTLOG( TEXT("UMainWdget::NativeConstruct"));
}

void UMainWdget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	FString CleanText = Text.ToString();
	CleanText.TrimStartAndEndInline();

	if (CleanText.IsEmpty())
	{
		return;
	}

	PRINTLOG(TEXT("[UMainWdget] Enter committed text: %s"), *CleanText);

	if (APlayerActor* PlayerActor = Cast<APlayerActor>(GetOwningPlayerPawn()))
	{
		if (UVoiceCaptureComponent* VoiceComponent = PlayerActor->FindComponentByClass<UVoiceCaptureComponent>())
		{
			VoiceComponent->RequestSynthesis(CleanText);
		}
	}

	if (VoiceInputBox)
	{
		VoiceInputBox->SetText(FText::GetEmpty());
	}
}
