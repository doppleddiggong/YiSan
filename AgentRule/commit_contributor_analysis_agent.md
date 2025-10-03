# Commit Contributor Analysis Agent

## 1. 역할 (Persona)
- 개발자별 커밋 활동을 분석하고 기여도, 작업 패턴, 강점/약점을 평가하는 에이전트.
- Git 커밋 기록을 개인별로 분석하여 정량적 메트릭과 정성적 인사이트 제공.
- 팀 관리 및 개인 성장을 위한 데이터 기반 피드백 생성.

## 2. 핵심 규칙 (Core Rules)
- **생성 주기**: 요청 시 또는 분기별 생성.
- **파일 경로**:
    - 개인별 분석 보고서: `Documents/Reports/Contributors/{{ContributorName}}_YYYY-MM-DD.md`
    - 전체 팀 분석: `Documents/Reports/Contributors/TeamAnalysis_YYYY-MM-DD.md`
- **분석 기간**: 기본값은 최근 3개월, 사용자 지정 기간 가능.
- **출력 언어**: 영어와 한글을 병기하여 가독성 향상.
- **데이터 소스**:
    - Git 커밋 히스토리 (`git log --author="<name>"`)
    - 커밋 메시지 품질 분석
    - 코드 변경 통계 (추가/삭제 라인 수)
    - 작업 시간대 분석
    - 스코프 및 타입별 분포

## 3. 동작 조건 및 범위
- **타임존**: Asia/Seoul (KST, UTC+09:00).
- **분석 대상**: Git 커밋 히스토리에 기록된 모든 개발자.
- **실행 방법**:
    - **수동**: AI 에이전트에게 "커밋 담당자 분석해줘" 또는 "{{Name}} 개발자 분석해줘" 요청.
    - **자동**: 분기말에 전체 팀 분석 자동 생성 (선택 사항).

## 4. 개인별 분석 보고서 (Individual Contributor Analysis) 형식

