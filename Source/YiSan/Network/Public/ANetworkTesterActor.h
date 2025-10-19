// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/// @file ANetworkTesterActor.h
/// @brief 네트워크 진단용 테스트 액터를 선언합니다.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkData.h"
#include "ANetworkTesterActor.generated.h"

UCLASS()
class YISAN_API ANetworkTesterActor : public AActor
{
    GENERATED_BODY()

public:
    /// @brief 기본 컴포넌트를 구성하는 생성자입니다.
    ANetworkTesterActor();

protected:
    /// @brief 에디터에서 수동으로 /health 요청을 트리거합니다.
    UFUNCTION(CallInEditor, Category = "TEST|RequestHealth")
    void RequestHealth();

    /// @brief 헬스 체크 응답을 수신했을 때 결과를 로그로 출력합니다.
    /// @param ResponseData [in] 서버에서 전달한 상태 정보입니다.
    /// @param bWasSuccessful [in] HTTP 요청 성공 여부입니다.
    void OnResponseHealth(FResponseHealth& ResponseData, bool bWasSuccessful);
};
