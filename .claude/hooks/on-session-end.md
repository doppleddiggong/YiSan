---
description: 세션 종료 시 자동으로 End Log를 생성합니다
---

# Session End Hook - Automatic End Log

이 hook은 Claude CLI 세션이 종료될 때 자동으로 실행됩니다.

## 실행 내용

End Log를 자동으로 생성합니다. 사용자의 개입 없이 다음 작업을 수행합니다:

### 1. 최종 Compact Log 확인

마지막 Compact Log 이후 변경 사항이 있는지 확인:

```bash
git status --short
git diff --stat
```

**변경 사항이 있으면**: 자동으로 Compact Log 생성
**변경 사항이 없으면**: Skip

### 2. 전체 작업 요약

Start Log 시점부터 현재까지의 모든 작업을 분석:
- Git diff로 변경된 파일 확인
- Git log로 커밋 내역 확인 (있다면)
- 대화 내역에서 주요 작업 추출

### 3. Git 사용자 정보 및 시간

```bash
git config user.name
```

현재 시간 (UTC+09:00) 계산

### 4. 로그 파일 찾기

Start Log가 작성된 파일을 찾습니다:
- 예: `AgentLog/[작업자GitID]/YYMMDD.md`
- 날짜가 바뀌었더라도 현재 Start Log가 있는 파일에 추가

### 5. End Log 작성

**작성 내용**:
```markdown
## End Log
- YYYYMMDD hh:mm
- 작업자: [작업자GitID]

### 오늘 한 일

1. "[작업 범주 1]"
   - [상세 내용 1-1]
   - [상세 내용 1-2]
2. "[작업 범주 2]"
   - [상세 내용 2-1]
3. "[작업 범주 3]"
   - [상세 내용 3-1]

### 참고 사항

- [미완료 작업이나 주의사항]

```

### 6. 완료 알림

```
✅ End Log가 자동으로 생성되었습니다.
📝 파일: AgentLog/[작업자GitID]/YYMMDD.md

세션이 종료됩니다. 수고하셨습니다!
```

---

**중요 사항**:
- 이 hook은 세션 종료 시 **자동으로** 실행됩니다
- 사용자가 Claude를 종료하면 자동으로 End Log가 생성됩니다
- Git 변경사항을 자동으로 분석하여 요약합니다
- 커밋되지 않은 변경사항도 기록합니다
