# YiSan Development Documentation

환영합니다! YiSan 프로젝트의 개발 문서 사이트입니다.

## 문서 구조

이 사이트는 YiSan 프로젝트의 개발 과정을 투명하게 기록하고 공유하기 위한 공간입니다.

### 📅 DevLog (개발 일지)
- **Daily**: 일별 개발 로그 - 커밋, 빌드, 테스트 결과를 자동으로 수집하여 매일 생성
  - 선택적 GPT 피드백: 개발자 성찰 질문, 대안 제시, 학습 포인트
- **Weekly**: 주간 요약 - 한 주간의 개발 내용을 종합 분석
  - 기본 GPT 회고: 작업 패턴 분석, 성장 피드백, 다음 주 제안
- **상세 가이드**: [워크플로우 사용 가이드](DevLog/WORKFLOW_GUIDE.md)

### 📋 Planning (기획 문서)
- 시스템 설계 문서
- API 문서
- 기능 명세서
- 발표 자료


## 관련 링크

- **GitHub Repository**: [doppleddiggong/YiSan](https://github.com/doppleddiggong/YiSan)
- **Doxygen 코드 문서**: [API Documentation](https://doppleddiggong.github.io/YiSan/doxygen/)

## 자동화 시스템

이 문서는 다음과 같은 GitHub Actions 워크플로우로 자동 관리됩니다:

### 1. Daily DevLog
- **실행**: 매일 KST 오전 9시
- **기능**: Git 커밋, 변경 파일 자동 수집 및 문서화
- **옵션**: GPT-4 성장 피드백 (수동 실행 시 선택 가능)
- **출력**: `Documents/DevLog/YYYY-MM-DD.md`

### 2. Weekly Report
- **실행**: 매주 일요일 KST 오후 11시
- **기능**: 주간 작업 통합 분석 + GPT-4 회고 피드백 (기본 포함)
- **출력**: `Documents/DevLog/YYYY-WXX-Summary.md`

### 3. Discord Integration
- 일일/주간 리포트를 Discord Webhook으로 자동 전송 (선택 사항)

**자세한 사용법**: [DevLog 워크플로우 가이드](DevLog/WORKFLOW_GUIDE.md)

---

**마지막 업데이트**: 2025-11-08
