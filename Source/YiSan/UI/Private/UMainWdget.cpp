#include "UMainWdget.h"

#include "GameLogging.h"
#include "Components/EditableTextBox.h"
// #include "ALattePlayerCharacter.h"
// #include "UAudioCaptureComponent.h"

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
	// if (CommitMethod != ETextCommit::OnEnter)
	// {
	// 	return;
	// }
	//
	// FString CleanText = Text.ToString();
	// CleanText.TrimStartAndEndInline();
	//
	// if (CleanText.IsEmpty())
	// {
	// 	return;
	// }
	//
	// PRINTLOG(TEXT("[UMainWdget] Enter committed text: %s"), *CleanText);
	//
	// if (ALattePlayerCharacter* PlayerActor = Cast<ALattePlayerCharacter>(GetOwningPlayerPawn()))
	// {
	// 	if (UVoiceCaptureComponent* VoiceComponent = PlayerActor->FindComponentByClass<UVoiceCaptureComponent>())
	// 	{
	// 		VoiceComponent->RequestSynthesis(CleanText);
	// 	}
	// }
	//
	// if (VoiceInputBox)
	// {
	// 	VoiceInputBox->SetText(FText::GetEmpty());
	// }
}
