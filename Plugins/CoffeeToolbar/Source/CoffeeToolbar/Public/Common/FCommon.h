#pragma once

#include "CoreMinimal.h"
#include "Styling/AppStyle.h"

/**
 * @file FCommon.h
 * @brief Coffee Toolbar 플러그인을 위한 공용 유틸리티 헬퍼를 선언합니다.
 */

/**
 * @brief Slate 및 월드 관련 유틸리티를 제공하는 네임스페이스형 헬퍼 클래스입니다.
 */
class FCommon
{
public:
    /** @brief 전달된 이름으로 애플리케이션 스타일에서 Slate 아이콘을 조회합니다. */
    static const FSlateIcon GetIcon(FName IconName);

    /** @brief 툴바 액션이 대상으로 삼는 활성 에디터 월드를 반환합니다. */
    static UWorld* GetActiveTargetWorld();
};