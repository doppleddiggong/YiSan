#pragma once

#include "CoreMinimal.h"

/**
 * @file FNetworkTestFeature.h
 * @brief 네트워크 API 테스트 기능을 제공합니다.
 */
class FNetworkTestFeature
{
public:
	/** @brief 네트워크 테스트 메뉴를 생성합니다. */
	TSharedRef<class SWidget> GenerateNetworkTestMenu();

private:
	/** @brief API 요청을 실행합니다. */
	void ExecuteApiRequest(int32 ApiIndex);
};
