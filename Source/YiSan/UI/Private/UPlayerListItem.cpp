#include "UPlayerListItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UPlayerListItem::SetPlayerStatus(const int32 InPlayerIndex, const int32 LocalPlayerIndex, const FString& InName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InName));
		PlayerNameText->SetColorAndOpacity(InPlayerIndex == LocalPlayerIndex ? FLinearColor::Green : FLinearColor::White );
	}

	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Visible);
		PlayerIcon->SetBrushFromTexture(PlayerIconTextures.IsValidIndex(InPlayerIndex) ? PlayerIconTextures[InPlayerIndex] : PlayerIconTextures[0]);
	}
}