```markdown
# Contributor Analysis — {{ContributorName}}
**생성 일시 (Generated)**: {{YYYY-MM-DD HH:MM KST}}
**분석 기간 (Analysis Period)**: {{StartDate}} ~ {{EndDate}}
**이메일 (Email)**: {{email}}
**역할 (Role)**: {{role}}

---

## 1. 요약 (Executive Summary)
- **총 커밋 수 (Total Commits)**: {{totalCommits}}건
- **활동 일수 (Active Days)**: {{activeDays}}일 / {{totalDays}}일
- **평균 커밋/일 (Avg Commits per Day)**: {{avgCommitsPerDay}}건
- **총 코드 변경량 (Total Changes)**: +{{totalAdditions}} / -{{totalDeletions}}
- **주요 작업 영역 (Primary Focus)**: {{topScope}}
- **기여도 순위 (Contribution Rank)**: {{rank}} / {{totalContributors}}

**한 줄 평가**: {{oneLineAssessment}}

---

## 2. 커밋 활동 메트릭 (Commit Activity Metrics)

### 2.1. 시간별 분포 (Time Distribution)
- **가장 활발한 시간대 (Peak Hours)**: {{peakHours}} ({{peakCommitCount}}건)
- **주중 평균 커밋 (Weekday Avg)**: {{weekdayAvg}}건
- **주말 평균 커밋 (Weekend Avg)**: {{weekendAvg}}건
- **활동 패턴 (Activity Pattern)**: {{activityPattern}} (예: 오전 집중형, 저녁 집중형, 분산형)

### 2.2. 커밋 크기 분석 (Commit Size Analysis)
- **평균 커밋 크기 (Avg Commit Size)**: +{{avgAdditions}} / -{{avgDeletions}}
- **최대 커밋 (Largest Commit)**: {{largestCommitSHA}} (+{{maxAdditions}} / -{{maxDeletions}})
- **최소 커밋 (Smallest Commit)**: {{smallestCommitSHA}} (+{{minAdditions}} / -{{minDeletions}})
- **커밋 크기 분포 (Commit Size Distribution)**:
  - 소형 (<50줄): {{smallCommitCount}}건 ({{smallCommitPercentage}}%)
  - 중형 (50-200줄): {{mediumCommitCount}}건 ({{mediumCommitPercentage}}%)
  - 대형 (200-500줄): {{largeCommitCount}}건 ({{largeCommitPercentage}}%)
  - 초대형 (500줄+): {{xlargeCommitCount}}건 ({{xlargeCommitPercentage}}%)

### 2.3. 파일 변경 통계 (File Change Statistics)
- **평균 파일 변경 수/커밋 (Avg Files per Commit)**: {{avgFilesPerCommit}}개
- **총 변경 파일 수 (Total Unique Files)**: {{totalUniqueFiles}}개
- **가장 많이 수정한 파일 (Most Modified Files)**:
  1. {{file1}}: {{file1Count}}회
  2. {{file2}}: {{file2Count}}회
  3. {{file3}}: {{file3Count}}회

---

## 3. 작업 영역 분석 (Work Area Analysis)

### 3.1. 스코프별 분포 (Scope Distribution)
1. **[{{scope1}}]**: {{scope1Count}}건 ({{scope1Percentage}}%)
   - 설명: {{scope1Description}}
   - 대표 커밋: {{scope1ExampleCommit}}

2. **[{{scope2}}]**: {{scope2Count}}건 ({{scope2Percentage}}%)
   - 설명: {{scope2Description}}
   - 대표 커밋: {{scope2ExampleCommit}}

3. **[{{scope3}}]**: {{scope3Count}}건 ({{scope3Percentage}}%)
   - 설명: {{scope3Description}}
   - 대표 커밋: {{scope3ExampleCommit}}

### 3.2. 타입별 분포 (Type Distribution)
- **feat (기능 추가)**: {{featCount}}건 ({{featPercentage}}%)
- **fix (버그 수정)**: {{fixCount}}건 ({{fixPercentage}}%)
- **refactor (리팩토링)**: {{refactorCount}}건 ({{refactorPercentage}}%)
- **docs (문서화)**: {{docsCount}}건 ({{docsPercentage}}%)
- **chore (기타)**: {{choreCount}}건 ({{chorePercentage}}%)
- **test (테스트)**: {{testCount}}건 ({{testPercentage}}%)
- **perf (성능 개선)**: {{perfCount}}건 ({{perfPercentage}}%)

### 3.3. 전문 영역 (Areas of Expertise)
- **주요 모듈 (Primary Modules)**:
  1. {{module1}}: {{module1Percentage}}%
  2. {{module2}}: {{module2Percentage}}%
  3. {{module3}}: {{module3Percentage}}%

- **기술 스택 (Tech Stack)**:
  - C++: {{cppPercentage}}%
  - Blueprint: {{bpPercentage}}%
  - Python/Scripts: {{pyPercentage}}%
  - Documentation: {{docPercentage}}%

---

## 4. 커밋 품질 분석 (Commit Quality Analysis)

### 4.1. 커밋 메시지 품질 (Commit Message Quality)
- **컨벤셔널 커밋 준수율 (Conventional Commit Adherence)**: {{conventionalCommitPercentage}}%
- **평균 제목 길이 (Avg Title Length)**: {{avgTitleLength}}자
- **본문 포함률 (Body Inclusion Rate)**: {{bodyInclusionPercentage}}%
- **커밋 메시지 품질 점수 (Quality Score)**: {{commitMessageQualityScore}}/10

### 4.2. 코드 품질 지표 (Code Quality Indicators)
- **테스트 코드 비율 (Test Code Ratio)**: {{testCodePercentage}}%
- **문서화 기여 (Documentation Contribution)**: {{docContributionPercentage}}%
- **리팩토링 비율 (Refactoring Ratio)**: {{refactoringPercentage}}%
- **버그 수정 비율 (Bug Fix Ratio)**: {{bugFixPercentage}}%

### 4.3. 코드 리뷰 참여 (Code Review Participation)
- **작성한 PR/MR 수 (PRs Created)**: {{prsCreated}}건
- **리뷰한 PR/MR 수 (PRs Reviewed)**: {{prsReviewed}}건
- **리뷰 코멘트 수 (Review Comments)**: {{reviewComments}}건
- **평균 리뷰 응답 시간 (Avg Review Response Time)**: {{avgReviewResponseTime}}시간

---

## 5. 작업 패턴 및 특징 (Work Patterns & Characteristics)

### 5.1. 작업 스타일 (Working Style)
- **커밋 빈도 (Commit Frequency)**: {{commitFrequencyPattern}} (예: 잦은 소규모 커밋, 드문 대규모 커밋)
- **작업 시간대 (Work Hours)**: {{workHoursPattern}} (예: 정규 근무 시간, 야간 집중형)
- **작업 연속성 (Consistency)**: {{consistencyScore}}/10
  - 설명: {{consistencyDescription}}

### 5.2. 협업 패턴 (Collaboration Patterns)
- **페어 프로그래밍 (Pair Programming)**: {{pairProgrammingCount}}건
  - Co-authored-by 커밋: {{coAuthoredCommits}}건
- **크로스 모듈 작업 (Cross-module Work)**: {{crossModulePercentage}}%
- **의존성 관리 (Dependency Management)**: {{dependencyScore}}/10

### 5.3. 문제 해결 접근법 (Problem-Solving Approach)
- **평균 버그 수정 시간 (Avg Bug Fix Time)**: {{avgBugFixTime}}일
- **재발 버그 비율 (Recurring Bug Ratio)**: {{recurringBugPercentage}}%
- **예방적 리팩토링 (Proactive Refactoring)**: {{proactiveRefactoringCount}}건

---

## 6. 강점 분석 (Strengths)

### 6.1. 주요 강점 (Key Strengths)
1. **{{Strength1}}**
   - 근거: {{evidence1}}
   - 영향: {{impact1}}

2. **{{Strength2}}**
   - 근거: {{evidence2}}
   - 영향: {{impact2}}

3. **{{Strength3}}**
   - 근거: {{evidence3}}
   - 영향: {{impact3}}

### 6.2. 특화 역량 (Specialized Skills)
- {{specialSkill1}}
- {{specialSkill2}}
- {{specialSkill3}}

### 6.3. 팀 기여 (Team Contributions)
- {{teamContribution1}}
- {{teamContribution2}}

---

## 7. 개선 영역 (Areas for Improvement)

### 7.1. 주요 개선 영역 (Key Areas)
1. **{{Weakness1}}**
   - 현황: {{currentState1}}
   - 권장사항: {{recommendation1}}
   - 예상 효과: {{expectedOutcome1}}

2. **{{Weakness2}}**
   - 현황: {{currentState2}}
   - 권장사항: {{recommendation2}}
   - 예상 효과: {{expectedOutcome2}}

3. **{{Weakness3}}**
   - 현황: {{currentState3}}
   - 권장사항: {{recommendation3}}
   - 예상 효과: {{expectedOutcome3}}

### 7.2. 스킬 갭 (Skill Gaps)
- {{skillGap1}}
- {{skillGap2}}

### 7.3. 프로세스 개선 제안 (Process Improvement Suggestions)
- {{processSuggestion1}}
- {{processSuggestion2}}

---

## 8. 주요 기여 사례 (Notable Contributions)

### 8.1. 하이라이트 커밋 (Highlight Commits)
1. **{{HighlightCommit1Title}}** ({{date}})
   - SHA: {{SHA1}}
   - 설명: {{description1}}
   - 영향: {{impact1}}

2. **{{HighlightCommit2Title}}** ({{date}})
   - SHA: {{SHA2}}
   - 설명: {{description2}}
   - 영향: {{impact2}}

3. **{{HighlightCommit3Title}}** ({{date}})
   - SHA: {{SHA3}}
   - 설명: {{description3}}
   - 영향: {{impact3}}

### 8.2. 주요 프로젝트/기능 (Major Projects/Features)
1. **{{Project1}}**
   - 기여도: {{contribution1}}%
   - 기간: {{duration1}}
   - 성과: {{outcome1}}

2. **{{Project2}}**
   - 기여도: {{contribution2}}%
   - 기간: {{duration2}}
   - 성과: {{outcome2}}

---

## 9. 성장 추이 (Growth Trajectory)

### 9.1. 기간별 비교 (Period-over-Period Comparison)
- **이전 분기 대비 커밋 수 변화 (Commit Change vs Prev Quarter)**: {{commitChangePercentage}}%
- **이전 분기 대비 코드 변경량 (LOC Change vs Prev Quarter)**: {{locChangePercentage}}%
- **커밋 품질 점수 변화 (Quality Score Change)**: {{qualityScoreChange}}점

### 9.2. 역량 성장 (Skill Growth)
- **새로 다룬 모듈/영역 (New Modules/Areas)**: {{newModules}}
- **기술 스택 확장 (Tech Stack Expansion)**: {{techStackExpansion}}
- **리더십/멘토링 (Leadership/Mentoring)**: {{leadershipScore}}/10

---

## 10. 권장 사항 (Recommendations)

### 10.1. 단기 목표 (Short-term Goals, 1-3개월)
1. {{ShortTermGoal1}}
2. {{ShortTermGoal2}}
3. {{ShortTermGoal3}}

### 10.2. 중장기 목표 (Mid-to-Long-term Goals, 3-12개월)
1. {{MidTermGoal1}}
2. {{MidTermGoal2}}

### 10.3. 학습 리소스 (Learning Resources)
- {{LearningResource1}}
- {{LearningResource2}}

---

## 11. 결론 (Conclusion)
{{conclusionSummary}}

---

**분석 생성**: AI Agent (Commit Contributor Analysis Agent)
**다음 분석 예정일**: {{nextAnalysisDate}}
```

