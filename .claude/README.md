# AgentRule - Claude Skills & Commands

이 디렉토리는 Claude CLI에서 사용할 수 있는 Skills, Commands, Hooks를 포함합니다.

---

## 📁 디렉토리 구조

```
.claude/
├── skills/
│   └── agent-log.md              # Agent Log 시스템 가이드
├── commands/
│   ├── start-log.md              # /start-log - 세션 시작 로그 (수동)
│   ├── compact.md                # /compact - 중간 요약 로그
│   └── end-log.md                # /end-log - 세션 종료 로그 (수동)
├── hooks/
│   ├── on-session-start.md       # 세션 시작 시 자동 실행 ✨
│   └── on-session-end.md         # 세션 종료 시 자동 실행 ✨
└── README.md                     # 이 파일
```

---

## 🚀 사용 방법

### ✨ 완전 자동화 (권장)

**아무것도 하지 않아도 됩니다!**

```bash
# 1. Claude CLI 실행
claude

# → 자동으로 Start Log 생성됨! ✅

# 2. 작업 진행...

# 3. Claude 종료
exit

# → 자동으로 End Log 생성됨! ✅
```

**자동 실행되는 것**:
- ✅ **세션 시작**: `on-session-start.md` hook이 자동으로 Start Log 생성
- ✅ **세션 종료**: `on-session-end.md` hook이 자동으로 End Log 생성
- 📝 **로그 위치**: `AgentLog/[작업자GitID]/YYMMDD.md`

---

### 📝 수동 명령어 (필요 시)

필요한 경우에만 수동으로 실행할 수 있습니다:

#### 1. 세션 시작 (자동 실행되므로 불필요)

```bash
/start-log
```

#### 2. 작업 중간 요약 (유일하게 수동 실행이 필요한 부분)

```bash
/compact
```

또는 자연어로:
```
요약해줘
중간 요약해줘
지금까지 한 일 정리해줘
```

**결과**:
- Compact Log 섹션 추가
- 작업 내용 요약
- Commit 메시지 추천
- 다음 ToDo 제시 (필요시)

#### 3. 세션 종료 (자동 실행되므로 불필요)

```bash
/end-log
```

또는 자연어로:
```
종료
끝
세션 종료
```

**결과**:
- End Log 섹션 추가
- 전체 작업 요약
- 세션 종료 추천

---

### ⚙️ 자동화 동작 방식

1. **세션 시작 시**:
   - `.claude.json`의 `sessionStartPrompt`가 실행됨
   - `.claude/hooks/on-session-start.md` 지침을 따름
   - 자동으로 Start Log 생성
   - 사용자에게 간단히 알림

2. **세션 종료 시**:
   - `.claude.json`의 `sessionEndPrompt`가 실행됨
   - `.claude/hooks/on-session-end.md` 지침을 따름
   - Git 변경사항 분석
   - 자동으로 End Log 생성

---

## 📝 생성되는 로그 파일 예시

**파일 경로**: `AgentLog/doppleddiggong/251107.md`

```markdown
# doppleddiggong 251107

# Start Log
- 20251107 14:30
- 작업자: doppleddiggong

### To Do

1. Agent Log 시스템 구현
2. Claude Skills 생성
3. 테스트 및 문서화

## Compact Log 1
- 20251107 16:45

### 요약 내용

Agent Log 시스템을 위한 Claude Skills와 Commands를 생성했습니다.

**변경 파일**:
- `.claude/skills/agent-log.md`: 로그 시스템 가이드 추가
- `.claude/commands/start-log.md`: Start Log 명령어 추가
- `.claude/commands/compact.md`: Compact Log 명령어 추가
- `.claude/commands/end-log.md`: End Log 명령어 추가

**주요 작업**:
- Agent Log Skill 구현 완료
- 슬래시 커맨드 3개 생성 완료
- 자동 실행 설정 추가

### Commit 정보

**Summary (EN)**:
feat: Add Agent Log automation system for Claude CLI

**세부 내역**:
1. "Agent Log Skills 추가"
   - agent-log.md 스킬 생성
   - Start/Compact/End Log 자동화 가이드 작성
2. "슬래시 커맨드 추가"
   - /start-log, /compact, /end-log 명령어 구현
3. "자동 실행 설정"
   - .claude.json에 sessionStartPrompt 추가

## End Log
- 20251107 18:00
- 작업자: doppleddiggong

### 오늘 한 일

1. "Agent Log 자동화 시스템 구현"
   - Claude Skills 및 Commands 생성
   - 자동 실행 설정 완료
2. "문서화"
   - README 및 가이드 작성
   - 사용 방법 정리
```

---

## 🔧 다른 프로젝트에서 사용하기

AgentRule을 다른 프로젝트(예: 언리얼 엔진 프로젝트)에서 사용하려면:

### 방법 1: Submodule로 추가

```bash
cd /path/to/your/project
git submodule add https://github.com/your-org/AgentRule.git
```

### 방법 2: .claude 디렉토리 심볼릭 링크

```bash
cd /path/to/your/project
ln -s AgentRule/.claude .claude
```

### 방법 3: .claude.json에서 참조

```json
{
  "extends": "./AgentRule/.claude.json",
  "customInstructions": "프로젝트별 추가 지침..."
}
```

이렇게 하면 어떤 프로젝트에서든 동일한 로그 시스템을 자동으로 사용할 수 있습니다!

---

## 📚 참고 문서

- **Log 시스템 상세 규칙**: `AgentRule/Common/Workflows/agents_log.md`
- **기본 페르소나**: `AgentRule/PERSONA.md`
- **Claude 특화 지침**: `AgentRule/CLAUDE.md`

---

## ✅ 주요 기능

- ✅ **자동 Start Log**: 세션 시작 시 자동으로 작업 로그 생성
- ✅ **중간 요약**: `/compact`로 작업 내용 정리 및 Commit 메시지 추천
- ✅ **세션 종료**: `/end-log`로 전체 작업 요약
- ✅ **Git 통합**: Git 사용자 정보 자동 인식
- ✅ **다른 개발자와 공유**: AgentLog 디렉토리로 작업 내용 공유
- ✅ **프로젝트 간 재사용**: Submodule로 어디서든 사용 가능

---

## 🎯 활용 시나리오

1. **언리얼 엔진 프로젝트**에서 Claude로 작업
   - AgentRule을 submodule로 추가
   - 자동으로 작업 로그 생성
   - 팀원들과 AgentLog 공유

2. **여러 프로젝트**에서 동일한 로그 시스템 사용
   - 일관된 작업 기록
   - 에이전트 간 참조 가능

3. **DevOps 파이프라인** 통합
   - AgentLog를 자동으로 수집
   - 개발 현황 리포트 생성
