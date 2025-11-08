# YiSan Development Documentation

환영합니다! YiSan 프로젝트의 개발 문서 사이트입니다.

## 문서 구조

이 사이트는 YiSan 프로젝트의 개발 과정을 투명하게 기록하고 공유하기 위한 공간입니다.

### 📅 DevLog (개발 일지)
- **Daily**: 일별 개발 로그 - 커밋, 빌드, 테스트 결과를 자동으로 수집하여 매일 생성
- **Weekly**: 주간 요약 - 한 주간의 개발 내용을 종합하여 피드백과 함께 제공
- **Monthly**: 월간 리포트 - 한 달간의 주요 성과와 시스템 변화 요약

### 📋 Planning (기획 문서)
- 시스템 설계 문서
- API 문서
- 기능 명세서
- 발표 자료

### 🔍 System Review
- 주간/월간 시스템 리뷰
- 아키텍처 변화 분석
- 성능 및 안정성 메트릭
- 리스크 평가 및 대응 계획

## 관련 링크

- **GitHub Repository**: [doppleddiggong/YiSan](https://github.com/doppleddiggong/YiSan)
- **Doxygen 코드 문서**: [API Documentation](https://doppleddiggong.github.io/YiSan/doxygen/)

## 자동화 시스템

이 문서는 다음과 같은 자동화 시스템으로 관리됩니다:

1. **Daily DevLog**: 매일 자동으로 Git 커밋, 빌드 결과, 테스트 결과를 수집하여 문서화
2. **Weekly Summary**: 7일간의 데이터를 집계하여 주간 리포트 자동 생성
3. **System Review**: 시스템 변화, 성능 메트릭, 위험도를 분석하여 리뷰 문서 생성
4. **Discord Integration**: 일일/주간 리포트를 Discord Webhook으로 자동 전송

---

**마지막 업데이트**: {{ site.time | date: "%Y-%m-%d" }}
