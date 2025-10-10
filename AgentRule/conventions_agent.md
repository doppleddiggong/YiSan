# Unreal Engine C++ 코딩 컨벤션 가이드 (Project YiSan)

이 문서는 `YiSan` 프로젝트의 C++ 코드 작성 시 일관성을 유지하기 위한 코딩 컨벤션을 정의합니다. 모든 코드는 언리얼 엔진 공식 코딩 표준을 기반으로 하며, 프로젝트의 특성을 반영한 추가 규칙을 포함합니다.

---

## 1. 네이밍 규칙 (Naming Conventions)

### 1.1. 클래스 및 파일명 (Classes and Filenames)

- **접두사**: 모든 클래스는 역할에 맞는 접두사를 사용해야 합니다.
  - `A`: `AActor`를 상속하는 클래스 (예: `APlayerActor`)
  - `U`: `UObject`를 상속하는 클래스 (예: `UStatSystem`)
  - `I`: 인터페이스 클래스 (예: `IControllable`)
  - `F`: 구조체, `UObject` 외의 일반 클래스 또는 델리게이트 (예: `FHitResult`, `FMathHelper`, `FResponseHealthDelegate`)
  - `E`: 열거형 (예: `ECharacterType`)
  - `T`: 템플릿 클래스 (예: `TArray`, `TSubclassOf`)

- **파일명 일치**: 클래스 이름은 해당 클래스를 정의하는 파일의 이름과 정확히 일치해야 합니다.
  - 예: `class UStatSystem`은 `UStatSystem.h` 및 `UStatSystem.cpp` 파일에 정의되어야 합니다.

### 1.2. 변수 (Variables)

- **형식**: `camelCase`를 사용합니다. (예: `maxHealth`, `currentAmmo`)
- **bool 변수**: 예외적으로 `b` 접두사와 `PascalCase`를 따릅니다. (예: `bIsDead`, `bIsPlayer`)
- **포인터**: `TObjectPtr` 사용을 권장하여 UObject 포인터를 안전하게 관리합니다.
- **접근성**: 멤버 변수는 `protected` 또는 `private`으로 선언하고, 필요한 경우 Getter/Setter 함수를 통해 접근합니다.

```cpp
protected:
    // 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
    float curHP = 1000;

    // bool 변수는 b + PascalCase 규칙을 유지합니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
    bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
    TObjectPtr<class UCameraShakeSystem> cameraShakeSystem;
```

### 1.3. 함수 (Functions)

- **형식**: `PascalCase`를 사용합니다.
- **Getter**: `Get` 접두사를 사용하며, 간단한 반환문만 있는 경우 `FORCEINLINE`을 사용하여 인라인 처리를 유도할 수 있습니다.
- **명령형 함수**: `Cmd_` 접두사를 사용하여 입력 처리를 명확히 구분할 수 있습니다. (예: `Cmd_Move`)

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
FORCEINLINE float GetCurHP()
{
    return CurHP;
}

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
void Cmd_Jump() override;
```

---

## 2. UPROPERTY / UFUNCTION 매크로

- **Category**: 기능별로 명확하게 그룹화하고, `|` 문자를 사용하여 하위 카테고리를 지정합니다. (예: `Category="Stats|HP"`)
- **Blueprint 접근**: `BlueprintReadOnly`와 `BlueprintReadWrite`를 명확히 구분하여 사용합니다. 외부에서 수정될 필요가 없는 변수는 `BlueprintReadOnly`를 사용합니다.
- **에디터 전용 함수**: `CallInEditor`와 `meta=(DevelopmentOnly)`를 함께 사용하여 에디터에서만 호출 가능하도록 지정합니다.
- **Blueprint 이벤트**: `BlueprintNativeEvent`는 C++ 기본 구현과 블루프린트 재정의가 모두 필요할 때, `BlueprintImplementableEvent`는 블루프린트에서만 구현할 때 사용합니다.

---

## 3. 파일 구조 및 주석

### 3.1. 헤더 파일

- **중복 포함 방지**: 모든 헤더 파일 상단에 `#pragma once`를 사용합니다.
- **포함 순서 (IWYU - Include What You Use)**:
  1. 해당 헤더에 대응하는 `.cpp` 파일의 헤더 (예: `MyClass.cpp` -> `MyClass.h`)
  2. 모듈의 다른 헤더
  3. 다른 모듈의 헤더
  4. 시스템 헤더 (`CoreMinimal.h` 등)
