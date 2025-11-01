#include "UPlayerListItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UPlayerListItem::SetPlayerName(const FString& InName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InName));
	}
}

void UPlayerListItem::SetPlayerStatus(bool bIsHost, bool bIsLocalPlayer, const int32 InPlayerIndex)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetColorAndOpacity(bIsLocalPlayer ? FLinearColor::Green : FLinearColor::White );
	}

	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Visible);
		PlayerIcon->SetBrushFromTexture(PlayerIconTextures.IsValidIndex(InPlayerIndex) ? PlayerIconTextures[InPlayerIndex] : PlayerIconTextures[0]);
	}
}
