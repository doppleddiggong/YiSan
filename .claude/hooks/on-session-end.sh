#!/bin/bash
# Session End Hook - Automatic End Log Generator
# 세션 종료 시 자동으로 End Log를 생성합니다

set -e

# 1. Git 사용자 정보
GIT_USER=$(git config user.name 2>/dev/null || echo "작업자")

# 2. Git Root 경로
GIT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null || pwd)

# 3. 현재 시간 (한국 시간, UTC+9)
CURRENT_DATE=$(python3 -c "from datetime import datetime, timedelta, timezone; kst = timezone(timedelta(hours=9)); now = datetime.now(kst); print(now.strftime('%y%m%d'))" 2>/dev/null || date +%y%m%d)
CURRENT_TIME=$(python3 -c "from datetime import datetime, timedelta, timezone; kst = timezone(timedelta(hours=9)); now = datetime.now(kst); print(now.strftime('%Y%m%d %H:%M'))" 2>/dev/null || date '+%Y%m%d %H:%M')

# 4. 로그 파일 경로
LOG_FILE="${GIT_ROOT}/Documents/AgentLog/${GIT_USER}/${GIT_USER}_${CURRENT_DATE}.md"

# 5. 로그 파일이 없으면 종료 (Start Log가 없는 경우)
if [ ! -f "${LOG_FILE}" ]; then
    echo "⚠️ Start Log가 없습니다. End Log를 생성하지 않습니다."
    exit 0
fi

# 6. Git 변경사항 분석
CHANGED_FILES=$(git status --short 2>/dev/null | head -20 || echo "")
COMMIT_COUNT=$(git log --oneline --since="today" 2>/dev/null | wc -l || echo "0")

# 7. 작업 요약 생성 (변경된 파일 기반)
WORK_SUMMARY=""
if [ -n "$CHANGED_FILES" ]; then
    WORK_SUMMARY="### 변경된 파일\n\n\`\`\`\n${CHANGED_FILES}\n\`\`\`\n\n"
fi

# 8. End Log 작성
cat >> "${LOG_FILE}" <<EOF

## End Log
- ${CURRENT_TIME}
- 작업자: ${GIT_USER}

### 오늘 한 일

1. [작업 내용을 정리해주세요]

${WORK_SUMMARY}
### 참고 사항

- 커밋 수: ${COMMIT_COUNT}개
- 세션이 정상적으로 종료되었습니다.

EOF

# 9. 성공 메시지
cat <<EOF
{
  "success": true,
  "message": "✅ End Log가 자동으로 생성되었습니다.\n📝 파일: Documents/AgentLog/${GIT_USER}/${GIT_USER}_${CURRENT_DATE}.md\n\n수고하셨습니다!",
  "logFile": "${LOG_FILE}"
}
EOF

exit 0
