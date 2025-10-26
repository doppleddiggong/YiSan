// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDialogWidgetService.generated.h"

class UDialogWidget;

/**
 * @brief 게임 전역에서 대화 토스트를 관리하는 서비스 서브시스템입니다.
 */
UCLASS()
class YISAN_API UDialogWidgetService : public UGameInstanceSubsystem
{
        GENERATED_BODY()

public:
        UDialogWidgetService();

        virtual void Initialize(FSubsystemCollectionBase& Collection) override;
        virtual void Deinitialize() override;

        DEFINE_SUBSYSTEM_GETTER_INLINE(UDialogWidgetService);

        /**
         * @brief 외부 시스템에서 직접 토스트 출력을 요청할 수 있도록 합니다.
         */
        UFUNCTION(BlueprintCallable, Category = "Dialog")
        void RequestDialog(const FString& Message);

protected:
        /**
         * @brief 토스트를 표시할 위젯 클래스를 반환합니다.
         */
        TSubclassOf<UDialogWidget> GetDialogWidgetClass() const;

        /**
         * @brief 현재 월드에 위젯이 없으면 생성하고 뷰포트에 부착합니다.
         */
        void EnsureWidgetForWorld(UWorld* World);

        /**
         * @brief 월드 종료 시 위젯을 정리합니다.
         */
        void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

        /**
         * @brief 월드 시작 시 위젯을 확인합니다.
         */
        void HandleWorldBeginPlay(UWorld* World);

        /**
         * @brief 브로드캐스트 매니저에서 전달된 토스트 메시지를 처리합니다.
         */
        UFUNCTION()
        void HandleToastMessage(const FString& InMessage);

        void BindBroadcastDelegates();
        void UnbindBroadcastDelegates();
        void TeardownWidget();

protected:
        /** @brief 생성할 다이얼로그 위젯 클래스입니다. 기본값은 UDialogWidget 입니다. */
        UPROPERTY(EditDefaultsOnly, Category = "Dialog")
        TSubclassOf<UDialogWidget> DialogWidgetClass;

        /** @brief 현재 뷰포트에 부착된 다이얼로그 위젯 인스턴스입니다. */
        UPROPERTY()
        TObjectPtr<UDialogWidget> DialogWidget;

        /** @brief OnWorldBeginPlay 델리게이트 핸들입니다. */
        FDelegateHandle WorldBeginPlayHandle;

        /** @brief OnWorldCleanup 델리게이트 핸들입니다. */
        FDelegateHandle WorldCleanupHandle;
};
