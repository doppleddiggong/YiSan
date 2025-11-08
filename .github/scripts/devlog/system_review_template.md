# System Review — {{ period }} ({{ date_from }} ~ {{ date_to }})

> 시스템 상태 리포트: 아키텍처 변화, 코드 건강도, 성능 메트릭, 리스크 평가

---

## 1. 아키텍처 변화 요약 (Structural Delta)

### 신규/삭제 모듈
{% if new_subsystems %}
**신규 서브시스템**:
{% for subsystem in new_subsystems %}
- `{{ subsystem.name }}` - {{ subsystem.description }}
{% endfor %}
{% else %}
- 신규 서브시스템 없음
{% endif %}

{% if removed_modules %}
**삭제/흡수된 모듈**:
{% for module in removed_modules %}
- `{{ module.name }}` - {{ module.reason }}
{% endfor %}
{% else %}
- 삭제된 모듈 없음
{% endif %}

### 주요 API 변경
{% if api_changes %}
- 신규 API: {{ api_changes.added }}개
- 변경된 API: {{ api_changes.changed }}개
- 폐기된 API: {{ api_changes.removed }}개

{% if api_changes.notable %}
**주목할 API 변경**:
| API | 타입 | 설명 |
|-----|------|------|
{% for api in api_changes.notable %}
| `{{ api.name }}` | {{ api.type }} | {{ api.description }} |
{% endfor %}
{% endif %}
{% else %}
- API 변화 없음
{% endif %}

### 영향 범위
{% if affected_areas %}
{% for area in affected_areas %}
- **{{ area.name }}**: {{ area.impact }}
{% endfor %}
{% else %}
- 영향 없음
{% endif %}

---

## 2. 코드베이스 건강도 (Code Health)

### Hotspot 분석
{% if hotspot_analysis %}
**상위 10개 Hotspot 파일**:

| 파일 | 변경 빈도 | 총 변경 라인 | 복잡도 | 위험도 |
|------|-----------|--------------|--------|--------|
{% for file in hotspot_analysis %}
| `{{ file.path }}` | {{ file.frequency }}회 | {{ file.lines }} | {{ file.complexity }} | {{ file.risk_level }} |
{% endfor %}

**분석**:
{{ hotspot_summary }}
{% else %}
- Hotspot 분석 데이터 없음
{% endif %}

### 코드 품질 메트릭
{% if quality_metrics %}
| 메트릭 | 이전 | 현재 | 변화 |
|--------|------|------|------|
| 경고 (Warnings) | {{ quality_metrics.warnings_prev }} | {{ quality_metrics.warnings_now }} | {% if quality_metrics.warnings_delta > 0 %}+{% endif %}{{ quality_metrics.warnings_delta }} |
| 심각도 High | {{ quality_metrics.high_prev }} | {{ quality_metrics.high_now }} | {% if quality_metrics.high_delta > 0 %}+{% endif %}{{ quality_metrics.high_delta }} |
| 순환 복잡도 평균 | {{ quality_metrics.complexity_prev }} | {{ quality_metrics.complexity_now }} | {% if quality_metrics.complexity_delta > 0 %}+{% endif %}{{ quality_metrics.complexity_delta }} |
{% else %}
- 품질 메트릭 데이터 없음
{% endif %}

### 반복 수정 패턴
{% if recurring_patterns %}
{% for pattern in recurring_patterns %}
- **{{ pattern.type }}**: {{ pattern.description }}
  - 발생 빈도: {{ pattern.frequency }}회
  - 주요 파일: {{ pattern.files|join(', ') }}
{% endfor %}
{% else %}
- 반복 패턴 감지 안 됨
{% endif %}

---

## 3. 성능·안정성 메트릭 (Performance & Stability)

