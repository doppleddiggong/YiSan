# Unreal Engine C++ 코딩 컨벤션 가이드 (Project YiSan)

이 문서는 `YiSan` 프로젝트의 C++ 코드 작성 시 일관성을 유지하기 위한 코딩 컨벤션을 정의합니다. 모든 코드는 언리얼 엔진 공식 코딩 표준을 기반으로 하며, 프로젝트의 특성을 반영한 추가 규칙을 포함합니다.

---

## 1. 네이밍 규칙 (Naming Conventions)

### 1.1. 클래스 (Classes)

- **접두사**: 모든 클래스는 역할에 맞는 접두사를 사용해야 합니다.
  - `A`: `AActor`를 상속하는 클래스 (예: `APlayerActor`)
  - `U`: `UObject`를 상속하는 클래스 (예: `UStatSystem`)
  - `I`: 인터페이스 클래스 (예: `IControllable`)
  - `F`: 구조체 또는 `UObject` 외의 일반 클래스 (예: `FHitResult`, `FMathHelper`)
  - `E`: 열거형 (예: `ECharacterType`)
  - `T`: 템플릿 클래스 (예: `TArray`, `TSubclassOf`)

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

- **파일 헤더**: 모든 소스 파일 상단에 저작권 및 라이선스 정보를 포함한 주석을 추가합니다.
- **기능 그룹**: `public:`, `protected:`와 같은 접근 지정자 뒤에 해당 섹션의 역할을 설명하는 주석을 추가하는 것을 권장합니다. (예: `public: // Control Interface`)
- **코드 설명**: 복잡한 로직이나 특정 결정의 이유를 설명하는 주석을 작성합니다. "무엇을" 하는지보다 "왜" 그렇게 했는지를 설명하는 데 집중합니다.

---

## 4. 기타 규칙

- **에디터 코드**: 에디터에서만 사용되는 코드는 `#if WITH_EDITOR` ... `#endif` 블록으로 감싸서 패키지 빌드 시 제외되도록 합니다.
- **상수**: `const`와 `constexpr`를 적절히 사용하여 불변 데이터를 명시합니다.
- **강제 형변환**: C-style 캐스팅 대신 `static_cast`, `const_cast`, `reinterpret_cast`를 사용합니다.
