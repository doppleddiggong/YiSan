// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Modules/ModuleManager.h"

#include "Level/FLevelFeature.h"
#include "Screenshot/FScreenshotFeature.h"
#include "Command/FCommandFeature.h"
#include "Folder/FolderFeature.h"
#include "NetworkTest/FNetworkTestFeature.h"

/**
 * @file FToolbar.h
 * @brief CoffeeToolbar 플러그인의 주요 모듈 구현을 선언합니다.
 */

/**
 * @brief Coffee Toolbar 기능을 에디터에 연결하는 모듈 구현입니다.
 */
class FToolbar : public IModuleInterface
{
public:
	/** @brief 툴바 기능을 초기화하고 Slate 메뉴 확장을 등록합니다. */
	virtual void StartupModule() override;

	/** @brief 등록된 메뉴를 해제하고 툴바 기능을 정리합니다. */
	virtual void ShutdownModule() override;

private:
	/** @brief 에디터 확장 프레임워크에 툴바 메뉴와 버튼을 등록합니다. */
	void RegisterMenus();

    /** @brief 툴바에서 명령 단축 기능을 제공하는 모듈입니다. */
    TUniquePtr<FCommandFeature> CommandFeature;

    /** @brief 레벨 관련 유틸리티 버튼을 제공하는 기능입니다. */
    TUniquePtr<FLevelFeature> LevelFeature;

    /** @brief 스크린샷 캡처 헬퍼를 제공하는 기능입니다. */
    TUniquePtr<FScreenshotFeature> ScreenshotFeature;

    /** @brief 폴더 열기 기능을 제공하는 모듈입니다. */
	TUniquePtr<FFolderFeature> FolderFeature;

	/** @brief 네트워크 API 테스트 기능을 제공하는 모듈입니다. */
	TUniquePtr<FNetworkTestFeature> NetworkTestFeature;
};