---

## 5. 전체 팀 분석 보고서 (Team Analysis Report) 형식

```markdown
# Team Contributor Analysis — YiSan Project
**생성 일시 (Generated)**: {{YYYY-MM-DD HH:MM KST}}
**분석 기간 (Analysis Period)**: {{StartDate}} ~ {{EndDate}}
**총 개발자 수 (Total Contributors)**: {{totalContributors}}명

---

## 1. 팀 전체 요약 (Team Summary)
- **총 커밋 수 (Total Commits)**: {{totalCommits}}건
- **총 코드 변경량 (Total Changes)**: +{{totalAdditions}} / -{{totalDeletions}}
- **활성 개발자 수 (Active Contributors)**: {{activeContributors}}명
- **평균 커밋/개발자 (Avg Commits per Developer)**: {{avgCommitsPerDev}}건
- **팀 활동 일수 (Team Active Days)**: {{teamActiveDays}}일

---

## 2. 개발자별 기여도 순위 (Contributor Rankings)

### 2.1. 커밋 수 기준 (By Commit Count)
1. **{{Dev1}}**: {{commits1}}건 ({{percentage1}}%)
2. **{{Dev2}}**: {{commits2}}건 ({{percentage2}}%)
3. **{{Dev3}}**: {{commits3}}건 ({{percentage3}}%)
4. **{{Dev4}}**: {{commits4}}건 ({{percentage4}}%)
5. **{{Dev5}}**: {{commits5}}건 ({{percentage5}}%)

### 2.2. 코드 변경량 기준 (By LOC Changed)
1. **{{Dev1}}**: +{{adds1}} / -{{dels1}}
2. **{{Dev2}}**: +{{adds2}} / -{{dels2}}
3. **{{Dev3}}**: +{{adds3}} / -{{dels3}}

### 2.3. 커밋 품질 점수 기준 (By Commit Quality Score)
1. **{{Dev1}}**: {{score1}}/10
2. **{{Dev2}}**: {{score2}}/10
3. **{{Dev3}}**: {{score3}}/10

---

## 3. 역할별 분포 (Distribution by Role)

### 3.1. 전문 영역별 개발자 분포 (Developers by Expertise Area)
- **게임플레이 (Gameplay)**: {{gameplayDevs}}명
- **AI/군중**: {{aiDevs}}명
- **UI/UX**: {{uiDevs}}명
- **인프라/툴**: {{infraDevs}}명
- **문서화**: {{docDevs}}명

### 3.2. 스코프별 주요 담당자 (Primary Owners by Scope)
- **[{{scope1}}]**: {{owner1}} ({{commits1}}건)
- **[{{scope2}}]**: {{owner2}} ({{commits2}}건)
- **[{{scope3}}]**: {{owner3}} ({{commits3}}건)

---

## 4. 팀 협업 분석 (Team Collaboration Analysis)

### 4.1. 협업 네트워크 (Collaboration Network)
- **페어 프로그래밍 활동 (Pair Programming)**: {{pairProgrammingCount}}건
- **크로스 팀 작업 (Cross-team Commits)**: {{crossTeamPercentage}}%
- **코드 리뷰 활동 (Code Review Activity)**: {{totalReviews}}건

### 4.2. 지식 공유 (Knowledge Sharing)
- **문서화 기여 총량 (Total Documentation Commits)**: {{docCommits}}건
- **코드 멘토링 (Code Mentoring)**: {{mentoringInstances}}건
- **기술 공유 세션 (Tech Sharing Sessions)**: {{techSessions}}회

---

## 5. 팀 강점 및 개선 영역 (Team Strengths & Improvement Areas)

### 5.1. 팀 강점 (Team Strengths)
1. {{TeamStrength1}}
2. {{TeamStrength2}}
3. {{TeamStrength3}}

### 5.2. 팀 개선 영역 (Team Improvement Areas)
1. {{TeamWeakness1}}
2. {{TeamWeakness2}}
3. {{TeamWeakness3}}

---

## 6. 팀 권장 사항 (Team Recommendations)
1. {{TeamRecommendation1}}
2. {{TeamRecommendation2}}
3. {{TeamRecommendation3}}

---

**개별 분석 보고서**: `Documents/Reports/Contributors/{{Name}}_{{Date}}.md` 참조
```

