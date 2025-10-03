# Project Report Agent

## 1. 역할 (Persona)
- 프로젝트 전체 상태를 분석하고 종합 보고서를 생성하는 에이전트.
- Git 기록, DevLog, 코드베이스 구조를 분석하여 프로젝트 건강도, 진행률, 주요 메트릭 제공.
- 프로젝트 관리자 및 이해관계자를 위한 고수준 인사이트 제공.

## 2. 핵심 규칙 (Core Rules)
- **생성 주기**: 요청 시 또는 주요 마일스톤 완료 시 생성.
- **파일 경로**:
    - 프로젝트 보고서: `Documents/Reports/ProjectReport_YYYY-MM-DD.md`
    - 최신 보고서 링크: `Documents/Reports/LatestReport.md` (심볼릭 링크 또는 최신 보고서 경로 포함)
- **출력 언어**: 영어와 한글을 병기하여 가독성 향상.
- **데이터 소스**:
    - Git 커밋 히스토리 (전체 기간 또는 지정 기간)
    - DevLog 파일들 (일일 및 월간)
    - 코드베이스 구조 분석 (파일 수, 라인 수, 언어별 분포)
    - 이슈 트래커 (선택 사항)
    - 문서화 현황 (Doxygen, README 등)

## 3. 동작 조건 및 범위
- **타임존**: Asia/Seoul (KST, UTC+09:00).
- **분석 기간**: 기본값은 프로젝트 시작부터 현재까지, 사용자 지정 기간 가능.
- **실행 방법**:
    - **수동**: AI 에이전트에게 "프로젝트 보고서 생성해줘" 요청.
    - **자동**: 주요 마일스톤 또는 월말에 자동 생성 (선택 사항).

## 4. 프로젝트 보고서 (Project Report) 형식

