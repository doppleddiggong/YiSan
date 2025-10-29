
// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "IControllable.h"
#include "NetworkData.h"
#include "GameFramework/Character.h"
/// @file APlayerActor.h
/// @brief 프로젝트 전반에서 사용되는 플레이어 조작 캐릭터 액터를 선언합니다.

#include "APlayerActor.generated.h"

/// @brief 이동, UI, 음성 하위 시스템을 통합하는 플레이어 조작 캐릭터입니다.
///
/// @details 이 액터는 입력 처리, 음성 명령, GPT 컨텍스트 조회, 주변 건물 탐지 로직을 묶어
/// 인게임 플레이 경험을 주도합니다. IControllable 인터페이스를 구현하여 게임플레이 코드와
/// UI 레이어가 동일한 방식으로 이동 및 상호작용 명령을 전달할 수 있습니다.
UCLASS()
class YISAN_API APlayerActor : public ACharacter, public IControllable
{
    GENERATED_BODY()

public:
    /// @brief 핵심 컴포넌트와 게임플레이 태그를 설정하는 기본 생성자입니다.
    APlayerActor();

protected:
    /// @brief 게임 플레이가 시작될 때 호출되어 의존 시스템을 초기화합니다.
    virtual void BeginPlay() override;
    
public:
    /// @brief 플레이어의 최신 GPT 컨텍스트 스냅샷을 수집합니다.
    /// @return 플레이어 위치, 시선 대상, 주변 건물을 담은 FGPTContext입니다.
    FGPTContext GetGPTContext() const;

    FString GetPlayerDisplayName() const;
    int GetLocalPlayerIndex() const;

private:
    /// @brief 브로드캐스트 매니저에서 전달되는 음성 명령 실행 이벤트에 대응합니다.
    UFUNCTION()
    void OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Owner")
    TObjectPtr<class USkeletalMeshComponent> MeshComp; ///< 애니메이션 재생을 담당하는 메시 컴포넌트입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Owner")
    TObjectPtr<class UCharacterMovementComponent> MoveComp; ///< 이동 보조 로직에서 사용하는 캐시된 무브먼트 컴포넌트입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Owner")
    TObjectPtr<class UAnimInstance> AnimInstance; ///< 게임플레이 기반 몽타주 조회에 사용하는 애님 인스턴스입니다.

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<class USpringArmComponent> SpringArmComp; ///< 3인칭 팔로우 카메라를 위한 스프링 암 컴포넌트입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<class UCameraComponent> FollowCamera; ///< 플레이어 시점을 담당하는 팔로우 카메라입니다.

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UMainWidget> MainWidgetClass; ///< 메인 HUD 위젯을 제공하는 블루프린트 클래스입니다.

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UMainWidget> MainWidgetInst; ///< 뷰포트에 배치되는 메인 위젯 인스턴스입니다.

    
    
    
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UChatUIWidget> ChatUIWidgetClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<class UChatBoxWidget> ChatBoxWidget;
    

    

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice", meta=(AllowPrivateAccess="true"))
    TObjectPtr<class UVoiceConversationSystem> VoiceConversationSystem; ///< 플레이어가 소유한 음성 명령 파이프라인입니다.

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GPT", meta=(AllowPrivateAccess="true"))
    TObjectPtr<class UGPTContextSystem> GPTContextSystem; ///< 시선 및 주변 건물을 추적하는 GPT 컨텍스트 제공자입니다.

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chat", meta=(AllowPrivateAccess="true"))
    TObjectPtr<class UChatPlayerSystem> ChatPlayerSystem; ///< 채팅 시스템 컴포넌트입니다.

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager; ///< UI 알림에 사용하는 캐시된 브로드캐스트 매니저입니다.

public: // 음성 관련
    /// @brief TTS 오디오를 재생합니다. VoiceConversationSystem으로 전달합니다.
    /// @param AudioData [in] TTS로 생성된 오디오 데이터 (WAV)
    /// @return 재생 성공 여부
    UFUNCTION(BlueprintCallable, Category = "Voice")
    void PlayTTSAudio(const TArray<uint8>& AudioData);

public: // 제어 인터페이스
    /// @brief 입력 축 데이터를 처리해 월드 공간 이동을 수행합니다.
    /// @param Axis [in] X, Y 입력 값을 담은 2D 벡터입니다.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_Move(const FVector2D& Axis) override;

    /// @brief 시야 입력을 플레이어 컨트롤러의 요와 피치에 반영합니다.
    /// @param Axis [in] 마우스 또는 패드 입력 축 값입니다.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_Look(const FVector2D& Axis) override;

    /// @brief ACharacter 기본 점프 동작을 호출합니다.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_Jump() override;

    /// @brief GPT 상호작용을 위한 음성 캡처를 시작합니다.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_RecordStart() override;

    /// @brief 음성 캡처를 종료하고 수집된 오디오 데이터를 전송합니다.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_RecordEnd() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_ShowDetail() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_ChatEnter() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_ChatScrollUp() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_ChatScrollDown() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_ShowMouse() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
    void Cmd_HideMouse() override;
};