### 성능 지표
{% if performance_metrics %}
| 지표 | 이전 | 현재 | 변화율 |
|------|------|------|--------|
| 로딩 시간 | {{ performance_metrics.load_prev }}s | {{ performance_metrics.load_now }}s | {{ performance_metrics.load_delta }}% |
| 평균 FPS | {{ performance_metrics.fps_prev }} | {{ performance_metrics.fps_now }} | {{ performance_metrics.fps_delta }}% |
| 평균 RTT | {{ performance_metrics.rtt_prev }}ms | {{ performance_metrics.rtt_now }}ms | {{ performance_metrics.rtt_delta }}% |
| 메모리 사용량 | {{ performance_metrics.memory_prev }}MB | {{ performance_metrics.memory_now }}MB | {{ performance_metrics.memory_delta }}% |
{% else %}
- 성능 메트릭 데이터 없음
{% endif %}

### 안정성 지표
{% if stability_metrics %}
- **크래시 발생**: {{ stability_metrics.crashes }}회 (이전 기간: {{ stability_metrics.crashes_prev }}회)
- **빌드 성공률**: {{ stability_metrics.build_success_rate }}%
- **테스트 성공률**: {{ stability_metrics.test_success_rate }}%
- **네트워크 실패율**: {{ stability_metrics.network_fail_rate }}%
{% else %}
- 안정성 메트릭 데이터 없음
{% endif %}

### UE5 빌드·쿠킹 메트릭
{% if ue5_metrics %}
- **평균 빌드 시간**: {{ ue5_metrics.build_time }}분
- **쿠킹 성공률**: {{ ue5_metrics.cook_success_rate }}%
- **DDC Hit율**: {{ ue5_metrics.ddc_hit_rate }}%
- **셰이더 컴파일 시간**: {{ ue5_metrics.shader_compile_time }}분
{% else %}
- UE5 메트릭 데이터 없음
{% endif %}

---

## 4. 기능 개발 흐름 (Feature Momentum)

### 신규 기능 트렌드
{% if feature_trend %}
{{ feature_trend.summary }}

**주요 기능**:
{% for feature in feature_trend.major %}
- **{{ feature.title }}** ({{ feature.date }})
  - {{ feature.description }}
{% endfor %}
{% else %}
- 기능 개발 데이터 없음
{% endif %}

### 리팩터링 동향
{% if refactor_trend %}
{{ refactor_trend.summary }}

**주요 리팩터링**:
{% for refactor in refactor_trend.major %}
- **{{ refactor.title }}** ({{ refactor.date }})
  - 목적: {{ refactor.purpose }}
  - 영향: {{ refactor.impact }}
{% endfor %}
{% else %}
- 리팩터링 데이터 없음
{% endif %}

### 버그 해결 흐름
{% if bug_trend %}
{{ bug_trend.summary }}

- 총 수정: {{ bug_trend.total }}개
- 평균 해결 시간: {{ bug_trend.avg_resolution_time }}
- 재발 버그: {{ bug_trend.recurring }}개
{% else %}
- 버그 수정 데이터 없음
{% endif %}

---

## 5. 리스크 평가 (Risk Evaluation)

### 주요 리스크
{% if risks %}
| 리스크 | 심각도 | 발생 원인 | 영향 범위 | 대응 계획 |
|--------|--------|-----------|-----------|-----------|
{% for risk in risks %}
| {{ risk.title }} | {{ risk.severity }} | {{ risk.cause }} | {{ risk.scope }} | {{ risk.mitigation }} |
{% endfor %}
{% else %}
- 식별된 리스크 없음
{% endif %}

### 기술 부채 현황
{% if tech_debt %}
**상위 기술 부채**:
{% for debt in tech_debt %}
- **{{ debt.title }}** ({{ debt.priority }})
  - 설명: {{ debt.description }}
  - 예상 해결 시간: {{ debt.estimated_hours }}시간
  - 영향: {{ debt.impact }}
{% endfor %}

**기술 부채 점수**: {{ tech_debt_score }}/100
{% else %}
- 기술 부채 데이터 없음
{% endif %}

---

## 6. 다음 단계 (Next Steps)