- **.generated.h**: 항상 파일의 마지막 `#include` 문이어야 합니다.

### 3.2. 주석 (Comments)

- **파일 헤더**: 모든 소스 파일 상단에 저작권 및 라이선스 정보를 포함한 주석을 추가합니다. (예: `// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.`)
- **기능 그룹**: `public:`, `protected:`와 같은 접근 지정자 뒤에 해당 섹션의 역할을 설명하는 주석을 추가하는 것을 권장합니다. (예: `public: // Control Interface`)
- **코드 설명**: 복잡한 로직이나 특정 결정의 이유를 설명하는 주석을 작성합니다. "무엇을" 하는지보다 "왜" 그렇게 했는지를 설명하는 데 집중합니다.

### 3.3. 전방 선언 (Forward Declaration)

헤더 파일(.h)에서는 컴파일 시간 단축을 위해 다른 헤더를 직접 `#include`하는 대신 전방 선언을 사용하는 것을 원칙으로 합니다.

**권장 스타일:**
`TObjectPtr` 또는 포인터 변수 선언 시, 템플릿 인자 안에서 `class` 키워드를 사용하여 해당 위치에서 바로 전방 선언을 수행합니다. 이 방식은 선언을 사용하는 곳과 가깝게 위치시켜 가독성을 높입니다.

```cpp
// MyActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS()
class YISAN_API AMyActor : public AActor
{
    GENERATED_BODY()

protected:
    // 권장: TObjectPtr<class UMyComponent>와 같이 사용하여 헤더 상단에 별도 선언이 필요 없음
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USpringArmComponent> SpringArmComponent;
};
```

**대안 스타일:**
파일 상단에서 여러 클래스를 한 번에 전방 선언하는 전통적인 방식도 허용됩니다.

```cpp
// MyLegacyActor.h

#pragma once

#include "CoreMinimal.h"
// ... other includes

class UStaticMeshComponent;
class USpringArmComponent;

UCLASS()
// ...
```

---

## 4. 기타 규칙

### 4.1. 로깅 (Logging)

- **일반 로깅**: `PRINTLOG` 매크로를 사용하여 일반적인 디버그 및 정보성 메시지를 기록합니다.
- **네트워크 로깅**: `NETWORK_LOG` 매크로를 사용하여 네트워크 통신 관련 로그를 기록하며, `ENetworkLogType` 열거형과 `[GET]`, `[POST]`, `[WS]`와 같은 접두사를 활용하여 로그의 종류를 명확히 구분합니다.

### 4.2. 기타

- **에디터 코드**: 에디터에서만 사용되는 코드는 `#if WITH_EDITOR` ... `#endif` 블록으로 감싸서 패키지 빌드 시 제외되도록 합니다.
- **상수**: `const`와 `constexpr`를 적절히 사용하여 불변 데이터를 명시합니다.
- **강제 형변환**: C-style 캐스팅 대신 `static_cast`, `const_cast`, `reinterpret_cast`를 사용합니다.

---

## 5. 네트워크 모듈 컨벤션 (Network Module Conventions)

