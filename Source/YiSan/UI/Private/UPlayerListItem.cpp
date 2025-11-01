#include "UPlayerListItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UPlayerListItem::SetPlayerName(const FString& InName)
{
	if (PlayerNameText)
		PlayerNameText->SetText(FText::FromString(InName));
}

void UPlayerListItem::SetPlayerStatus(const int32 InPlayerIndex)
{
	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Visible);
		if (PlayerIconTextures.IsValidIndex(InPlayerIndex))
		{
			PlayerIcon->SetBrushFromTexture(PlayerIconTextures[InPlayerIndex]);
		}
		else
		{
			PlayerIcon->SetBrushFromTexture(PlayerIconTextures[0]);
		}
	}
}
