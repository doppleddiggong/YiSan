#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Framework/Commands/UIAction.h"
#include "AssetRegistry/AssetData.h" // For FAssetData

/**
 * @file FLevelFeature.h
 * @brief 툴바에 레벨 선택 컨트롤을 제공하는 기능을 선언합니다.
 */

/**
 * @brief 빠른 레벨 전환을 위한 월드 검색과 에디터 명령을 처리합니다.
 */
class FLevelFeature
{
public:
    /** @brief 기본 상태를 설정하고 저장된 선택을 불러옵니다. */
    FLevelFeature();

    /** @brief 레벨 선택 항목을 포함한 Slate 위젯을 구성합니다. */
    TSharedRef<SWidget> GenerateLevelMenu();

    /** @brief 선택된 맵 패키지 이름을 저장합니다. */
    void OnSelectedMap(FString InLongPackageName);

    /** @brief 선택된 맵 에셋을 에디터에서 엽니다. */
    void OnSelectedMap_Open();

    /** @brief 선택된 맵으로 현재 뷰포트에서 PIE를 시작합니다. */
    void OnSelectedMap_PlaySelectedViewport();

    /** @brief 선택된 맵으로 새로운 뷰포트에서 PIE를 실행합니다. */
    void OnSelectedMap_PlayInEditor();

    /** @brief 시작 이후에도 유효한 기본 맵 선택이 존재하도록 보장합니다. */
    void EnsureDefaultSelection();

private:
    /** @brief 설정된 검색 경로에서 발견 가능한 맵 에셋을 모두 수집합니다. */
    bool CollectWorlds(TArray<FAssetData>& OutWorlds) const;

    /** @brief 선택된 맵 에셋이 여전히 존재하는지 검증합니다. */
    bool EnsureValidSelectedMap() const;

    /** @brief 현재 선택된 맵 패키지를 에디터에 로드합니다. */
    bool LoadSelectedMap() const;

public:
    /** @brief 현재 선택된 맵의 패키지 경로입니다. */
    FString SelectedMapPackage;
};