```markdown
# Project Report — YiSan
**생성 일시 (Generated)**: {{YYYY-MM-DD HH:MM KST}}
**보고 기간 (Report Period)**: {{StartDate}} ~ {{EndDate}}
**프로젝트 버전 (Version)**: {{ProjectVersion}}
**엔진 버전 (Engine)**: Unreal Engine {{EngineVersion}}

---

## 1. 전체 요약 (Executive Summary)
- **프로젝트 상태 (Status)**: {{Active/OnTrack/AtRisk/Completed}}
- **전체 진행률 (Overall Progress)**: {{ProgressPercentage}}%
- **총 커밋 수 (Total Commits)**: {{totalCommits}}건
- **활동 기간 (Active Period)**: {{activeDays}}일
- **참여 개발자 (Contributors)**: {{contributorCount}}명
- **주요 성과 (Key Achievements)**: {{topAchievements}}

**한 줄 요약**: {{oneLineSummary}}

---

## 2. 프로젝트 메트릭 (Project Metrics)

### 2.1. 코드베이스 현황 (Codebase Status)
- **총 파일 수 (Total Files)**: {{totalFiles}}개
- **총 코드 라인 수 (Total Lines of Code)**: {{totalLOC}}줄
- **언어별 분포 (Language Distribution)**:
  - C++: {{cppPercentage}}% ({{cppLOC}}줄)
  - Blueprint: {{bpPercentage}}% ({{bpAssets}}개 에셋)
  - Python: {{pyPercentage}}% ({{pyLOC}}줄)
  - Markdown: {{mdPercentage}}% ({{mdLOC}}줄)

### 2.2. 커밋 통계 (Commit Statistics)
- **총 커밋 수 (Total Commits)**: {{totalCommits}}건
- **일 평균 커밋 (Daily Average)**: {{avgCommitsPerDay}}건
- **주 평균 커밋 (Weekly Average)**: {{avgCommitsPerWeek}}건
- **월 평균 커밋 (Monthly Average)**: {{avgCommitsPerMonth}}건
- **최초 커밋 (First Commit)**: {{firstCommitDate}} ({{firstCommitSHA}})
- **최근 커밋 (Latest Commit)**: {{latestCommitDate}} ({{latestCommitSHA}})

### 2.3. 타입별 커밋 분포 (Commit Type Distribution)
- **feat (기능 추가)**: {{featCount}}건 ({{featPercentage}}%)
- **fix (버그 수정)**: {{fixCount}}건 ({{fixPercentage}}%)
- **refactor (리팩토링)**: {{refactorCount}}건 ({{refactorPercentage}}%)
- **docs (문서화)**: {{docsCount}}건 ({{docsPercentage}}%)
- **chore (기타)**: {{choreCount}}건 ({{chorePercentage}}%)
- **test (테스트)**: {{testCount}}건 ({{testPercentage}}%)
- **perf (성능 개선)**: {{perfCount}}건 ({{perfPercentage}}%)

### 2.4. 스코프별 활동 (Scope Activity)
1. **[{{scope1}}]**: {{scope1Count}}건 ({{scope1Percentage}}%)
2. **[{{scope2}}]**: {{scope2Count}}건 ({{scope2Percentage}}%)
3. **[{{scope3}}]**: {{scope3Count}}건 ({{scope3Percentage}}%)
4. **[{{scope4}}]**: {{scope4Count}}건 ({{scope4Percentage}}%)
5. **[{{scope5}}]**: {{scope5Count}}건 ({{scope5Percentage}}%)

---

## 3. 모듈별 현황 (Module Status)

### 3.1. YiSan (Game Module)
- **파일 수 (Files)**: {{yisanFiles}}개
- **코드 라인 수 (LOC)**: {{yisanLOC}}줄
- **최근 변경 (Recent Changes)**: {{yisanRecentChanges}}
- **주요 기능 (Key Features)**:
  - {{feature1}}
  - {{feature2}}
  - {{feature3}}
- **상태 (Status)**: {{yisanStatus}}

### 3.2. CoffeeLibrary (Utility Module)
- **파일 수 (Files)**: {{coffeeFiles}}개
- **코드 라인 수 (LOC)**: {{coffeeLOC}}줄
- **최근 변경 (Recent Changes)**: {{coffeeRecentChanges}}
- **주요 기능 (Key Features)**:
  - {{feature1}}
  - {{feature2}}
- **상태 (Status)**: {{coffeeStatus}}

### 3.3. LatteLibrary (Gameplay Module)
- **파일 수 (Files)**: {{latteFiles}}개
- **코드 라인 수 (LOC)**: {{latteLOC}}줄
- **최근 변경 (Recent Changes)**: {{latteRecentChanges}}
- **주요 기능 (Key Features)**:
  - {{feature1}}
  - {{feature2}}
  - {{feature3}}
- **상태 (Status)**: {{latteStatus}}

### 3.4. CoffeeToolbar (Editor Plugin)
- **파일 수 (Files)**: {{toolbarFiles}}개
- **코드 라인 수 (LOC)**: {{toolbarLOC}}줄
- **최근 변경 (Recent Changes)**: {{toolbarRecentChanges}}
- **주요 기능 (Key Features)**:
  - {{feature1}}
- **상태 (Status)**: {{toolbarStatus}}

---

## 4. 주요 성과 및 마일스톤 (Key Achievements & Milestones)

### 4.1. 완료된 주요 기능 (Completed Features)
1. **{{Feature1}}**
   - 완료 일자: {{date}}
   - 설명: {{description}}
   - 관련 커밋: {{commits}}

2. **{{Feature2}}**
   - 완료 일자: {{date}}
   - 설명: {{description}}
   - 관련 커밋: {{commits}}

3. **{{Feature3}}**
   - 완료 일자: {{date}}
   - 설명: {{description}}
   - 관련 커밋: {{commits}}

### 4.2. 주요 버그 수정 (Major Bug Fixes)
1. **{{BugFix1}}** ({{date}})
2. **{{BugFix2}}** ({{date}})
3. **{{BugFix3}}** ({{date}})

### 4.3. 성능 개선 (Performance Improvements)
1. **{{PerfImprovement1}}** ({{date}})
2. **{{PerfImprovement2}}** ({{date}})

---

## 5. 문서화 현황 (Documentation Status)

### 5.1. 코드 문서화 (Code Documentation)
- **Doxygen 문서**: {{doxygenStatus}}
  - 경로: `{{doxygenUrl}}`
  - 마지막 업데이트: {{doxygenLastUpdate}}
- **문서화율 (Documentation Coverage)**: {{docCoveragePercentage}}%

### 5.2. 프로젝트 문서 (Project Documentation)
- **README.md**: {{readmeStatus}}
- **AGENTS.md**: {{agentsStatus}}
- **AgentRule 가이드**: {{agentRuleCount}}개
- **DevLog**: {{devLogCount}}개 (일일), {{monthlyLogCount}}개 (월간)
- **기타 문서**: {{otherDocsCount}}개

---

## 6. 기술 부채 및 리스크 (Technical Debt & Risks)

### 6.1. 식별된 기술 부채 (Identified Technical Debt)
1. **{{TechDebt1}}**
   - 설명: {{description}}
   - 영향도: {{impact}}
   - 우선순위: {{priority}}
   - 예상 해결 시간: {{estimatedTime}}

2. **{{TechDebt2}}**
   - 설명: {{description}}
   - 영향도: {{impact}}
   - 우선순위: {{priority}}
   - 예상 해결 시간: {{estimatedTime}}

### 6.2. 리스크 평가 (Risk Assessment)
1. **{{Risk1}}**
   - 확률: {{probability}}
   - 영향: {{impact}}
   - 완화책: {{mitigation}}

2. **{{Risk2}}**
   - 확률: {{probability}}
   - 영향: {{impact}}
   - 완화책: {{mitigation}}

---

## 7. 품질 지표 (Quality Indicators)

### 7.1. 코드 품질 (Code Quality)
- **컨벤션 준수율 (Convention Adherence)**: {{conventionAdherencePercentage}}%
- **주석 비율 (Comment Ratio)**: {{commentRatioPercentage}}%
- **평균 함수 길이 (Avg Function Length)**: {{avgFunctionLength}}줄
- **평균 파일 크기 (Avg File Size)**: {{avgFileSize}}줄

### 7.2. 커밋 품질 (Commit Quality)
- **컨벤셔널 커밋 준수율 (Conventional Commit Adherence)**: {{conventionalCommitPercentage}}%
- **평균 커밋 크기 (Avg Commit Size)**: +{{avgAdditions}} / -{{avgDeletions}}
- **커밋 메시지 품질 점수 (Commit Message Quality)**: {{commitMessageQualityScore}}/10

---

## 8. 팀 협업 현황 (Team Collaboration Status)

### 8.1. 개발자별 기여도 (Contributor Statistics)
- **총 개발자 수 (Total Contributors)**: {{contributorCount}}명
- **활성 개발자 수 (Active Contributors)**: {{activeContributorCount}}명
- **기여도 상위 5명 (Top 5 Contributors)**:
  1. {{contributor1}}: {{commits1}}건 ({{percentage1}}%)
  2. {{contributor2}}: {{commits2}}건 ({{percentage2}}%)
  3. {{contributor3}}: {{commits3}}건 ({{percentage3}}%)
  4. {{contributor4}}: {{commits4}}건 ({{percentage4}}%)
  5. {{contributor5}}: {{commits5}}건 ({{percentage5}}%)

### 8.2. 협업 패턴 (Collaboration Patterns)
- **코드 리뷰 활동 (Code Review Activity)**: {{codeReviewCount}}건
- **이슈 해결 (Issue Resolution)**: {{resolvedIssues}}건 / {{totalIssues}}건
- **평균 이슈 해결 시간 (Avg Issue Resolution Time)**: {{avgResolutionTime}}일

---

## 9. 미해결 사항 (Outstanding Items)

### 9.1. TODO 목록 (TODO List)
- [ ] {{TODO1}} (출처: {{source}}, 우선순위: {{priority}})
- [ ] {{TODO2}} (출처: {{source}}, 우선순위: {{priority}})
- [ ] {{TODO3}} (출처: {{source}}, 우선순위: {{priority}})

### 9.2. 진행 중인 작업 (In-Progress Tasks)
- {{InProgressTask1}} (담당: {{assignee}}, 진행률: {{progress}}%)
- {{InProgressTask2}} (담당: {{assignee}}, 진행률: {{progress}}%)

---

## 10. 다음 단계 및 권장사항 (Next Steps & Recommendations)

### 10.1. 단기 목표 (Short-term Goals, 1-4주)
1. {{ShortTermGoal1}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

2. {{ShortTermGoal2}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

### 10.2. 중기 목표 (Mid-term Goals, 1-3개월)
1. {{MidTermGoal1}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

2. {{MidTermGoal2}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

### 10.3. 장기 목표 (Long-term Goals, 3개월+)
1. {{LongTermGoal1}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

2. {{LongTermGoal2}}
   - 근거: {{rationale}}
   - 예상 효과: {{expectedOutcome}}

---

## 11. 결론 (Conclusion)
{{conclusionSummary}}

---

**보고서 생성**: AI Agent (Project Report Agent)
**다음 보고서 예정일**: {{nextReportDate}}
```

