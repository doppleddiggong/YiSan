# Debug Agent Guide

## 1. 목적 (Purpose)
- 코딩 버그 발생 시 체계적인 디버깅 프로세스를 지원하여 문제 해결 효율성 증대.

## 2. 디버깅 전략 (Debugging Strategy)

### 2.1. 문제 이해 (Understanding the Problem)
- 사용자로부터 문제 재현 조건, 예상 동작, 실제 동작을 명확히 확인한다.
- 관련 코드 영역 및 시스템(예: 입력, 애니메이션, 상태 머신)을 파악한다.

### 2.2. 디버그 포인트 식별 (Identifying Debug Points)
- 문제 발생 가능성이 있는 함수 진입/종료 지점.
- 조건문 (if/else, switch)의 분기점.
- 변수 값 변경 지점.
- 상태(State) 변경 지점.
- 이벤트 발생/처리 지점.

### 2.3. 디버그 코드 삽입 (Inserting Debug Code)
- `PRINTLOG(TEXT("... "));` 매크로를 사용하여 콘솔에 로그를 출력한다.
- 로그 메시지에는 다음 정보가 포함되어야 한다:
    - **출처**: `[파일명::함수명]` 또는 `[클래스명::함수명]`
    - **상태**: 현재 변수 값, 조건 결과 (true/false), 진입/종료 여부 등.
    - **고유 식별자**: 플레이어/적 등 엔티티를 구분할 수 있는 접두사 (예: `PLAYER:`, `ENEMY:`).
- **예시**:
  ```cpp
  // 함수 진입 시
  PRINTLOG(TEXT("PLAYER: [APlayerActor::Cmd_Move_Implementation] - Entry. Axis.X: %f, Axis.Y: %f"), Axis.X, Axis.Y);

  // 조건문 분기 시
  if (bIsAttacking)
  {
      PRINTLOG(TEXT("PLAYER: [URushAttackSystem::OnAttack] - bIsAttacking is true. Returning."));
      return;
  }

  // 변수 값 변경 시
  bIsAttacking = true;
  PRINTLOG(TEXT("PLAYER: [URushAttackSystem::PlayMontage] - bIsAttacking set to true."));
  ```

### 2.4. 로그 분석 (Analyzing Logs)
- 사용자로부터 제공된 로그를 시간 순서대로 분석하여 예상 동작과 실제 동작의 차이를 파악한다.
- `PRINTLOG` 메시지를 통해 코드 실행 흐름을 추적하고, 어떤 조건이 예상과 다르게 동작하는지 식별한다.
- 특히, `false`로 반환되어야 할 조건이 `true`로 반환되거나, 특정 함수가 호출되지 않는 경우 등을 집중적으로 확인한다.

### 2.5. 수정 방안 제안 (Proposing Solutions)
- 로그 분석 결과를 바탕으로 문제의 근본 원인을 파악한다.
- 문제 해결을 위한 구체적인 코드 수정 방안을 제시한다.
- 수정 방안은 기존 코드 컨벤션 및 프로젝트 구조를 따른다.

### 2.6. 디버그 코드 정리 (Cleaning Up Debug Code)
- 문제 해결 및 수정 적용 후, 삽입했던 모든 `PRINTLOG` 및 임시 디버그 코드를 제거한다.
- `Core/Macro.h`와 같은 디버그 관련 헤더 파일 include도 제거한다.

## 3. 반복적인 문제 처리 (Handling Recurring Issues)
- 동일하거나 유사한 버그가 반복적으로 발생하는 경우, 해당 문제에 대한 디버그 가이드를 업데이트하거나 새로운 가이드를 추가하여 에이전트의 학습을 강화한다.

---

## 4. 로그 파일 자동 분석 (규칙 추가)
- **상황**: 디버깅이 필요하거나 사용자가 로그 확인을 요청할 경우.
- **동작**:
  1. 에이전트는 먼저 `Saved/Logs/` 디렉터리에서 가장 최근에 수정된 `.log` 파일을 찾는다.
  2. 해당 로그 파일의 내용을 읽고 분석하여 문제의 원인을 파악하는 데 사용한다.
  3. 분석 결과를 바탕으로 사용자에게 답변이나 해결책을 제시한다.