- **서브시스템 활용**: `UGameInstanceSubsystem`을 상속받아 네트워크 시스템을 구현하며, `DEFINE_SUBSYSTEM_GETTER_INLINE` 매크로를 사용하여 전역 접근을 용이하게 합니다.
- **서버 설정 관리**: `UCustomNetworkSettings` (UDeveloperSettings 파생)를 사용하여 개발, 스테이지, 라이브 환경별 서버 주소 및 포트 설정을 관리합니다. 명령줄 인수를 통해 서버 모드를 동적으로 변경할 수 있습니다.
- **API 엔드포인트 정의**: `RequestAPI` 네임스페이스 내에 정적 `FString` 변수로 각 API 엔드포인트를 정의하여 일관성을 유지합니다.
- **URL 구성**: `NetworkConfig` 네임스페이스의 정적 함수를 통해 `UCustomNetworkSettings`에서 현재 서버 모드에 맞는 `BaseUrl`과 `Port`를 가져와 전체 URL을 구성합니다.
- **HTTP 요청**: `FHttpModule`을 사용하여 HTTP GET/POST 요청을 처리하며, `FJsonObjectConverter`를 통해 USTRUCT와 JSON 간의 데이터 변환을 수행합니다.
- **WebSocket 통신**: `FWebSocketsModule`을 사용하여 WebSocket 연결, 메시지 송수신, 연결 상태 관리를 처리합니다.
- **응답 데이터 구조**: 서버 응답 데이터를 위한 `USTRUCT` (예: `FResponseHealth`, `FResponseHelpChat`)를 정의하고, `SetFromHttpResponse` 및 `PrintData`와 같은 헬퍼 메서드를 포함하여 응답 처리 및 로깅을 표준화합니다.
- **네트워크 로깅**: `NETWORK_LOG` 매크로를 사용하여 네트워크 통신 관련 로그를 기록하며, `ENetworkLogType` 열거형과 `[GET]`, `[POST]`, `[WS]`와 같은 접두사를 활용하여 로그의 종류를 명확히 구분합니다.
- **델리게이트 사용**: 비동기 네트워크 응답 처리를 위해 `DECLARE_DELEGATE_TwoParams`를 사용하여 C++ 콜백을 정의하고, `DECLARE_DYNAMIC_MULTICAST_DELEGATE`를 사용하여 블루프린트에서 바인딩 가능한 이벤트를 제공합니다.

---

## 6. Doxygen 주석 스타일 가이드 (Doxygen Comment Style Guide)

일관성 있고 가독성 높은 문서 자동화를 위해 모든 C++ 코드의 주석은 Doxygen 스타일을 따릅니다.

### 6.1. 주석 형식

- **C++ 스타일**: `///` (세 개의 슬래시) 또는 `/** ... */` 형식을 사용합니다. `///`를 권장합니다.
- **배치**: 클래스, 함수, 변수, 열거형 등 문서화가 필요한 선언 바로 위에 주석을 작성합니다.

### 6.2. 필수 태그 및 예시

- **간단한 설명**: `@brief` 태그를 사용하여 한 줄로 요약된 설명을 작성합니다.
- **상세 설명**: `@brief` 다음에 한 줄을 띄고 상세한 설명을 작성합니다.
- **파라미터**: `@param [in/out] 파라미터명 설명` 형식으로 함수의 모든 파라미터를 설명합니다. `[in]`, `[out]`을 명시하여 데이터 흐름을 나타냅니다.
- **반환값**: `@return 설명` 형식으로 함수의 반환값을 설명합니다.
- **참고**: `@note` 태그를 사용하여 특별히 주의해야 할 사항이나 참고 정보를 기재합니다.

### 6.3. 코드 예시

다음은 Doxygen 주석 스타일을 적용한 함수 예시입니다.

```cpp
/// @brief 캐릭터의 현재 체력을 변경하고, 변경 결과를 브로드캐스트합니다.
/// @param InDamage 적용할 데미지 또는 치유량. 양수이면 데미지, 음수이면 치유입니다.
/// @param DamageType 데미지의 유형 (예: 물리, 화염).
/// @return 최종적으로 적용된 후의 체력을 반환합니다.
/// @note 이 함수는 서버에서만 호출되어야 합니다. 클라이언트에서의 호출은 무시됩니다.
float AMyCharacter::TakeDamage(float InDamage, EDamageType DamageType)
{
    // ... 함수 본문 ...
    
    float FinalHealth = CurrentHealth - InDamage;
    
    // ... 체력 변경 및 사망 처리 로직 ...

    OnHealthChanged.Broadcast(FinalHealth);

    return FinalHealth;
}
```

---

## 7. UObject 포인터 사용 규칙

- **TObjectPtr 사용**: Unreal Engine 5부터 도입된 `TObjectPtr`를 사용하여 UObject에 대한 소유권 및 참조를 명확하게 표현하는 것을 선호합니다. 이는 메모리 관리 및 안정성 향상에 도움이 됩니다.

### 예시

**선호하는 방식:**
```cpp
// 헤더 파일 (.h)
UPROPERTY()
TObjectPtr<UStaticMeshComponent> Door_Right;

// 전방 선언 시
TObjectPtr<class UBoxComponent> BoxCollision;
```

**비선호하는 방식:**
```cpp
// 헤더 파일 (.h)
UPROPERTY()
UStaticMeshComponent* Door_Right;

// 전방 선언 시
class UBoxComponent;
```