---

## 5. 생성 절차 (Generation Process)

### 5.1. 데이터 수집 단계
1. **Git 데이터 수집**:
   ```bash
   git log --all --no-merges --pretty=format:"%H|%an|%ae|%ad|%s" --date=iso
   git log --all --no-merges --numstat --pretty=format:"%H"
   git shortlog -sn --all --no-merges
   ```

2. **코드베이스 분석**:
   ```bash
   cloc Source/ Plugins/ --by-file --json
   find Source/ Plugins/ -type f -name "*.h" -o -name "*.cpp" | wc -l
   ```

3. **DevLog 분석**:
   - `Documents/DevLog/_Last30Summary.md` 읽기
   - `Documents/DevLog/Monthly/*.md` 파일들 읽기
   - 최근 일일 DevLog 파일들 읽기

4. **문서화 상태 확인**:
   - Doxygen 문서 생성 여부 확인
   - README, AGENTS.md 업데이트 일자 확인
   - AgentRule 파일들 개수 확인

### 5.2. 분석 단계
1. **메트릭 계산**:
   - 커밋 통계 (총 개수, 평균, 분포)
   - 코드 라인 수 및 언어별 분포
   - 타입별, 스코프별 커밋 분류

2. **트렌드 분석**:
   - 시간별 커밋 추세
   - 주요 활동 영역 식별
   - 진행률 계산

