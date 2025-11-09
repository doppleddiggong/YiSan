#!/bin/bash
# Session Start Hook - Automatic Start Log Generator
# 세션 시작 시 자동으로 Start Log를 생성합니다

set -e

# 1. Git 사용자 정보 가져오기
GIT_USER=$(git config user.name 2>/dev/null || echo "작업자")
GIT_EMAIL=$(git config user.email 2>/dev/null || echo "")

# 2. Git Root 경로
GIT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null || pwd)

# 3. 현재 시간 (한국 시간, UTC+9)
# Windows에서는 date 명령이 다르므로 Python 사용
CURRENT_DATE=$(python3 -c "from datetime import datetime, timedelta, timezone; kst = timezone(timedelta(hours=9)); now = datetime.now(kst); print(now.strftime('%y%m%d'))" 2>/dev/null || date +%y%m%d)
CURRENT_TIME=$(python3 -c "from datetime import datetime, timedelta, timezone; kst = timezone(timedelta(hours=9)); now = datetime.now(kst); print(now.strftime('%Y%m%d %H:%M'))" 2>/dev/null || date '+%Y%m%d %H:%M')

# 4. AgentLog 디렉토리 생성
AGENT_LOG_DIR="${GIT_ROOT}/Documents/AgentLog/${GIT_USER}"
mkdir -p "${AGENT_LOG_DIR}"

# 5. 로그 파일 경로
LOG_FILE="${AGENT_LOG_DIR}/${GIT_USER}_${CURRENT_DATE}.md"

# 6. Start Log 작성
if [ -f "${LOG_FILE}" ]; then
    # 파일이 이미 존재하면 추가
    echo "" >> "${LOG_FILE}"
    echo "# Start Log" >> "${LOG_FILE}"
    echo "- ${CURRENT_TIME}" >> "${LOG_FILE}"
    echo "- 작업자: ${GIT_USER}" >> "${LOG_FILE}"
    echo "" >> "${LOG_FILE}"
    echo "### To Do" >> "${LOG_FILE}"
    echo "" >> "${LOG_FILE}"
    echo "1. [사용자의 요청을 기다리는 중...]" >> "${LOG_FILE}"
    echo "" >> "${LOG_FILE}"
else
    # 파일이 없으면 새로 생성
    cat > "${LOG_FILE}" <<EOF
# ${GIT_USER} (${CURRENT_DATE})

# Start Log
- ${CURRENT_TIME}
- 작업자: ${GIT_USER}

### To Do

1. [사용자의 요청을 기다리는 중...]

EOF
fi

# 7. 환경변수 설정 (선택적)
if [ -n "$CLAUDE_ENV_FILE" ]; then
    echo "export AGENT_LOG_FILE=\"${LOG_FILE}\"" >> "$CLAUDE_ENV_FILE"
    echo "export AGENT_LOG_DATE=\"${CURRENT_DATE}\"" >> "$CLAUDE_ENV_FILE"
fi

# 8. 성공 메시지 (JSON 출력)
cat <<EOF
{
  "success": true,
  "message": "✅ Start Log가 자동으로 생성되었습니다.\n📝 파일: Documents/AgentLog/${GIT_USER}/${GIT_USER}_${CURRENT_DATE}.md",
  "logFile": "${LOG_FILE}"
}
EOF

exit 0
