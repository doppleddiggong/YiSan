# Weekly DevLog — {{ week_label }} ({{ date_from }} ~ {{ date_to }})

**주간 요약**: {{ summary }}

---

## 📊 주간 통계 (Weekly Stats)

### 커밋 활동
- **총 커밋**: {{ total_commits }}개
- **변경 라인**: +{{ total_added }} / -{{ total_deleted }}
- **활동 일수**: {{ active_days }}/7일
- **주요 작업자**: {{ top_authors|join(', ') if top_authors else 'N/A' }}

### 커밋 타입 분포
{% if commit_type_stats %}
| 타입 | 개수 | 비율 |
|------|------|------|
{% for type, count, pct in commit_type_stats %}
| {{ type }} | {{ count }} | {{ pct }}% |
{% endfor %}
{% else %}
- 데이터 없음
{% endif %}

### Hotspot 파일 (상위 10개)
{% if hotspot_files %}
| 파일 | 변경 라인 | 변경 빈도 |
|------|-----------|-----------|
{% for file, lines, freq in hotspot_files %}
| `{{ file }}` | {{ lines }} | {{ freq }}회 |
{% endfor %}
{% else %}
- 변경된 파일 없음
{% endif %}

---

## 🎯 주요 성과 (Key Achievements)

### 신규 기능 (Features)
{% if features %}
{% for item in features %}
- **{{ item.title }}**{% if item.pr %} (#{{ item.pr }}){% endif %}
  - {{ item.description }}
{% endfor %}
{% else %}
- 이번 주 신규 기능 없음
{% endif %}

### 버그 수정 (Fixes)
{% if fixes %}
{% for item in fixes %}
- **{{ item.title }}**{% if item.pr %} (#{{ item.pr }}){% endif %}
  - {{ item.description }}
{% endfor %}
{% else %}
- 이번 주 버그 수정 없음
{% endif %}

### 리팩토링 (Refactoring)
{% if refactors %}
{% for item in refactors %}
- **{{ item.title }}**{% if item.pr %} (#{{ item.pr }}){% endif %}
  - {{ item.description }}
{% endfor %}
{% else %}
- 이번 주 리팩토링 없음
{% endif %}

### 성능 개선 (Performance)
{% if performance %}
{% for item in performance %}
- **{{ item.title }}**{% if item.pr %} (#{{ item.pr }}){% endif %}
  - {{ item.description }}
{% endfor %}
{% else %}
- 이번 주 성능 개선 없음
{% endif %}

---

## 📈 시스템 건강도 (System Health)

### 빌드 안정성
{% if build_stats %}
- 성공: {{ build_stats.success }}회
- 실패: {{ build_stats.failure }}회
- 성공률: {{ build_stats.success_rate }}%
{% else %}
- 빌드 데이터 없음
{% endif %}

### 테스트 결과
{% if test_stats %}
- 총 테스트: {{ test_stats.total }}개
- 성공: {{ test_stats.pass }}개
- 실패: {{ test_stats.fail }}개
- 성공률: {{ test_stats.success_rate }}%
{% else %}
- 테스트 데이터 없음
{% endif %}

### 코드 품질
{% if quality_stats %}
- 경고: {{ quality_stats.warnings }}개 (전주 대비 {% if quality_stats.warnings_delta > 0 %}+{% endif %}{{ quality_stats.warnings_delta }})
- 심각도 High: {{ quality_stats.high_issues }}개
{% else %}
- 정적분석 데이터 없음
{% endif %}

---

## 🔥 주목할 변화 (Notable Changes)

### 아키텍처 변화
{% if architecture_changes %}
{% for change in architecture_changes %}
- {{ change }}
{% endfor %}
{% else %}
- 이번 주 아키텍처 변화 없음
{% endif %}

### API 변화
{% if api_changes %}
- 신규 API: {{ api_changes.added }}개
- 변경된 API: {{ api_changes.changed }}개
- 폐기된 API: {{ api_changes.removed }}개

{% if api_changes.notable %}
**주목할 API:**
{% for api in api_changes.notable %}
- `{{ api.name }}` - {{ api.description }}
{% endfor %}
{% endif %}
{% else %}
- API 변화 없음
{% endif %}

---

## 🚧 이슈 및 위험 (Issues & Risks)

### 발견된 이슈
{% if issues %}
{% for issue in issues %}
- **{{ issue.title }}** ({{ issue.severity }})
  - 설명: {{ issue.description }}
  - 상태: {{ issue.status }}
{% endfor %}
{% else %}
- 발견된 이슈 없음
{% endif %}

### 위험 요소
{% if risks %}
{% for risk in risks %}
- **{{ risk.title }}** ({{ risk.level }})
  - 내용: {{ risk.description }}
  - 대응: {{ risk.mitigation }}
{% endfor %}
{% else %}
- 위험 요소 없음
{% endif %}

---

## 📝 다음 주 계획 (Next Week)

### 우선순위 작업
{% if next_week_priority %}
{% for item in next_week_priority %}
1. {{ item }}
{% endfor %}
{% else %}
- TBD
{% endif %}

### 기술 부채 해결
{% if tech_debt %}
{% for item in tech_debt %}
- {{ item }}
{% endfor %}
{% else %}
- 없음
{% endif %}

---

## 💡 회고 및 피드백 (Reflection & Feedback)

### 이번 주 회고 질문

{% if reflection_questions %}
{% for question in reflection_questions %}
**Q: {{ question.q }}**

_답변을 작성해주세요:_

---

{% endfor %}
{% else %}
1. **이번 주 가장 큰 성과는 무엇이었나요?**

   _답변을 작성해주세요_

2. **어떤 부분에서 어려움을 겪었나요?**

   _답변을 작성해주세요_

3. **다음 주에 개선하고 싶은 점은?**

   _답변을 작성해주세요_
{% endif %}

---

## 📊 Mermaid 주간 요약

```mermaid
graph TB
    subgraph "Weekly Summary"
        A[총 커밋: {{ total_commits }}]
        B[활동 일수: {{ active_days }}/7]
        C[주요 타입]
    end

    subgraph "타입 분포"
    {% if commit_type_stats %}
    {% for type, count, pct in commit_type_stats[:3] %}
        C --> {{ type|upper }}[{{ type }}: {{ count }}개]
    {% endfor %}
    {% endif %}
    end

    subgraph "시스템 건강도"
    {% if build_stats %}
        D[빌드 성공률: {{ build_stats.success_rate }}%]
    {% endif %}
    {% if test_stats %}
        E[테스트 성공률: {{ test_stats.success_rate }}%]
    {% endif %}
    end

    style A fill:#e1f5ff
    style B fill:#ffe1f5
    style C fill:#f5ffe1
    {% if build_stats and build_stats.success_rate >= 80 %}
    style D fill:#d4edda
    {% else %}
    style D fill:#f8d7da
    {% endif %}
    {% if test_stats and test_stats.success_rate >= 80 %}
    style E fill:#d4edda
    {% else %}
    style E fill:#f8d7da
    {% endif %}
```

---

**생성 시간**: {{ generation_time }}
**Daily Logs**: {% for day in daily_log_files %}[{{ day }}]({{ day }}.md) {% endfor %}