3. **품질 평가**:
   - 커밋 메시지 품질 점수
   - 코드 컨벤션 준수율
   - 문서화 커버리지

4. **리스크 식별**:
   - 미해결 TODO 분석
   - 기술 부채 식별
   - 차단 요인 평가

### 5.3. 보고서 생성 단계
1. 위 형식에 따라 마크다운 파일 생성
2. `Documents/Reports/` 디렉토리에 저장
3. `Documents/Reports/LatestReport.md` 업데이트

### 5.4. 검증 단계
1. 모든 섹션이 채워졌는지 확인
2. 통계 수치의 정확성 검증
3. 링크 및 참조 유효성 확인

---

## 6. 참고 사항
- 보고서 생성 시 `Documents/DevLog/_Last30Summary.md`, 월간 로그, 최근 커밋 기록을 종합 분석.
- 프로젝트 상태는 진행률, 리스크, 활동 추세를 종합하여 결정.
- 주요 성과는 커밋 크기, 영향 범위, 중요도를 기준으로 선정.
- 보고서는 프로젝트 관리자 및 이해관계자가 이해하기 쉽도록 고수준 요약 중심으로 작성.

---

## 7. 연계 에이전트
- **DevLog Agent**: 일일 로그 데이터 소스
- **Monthly Log Agent**: 월간 통계 데이터 소스
- **Commit Contributor Analysis Agent**: 개발자별 기여 데이터 참조
- **Commit Agent**: 커밋 메시지 품질 평가 기준 참조