### 즉시 조치 필요
{% if urgent_actions %}
{% for action in urgent_actions %}
1. **{{ action.title }}** ({{ action.priority }})
   - 이유: {{ action.reason }}
   - 담당: {{ action.assignee }}
   - 기한: {{ action.deadline }}
{% endfor %}
{% else %}
- 긴급 조치 사항 없음
{% endif %}

### 구조 개선 작업
{% if improvement_plans %}
{% for plan in improvement_plans %}
- **{{ plan.title }}**
  - 목표: {{ plan.goal }}
  - 예상 기간: {{ plan.duration }}
  - 우선순위: {{ plan.priority }}
{% endfor %}
{% else %}
- 계획된 개선 작업 없음
{% endif %}

### 테스트 보완
{% if test_improvements %}
{% for test in test_improvements %}
- **{{ test.area }}**: {{ test.description }}
  - 현재 커버리지: {{ test.current_coverage }}%
  - 목표 커버리지: {{ test.target_coverage }}%
{% endfor %}
{% else %}
- 테스트 보완 계획 없음
{% endif %}

---

## 7. 시스템 다이어그램 (Architecture Overview)

### 모듈 의존성 변화
```mermaid
graph TB
    subgraph "Core Systems"
        YiSan[YiSan Module]
        Latte[LatteLibrary]
        Coffee[CoffeeLibrary]
    end

    subgraph "Subsystems"
    {% for subsystem in subsystems %}
        {{ subsystem.id }}[{{ subsystem.name }}]
    {% endfor %}
    end

    YiSan --> Latte
    YiSan --> Coffee
    {% for dep in dependencies %}
    {{ dep.from }} --> {{ dep.to }}
    {% endfor %}

    {% for subsystem in changed_subsystems %}
    style {{ subsystem.id }} fill:#ffe1f5
    {% endfor %}
```

### 성능 메트릭 트렌드
```mermaid
graph LR
    subgraph "Performance Trend"
        {% if performance_metrics %}
        A[로딩: {{ performance_metrics.load_now }}s<br/>{% if performance_metrics.load_delta < 0 %}✅{% else %}⚠️{% endif %} {{ performance_metrics.load_delta }}%]
        B[FPS: {{ performance_metrics.fps_now }}<br/>{% if performance_metrics.fps_delta > 0 %}✅{% else %}⚠️{% endif %} {{ performance_metrics.fps_delta }}%]
        C[메모리: {{ performance_metrics.memory_now }}MB<br/>{% if performance_metrics.memory_delta < 0 %}✅{% else %}⚠️{% endif %} {{ performance_metrics.memory_delta }}%]
        {% endif %}
    end

    {% if performance_metrics %}
    {% if performance_metrics.load_delta < 0 %}
    style A fill:#d4edda
    {% else %}
    style A fill:#f8d7da
    {% endif %}
    {% if performance_metrics.fps_delta > 0 %}
    style B fill:#d4edda
    {% else %}
    style B fill:#f8d7da
    {% endif %}
    {% if performance_metrics.memory_delta < 0 %}
    style C fill:#d4edda
    {% else %}
    style C fill:#f8d7da
    {% endif %}
    {% endif %}
```

---

## 8. 결론 및 권장 사항 (Conclusion)

### 전반적 평가
{{ overall_assessment }}

### 강점 (Strengths)
{% if strengths %}
{% for strength in strengths %}
- {{ strength }}
{% endfor %}
{% else %}
- TBD
{% endif %}

### 개선 필요 영역 (Areas for Improvement)
{% if improvements_needed %}
{% for improvement in improvements_needed %}
- {{ improvement }}
{% endfor %}
{% else %}
- TBD
{% endif %}

### 권장 조치 (Recommendations)
{% if recommendations %}
{% for rec in recommendations %}
1. **{{ rec.title }}** ({{ rec.priority }})
   - {{ rec.description }}
{% endfor %}
{% else %}
- TBD
{% endif %}

---

**생성 시간**: {{ generation_time }}
**포함된 DevLog**: {% if weekly_logs %}{{ weekly_logs|length }}개 주간 리포트{% else %}N/A{% endif %}
**분석 기간**: {{ analysis_days }}일
