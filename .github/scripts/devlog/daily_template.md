# Daily DevLog — {{ date }} ({{ weekday }})

**범위**: {{ date_from }} ~ {{ date_to }}
**브랜치**: {{ branch }} / 베이스: {{ base_branch }}
**릴리즈 타겟**: {{ release_tag }}

---

## 1. 오늘의 핵심 변경 (Top Changes)

{% if top_changes %}
{% for change in top_changes %}
- [{{ change.type }}] {{ change.summary }}{% if change.pr_number %} (#{{ change.pr_number }}){% endif %} — 영향: {{ change.impact }}
{% endfor %}
{% else %}
- 변경 사항 없음
{% endif %}

### Commit Heatmap
- 총 커밋: {{ commit_count }}
- 변경 라인: +{{ added }} / -{{ deleted }}
- 영향 파일: {{ hotspot_files|join(', ') if hotspot_files else 'N/A' }}

---

## 2. 시스템 영향도 (Impact)

### 성능
{% if load_time_now %}
- 로딩: {{ load_time_prev }}s → {{ load_time_now }}s ({{ delta_pct }}%)
{% else %}
- 로딩: 데이터 없음
{% endif %}

### 안정성
{% if crash_now is not none %}
- 크래시: {{ crash_prev }} → {{ crash_now }}
- 실패 빌드: {{ build_fail_count }}
{% else %}
- 크래시: 데이터 없음
{% endif %}

### 네트워크
{% if rtt_ms %}
- 평균 RTT: {{ rtt_ms }}ms
- 실패율: {{ net_fail_pct }}%
{% else %}
- 네트워크: 데이터 없음
{% endif %}

---

## 3. 검증 (Verification)

### 빌드 (UE5)
{% if ubt_target %}
- Target: {{ ubt_target }}
- 쿠킹 결과: {{ cook_summary }}
{% else %}
- 빌드 정보 없음
{% endif %}

### 테스트
{% if pass is not none or fail is not none %}
- 단위/통합/에디터 테스트: {{ pass }}/{{ fail }}
{% if coverage %}
- 커버리지: {{ coverage }}%
{% endif %}
{% else %}
- 테스트 결과 없음
{% endif %}

### 정적분석
{% if warn_now is not none %}
- 경고: {{ warn_prev }} → {{ warn_now }}
- 신규 심각도(High): {{ high_new }}
{% else %}
- 정적분석 결과 없음
{% endif %}

---

## 4. 코드 문서화 변화 (Doxygen Delta)

{% if api_added or api_changed or api_removed %}
- 신규 API: {{ api_added }}
- 변경: {{ api_changed }}
- 폐기: {{ api_removed }}

{% if notable_api_list %}
### 주목 API
{% for api in notable_api_list %}
- {{ api }}
{% endfor %}
{% endif %}
{% else %}
- API 변화 없음
{% endif %}

---

## 5. 리팩토링·위험 이슈

### 리팩토링
{% if refactor_items %}
{% for item in refactor_items %}
- {{ item }}
{% endfor %}
{% else %}
- 리팩토링 없음
{% endif %}

### 위험
{% if risk_items %}
{% for item in risk_items %}
- {{ item }}
{% endfor %}

**대응**: {{ mitigation if mitigation else 'N/A' }}
{% else %}
- 위험 항목 없음
{% endif %}

---

## 6. 내일(Next)·미진(Action)

### Next
{% if next_items %}
{% for item in next_items %}
- {{ item }}
{% endfor %}
{% else %}
- 계획된 작업 없음
{% endif %}

### 미진
{% if pending_items %}
{% for item in pending_items %}
- {{ item }}
{% endfor %}
{% else %}
- 미진 작업 없음
{% endif %}

---

## 7. Mermaid 개요도

```mermaid
flowchart LR
  Dev[Commits: {{ commit_count }}] --> Build[UE5 Build/Cook]
  Build --> Test[Tests: {{ pass if pass else 0 }}/{{ fail if fail else 0 }}]
  Test --> Doc[Doxygen Update]
  Doc --> Daily[Daily DevLog]

  style Dev fill:#e1f5ff
  style Build fill:#{% if ubt_target %}d4edda{% else %}f8d7da{% endif %}
  style Test fill:#{% if fail == 0 %}d4edda{% else %}fff3cd{% endif %}
  style Doc fill:#e2e3e5
  style Daily fill:#d1ecf1
```

---

**생성 시간**: {{ generation_time }}