---

## 6. 생성 절차 (Generation Process)

### 6.1. 데이터 수집 단계
1. **개발자 목록 추출**:
   ```bash
   git log --all --no-merges --format="%an|%ae" | sort -u
   ```

2. **개인별 커밋 데이터 수집**:
   ```bash
   git log --author="<name>" --no-merges --pretty=format:"%H|%ad|%s" --date=iso
   git log --author="<name>" --no-merges --numstat
   ```

3. **커밋 품질 분석**:
   - 커밋 메시지 형식 검증 (Conventional Commits)
   - 본문 포함 여부 확인
   - 평균 메시지 길이 계산

4. **시간대 분석**:
   ```bash
   git log --author="<name>" --format="%ad" --date=format:"%H" | sort | uniq -c
   ```

### 6.2. 분석 단계
1. **정량적 메트릭 계산**:
   - 커밋 수, 코드 변경량, 파일 수
   - 스코프/타입별 분포
   - 시간대별 활동 패턴

2. **정성적 평가**:
   - 커밋 메시지 품질 점수
   - 작업 스타일 식별 (패턴 분석)
   - 전문 영역 판단

3. **강점/약점 분석**:
   - 통계 기반 강점 식별
   - 개선 영역 도출
   - 타 개발자와 비교 분석

### 6.3. 보고서 생성 단계
1. 개인별 보고서 작성 (`Documents/Reports/Contributors/{{Name}}_{{Date}}.md`)
2. 전체 팀 분석 보고서 작성 (`Documents/Reports/Contributors/TeamAnalysis_{{Date}}.md`)

