---
description: 새로운 작업 세션을 시작하고 Start Log를 자동으로 생성합니다
---

# Start Log 생성

**참고**: `AgentRule/AgentRule/Common/Workflows/agents_log.md`

## 실행 내용

1. 현재 Git 사용자 정보 확인
2. 현재 시간 (UTC+09:00) 계산
3. AgentLog 디렉토리 생성 (없으면)
4. Start Log 작성 및 저장
5. 사용자에게 ToDo 리스트 제시

## 작업 지시

다음 절차를 따라 Start Log를 생성하세요:

### 1. Git 사용자 정보 확인
```bash
git config user.name
```

이 값을 `[작업자GitID]`로 사용합니다. 값이 없으면 "작업자"를 사용합니다.

### 2. 현재 시간 계산 (UTC+09:00, 한국 시간)
- 파일명 형식: `[작업자GitID]_YYMMDD.md` (예: `dopple_251107.md`)
- 시간 형식: `YYYYMMDD hh:mm` (예: `20251107 19:30`)

### 3. Git Root 확인
```bash
git rev-parse --show-toplevel
```

### 4. AgentLog 디렉토리 생성
경로: `[GitRoot]/AgentLog/[작업자GitID]/`

디렉토리가 없으면 생성합니다.

### 5. Start Log 파일 작성

**파일 경로**: `[GitRoot]/AgentLog/[작업자GitID]/[작업자GitID]_YYMMDD.md`
**예시**: `AgentLog/dopple/dopple_251109.md`

**파일이 이미 존재하는 경우**: 맨 아래에 추가
**파일이 없는 경우**: 새로 생성

**작성 내용**:
```markdown
# [작업자GitID] YYMMDD

# Start Log
- YYYYMMDD hh:mm
- 작업자: [작업자GitID]

### To Do

1. [사용자가 현재 요청한 작업을 구체적으로 작성]

```

### 6. ToDo 작성 규칙
- **작은 단위**로 구체적으로 작성
- 현재 시점에서 바로 해야 할 일만 포함
- 큰 작업은 여러 단계로 나누기

### 7. 사용자에게 알림
Start Log가 작성되었음을 알리고, 작업을 시작합니다.

---

**중요**: Start Log는 세션당 한 번만 생성됩니다. 같은 날짜에 여러 세션이 있더라도 각 세션마다 새로운 Start Log를 추가합니다.
