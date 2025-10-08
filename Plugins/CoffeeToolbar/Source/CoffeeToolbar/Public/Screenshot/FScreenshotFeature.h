#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h" // Not strictly needed here, but good practice for UI features

/**
 * @file FScreenshotFeature.h
 * @brief 툴바의 스크린샷 워크플로우 버튼을 위한 헬퍼를 선언합니다.
 */

/**
 * @brief 스크린샷 촬영과 폴더 탐색 동작을 제공합니다.
 */
class FScreenshotFeature
{
public:
    /** @brief 기본 스크린샷 설정 상태를 구성합니다. */
    FScreenshotFeature();

    /** @brief 스크린샷 옵션을 제공하는 메뉴 위젯을 생성합니다. */
    TSharedRef<SWidget> GenerateScreenshotMenu();

    /** @brief 촬영된 스크린샷이 저장된 디렉터리를 엽니다. */
    void OnOpenScreenShotDir();

private:
    /** @brief 지정된 배율로 활성 뷰포트에 고해상도 캡처를 요청합니다. */
    void CaptureActiveViewport(int32 ResolutionMultiplier);
};