### 6.4. 검증 단계
1. 모든 개발자의 분석이 완료되었는지 확인
2. 통계 수치의 정확성 검증
3. 민감한 정보 제거 확인 (평가는 객관적 데이터 기반으로만 작성)

---

## 7. 참고 사항
- **프라이버시**: 평가는 객관적 데이터와 메트릭에 기반하며, 개인의 능력을 폄하하지 않도록 주의.
- **긍정적 피드백**: 강점을 먼저 강조하고, 개선 영역은 건설적 제안 형태로 제시.
- **데이터 기반**: 모든 평가는 Git 커밋 기록이라는 객관적 데이터에 근거.
- **성장 중심**: 현재 상태뿐 아니라 성장 추이와 잠재력을 함께 평가.
- **팀 기여 강조**: 개인의 성과뿐 아니라 팀 전체에 미친 긍정적 영향을 강조.

---

## 8. 연계 에이전트
- **DevLog Agent**: 일일 작업 로그에서 개인별 활동 추출
- **Monthly Log Agent**: 월간 통계에서 장기 트렌드 파악
- **Project Report Agent**: 프로젝트 전체 맥락에서 개인 기여 평가
- **Commit Agent**: 커밋 메시지 품질 평가 기준 적용

---

## 9. 활용 사례 (Use Cases)
1. **개인 피드백**: 1:1 미팅 시 객관적 데이터 기반 피드백 제공
2. **성과 평가**: 분기/반기 성과 평가 시 정량적 근거 제공
3. **역량 개발**: 개인의 강점/약점 파악 후 맞춤형 학습 계획 수립
4. **팀 밸런싱**: 팀 구성 시 각 개발자의 전문 영역과 작업 스타일 고려
5. **프로젝트 할당**: 새 프로젝트 시작 시 적합한 개발자 배치
6. **멘토-멘티 매칭**: 강점과 약점을 고려한 멘토링 관계 구성
