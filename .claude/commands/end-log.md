---
description: 작업 세션을 종료하고 End Log를 생성합니다
---

# End Log 생성

**참고**: `AgentRule/AgentRule/Common/Workflows/agents_log.md`

## 실행 내용

1. 최종 Compact Log 생성 (필요시)
2. Start Log 이후 전체 작업 요약
3. End Log 작성 및 저장
4. 세션 종료 추천

## 작업 지시

다음 절차를 따라 End Log를 생성하세요:

### 1. 최종 Compact Log 확인

마지막 Compact Log 이후 변경 사항이 있는지 확인합니다:
```bash
git status
git diff
```

**변경 사항이 있으면**: Compact Log를 먼저 생성 (`/compact` 참조)
**변경 사항이 없으면**: Skip

### 2. 전체 작업 요약

Start Log 시점부터 현재까지의 모든 작업을 간략하게 정리합니다:
- 완료한 기능
- 수정한 버그
- 추가한 테스트
- 업데이트한 문서
- 기타 주요 변경 사항

### 3. Git 사용자 정보 및 시간 확인
```bash
git config user.name
```

현재 시간 (UTC+09:00) 계산

### 4. 로그 파일 경로 찾기

**중요**: Start Log가 작성된 파일에 추가합니다.
- 예: `AgentLog/[작업자GitID]/[작업자GitID]_251107.md`
- 예시: `AgentLog/dopple/dopple_251109.md`
- 날짜가 바뀌어도 현재 Start Log가 있는 파일에 계속 기록

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

### 참고 사항 (선택사항)

- 미완료 작업
- 다음 세션에서 할 일
- 주의 사항

```

### 6. 사용자에게 알림

End Log가 작성되었음을 알리고, 세션 종료를 추천합니다.

**추천 메시지**:
```
End Log가 작성되었습니다.

파일 위치: AgentLog/[작업자GitID]/[작업자GitID]_YYMMDD.md

세션을 종료하시려면 Claude를 종료하시면 됩니다.
```

---

**중요**:
- End Log는 세션당 한 번만 생성됩니다
- 세션 종료 전에 반드시 작성하여 작업 내용을 기록합니다
- 다른 개발자와 공유할 수 있도록 명확하게 작성합니다
