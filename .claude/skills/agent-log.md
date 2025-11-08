# Agent Log Skill

**목적**: Claude CLI 세션의 작업 내용을 자동으로 기록하고 추적

**참고**: `AgentRule/AgentRule/Common/Workflows/agents_log.md`의 Log 시스템 규칙을 따름

---

## 자동 실행 조건

이 스킬은 다음 상황에서 자동으로 실행됩니다:
- `/agent-log start` - 세션 시작 시 Start Log 생성
- `/agent-log compact` - 작업 중간 요약 생성
- `/agent-log end` - 세션 종료 시 End Log 생성

---

## Start Log 생성 절차

**실행 시점**: 세션 시작 시 또는 새로운 작업 시작 시

### 1단계: Git 사용자 정보 확인
```bash
git config user.name
git config user.email
```

### 2단계: 날짜 및 시간 계산
- UTC+09:00 (한국 시간) 기준
- 파일명: `YYMMDD.md` (예: `251107.md`)
- 시간: `YYYYMMDD hh:mm` (예: `20251107 19:30`)

### 3단계: AgentLog 디렉토리 확인 및 생성
```bash
# Git Root 찾기
git rev-parse --show-toplevel

# AgentLog 디렉토리 생성 (없으면)
mkdir -p [GitRoot]/AgentLog/[작업자GitID]/
```

### 4단계: Start Log 작성

**파일 경로**: `[GitRoot]/AgentLog/[작업자GitID]/YYMMDD.md`

**내용**:
```markdown
# [작업자GitID] YYMMDD

# Start Log
- YYYYMMDD hh:mm
- 작업자: [작업자 아이디]

### To Do

1. "사용자가 요청한 작업 1"
2. "사용자가 요청한 작업 2"
...

```

**중요 사항**:
- 파일이 이미 존재하면 **맨 아래에 추가**
- ToDo는 **작은 단위**로 구체적으로 작성
- 현재 시점에서 해야 할 일만 포함

---

## Compact Log 생성 절차

**실행 시점**: 작업 중간 요약 요청 시 (`요약`, `중간 요약` 등)

### 1단계: 최근 작업 분석
- 최근 Start Log 또는 이전 Compact Log 이후 작업 내용 파악
- 변경된 파일, 작성한 코드, 해결한 문제 등 정리

### 2단계: Compact Log 작성

**파일 경로**: Start Log가 작성된 파일 (예: `AgentLog/[작업자GitID]/251107.md`)

**내용**:
```markdown
## Compact Log N
- YYYYMMDD hh:mm

### 요약 내용

[작업 내용 요약]
- 변경 파일: file1.cpp, file2.h
- 주요 작업: 기능 추가, 버그 수정 등

### Commit 정보

**Summary (EN)**:
Brief description of changes

**세부 내역**:
1. "변경 범주 1"
   - "변경 범주 1의 상세 설명"
2. "변경 범주 2"
   - "변경 범주 2의 상세 설명"
```

### 3단계: 추천 사항 제시
- 다음 ToDo 제시 (이전 ToDo를 완료했다면)
- `/compact` 명령어 사용 추천

---

## End Log 생성 절차

**실행 시점**: 세션 종료 시 (`종료`, `끝` 등)

### 1단계: 최종 Compact Log 생성
- 이전 Compact 이후 변경 사항이 있으면 Compact Log 작성
- 변경 사항이 없으면 Skip

### 2단계: End Log 작성

**파일 경로**: Start Log가 작성된 파일

**내용**:
```markdown
## End Log
- YYYYMMDD hh:mm
- 작업자: [작업자 아이디]

### 오늘 한 일

1. "작업 범주 1"
2. "작업 범주 2"
3. "작업 범주 3"
```

### 3단계: 세션 종료 추천
- 사용자에게 세션 종료를 추천

---

## 파일 구조 예시

```
AgentLog/
├── doppleddiggong/
│   ├── 251107.md      # 오늘의 로그
│   ├── 251106.md      # 어제 로그
│   └── ...
└── another-user/
    └── 251107.md
```

---

## 실행 예시

### Start Log 생성
```bash
# 사용자 명령: "/agent-log start" 또는 세션 시작 시 자동

# 1. Git 정보 확인
git config user.name  # => "doppleddiggong"

# 2. 현재 시간 (UTC+09:00)
# => 20251107 19:30

# 3. 로그 파일 생성
# => AgentLog/doppleddiggong/251107.md
```

### Compact Log 생성
```bash
# 사용자 명령: "요약해줘", "/agent-log compact"

# 1. 최근 작업 분석
# 2. Compact Log 작성 (기존 파일에 추가)
# 3. Commit 메시지 추천
# 4. "/compact" 명령 추천
```

### End Log 생성
```bash
# 사용자 명령: "종료", "/agent-log end"

# 1. 최종 Compact Log (필요 시)
# 2. End Log 작성 (기존 파일에 추가)
# 3. 세션 종료 추천
```

---

## 주의 사항

1. **날짜가 바뀌어도** 현재 진행 중인 Start Log가 있는 파일에 계속 기록
2. **Git Root**를 기준으로 경로 설정
3. **작업자 Git ID**를 정확히 가져오기 (git config user.name)
4. **한국 시간 (UTC+09:00)** 기준으로 날짜/시간 표시
5. **파일이 이미 존재하면 덮어쓰지 말고 추가**

---

## 통합 방법

이 AgentRule을 다른 프로젝트(예: 언리얼 프로젝트)에서 사용하려면:

### 1. Submodule로 추가
```bash
cd /path/to/your/unreal/project
git submodule add https://github.com/your-org/AgentRule.git
```

### 2. .claude 설정 심볼릭 링크
```bash
ln -s AgentRule/.claude .claude
```

### 3. 또는 .claude/hooks에서 참조
```bash
# .claude/hooks/on-session-start.sh
#!/bin/bash
# AgentRule의 agent-log 스킬 자동 실행
claude skill agent-log start
```

이렇게 하면 어떤 프로젝트에서든 동일한 로그 시스템을 사용할 수 있습니다.
