// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UVoiceGuidePanel.h
 * @brief UVoiceGuidePanel 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UVoiceGuidePanel.generated.h"

/**
 * @brief 사용 가능한 음성 명령 예시를 보여주는 안내 UI 패널 위젯입니다.
 * @details 플레이어가 NPC와 어떤 대화를 할 수 있는지 돕는 가이드 역할을 합니다.
 */
UCLASS()
class YISAN_API UVoiceGuidePanel : public UUserWidget
{
	GENERATED_BODY()
};
