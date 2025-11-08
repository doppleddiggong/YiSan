1. 원하는 것
github의 work flow를 이용한 honkit 추가
honkit은 
\Documents\DevLog
을 이용해서 일자별로 구성

honkit의 워크 플로우는 깃허브에서 수동 테스트를 위해.
workflow_dispatch:
honkit의 기본 작동은 document폴더의 devlog, planning이 
업데이트 시 발동


2. 혼킷으로 devlog가 빠짐에 따라,
독시젠에서는 devlog 제거

3. 그 외 Document폴더의 planning의 내용도 Honkit으로 분류

4. 독시젠쪽에서는 더이상 devlog쪽 미노출

5. readme에 honkit용 url 정보를 추가

6. 이것을 테스트하면서,
깃 자동 커밋 메세지와 agentlog를 테스트

7. 주간 리포트의 추가
devlog의 7일분량의 종합버전

8. 이때 honkit은 honkit 용 agent를 추가로 생성해서 다른 프로젝트에서 이용할수 있게 세팅을 할것이다.
이것을 claude skill화 시켜서 다른 프로젝트 구동시 명령어로 입력
이것에 따른 구성도 필요함



일일 로그는

# Daily DevLog — {{date}} ({{weekday}})
- 범위: {{date_from}} ~ {{date_to}}
- 브랜치: {{branch}} / 베이스: {{base_branch}}
- 릴리즈 타겟: {{release_tag | default("N/A")}}

## 1. 오늘의 핵심 변경 (Top Changes)
- [{{commit_type}}] {{summary}} (#{{pr_number}}) — 영향: {{impact_brief}}
- …

### Commit Heatmap
- 총 커밋: {{commit_count}}, 변경 라인: +{{added}} / -{{deleted}}, 영향 파일: {{hotspot_files}}

## 2. 시스템 영향도 (Impact)
- 성능: 로딩 {{load_time_prev}}s → {{load_time_now}}s ({{delta_pct}}%)
- 안정성: 크래시 {{crash_prev}} → {{crash_now}}, 실패 빌드 {{build_fail_count}}
- 네트워크: 평균 RTT {{rtt_ms}}ms, 실패율 {{net_fail_pct}}%

## 3. 검증(Verification)
- 빌드(UE5): {{ubt_target}} 성공/실패, 쿠킹 결과: {{cook_summary}}
- 테스트: 단위/통합/에디터 테스트 {{pass}}/{{fail}} (커버리지 {{coverage}}%)
- 정적분석: 경고 {{warn_prev}} → {{warn_now}}, 신규 심각도(High): {{high_new}}

## 4. 코드 문서화 변화(Doxygen Delta)
- 신규 API: {{api_added}}, 변경: {{api_changed}}, 폐기: {{api_removed}}
- 주목 API: {{notable_api_list}}

## 5. 리팩토링·위험 이슈
- 리팩토링: {{refactor_items}}
- 위험: {{risk_items}} (대응: {{mitigation}})

## 6. 내일(Next)·미진(Action)
- Next: {{next_items}}
- 미진: {{pending_items}}

## 7. Mermaid 개요도(자동 생성)
```mermaid
flowchart LR
  Dev[Commits/PRs] --> Build[UE5 Build/Cook]
  Build --> Test[Unit/Editor Tests]
  Test --> Doc[Doxygen Update]
  Doc --> Daily[Daily DevLog]






**핵심 포인트**  
- “핵심 변경 → 영향 → 검증 → 위험 → Next” 순으로 **의미**를 먼저 보여줍니다.  
- 모든 수치는 **전일 대비**로 표기하여 추세를 바로 파악합니다.  
- Mermaid 블록은 파이프라인과 변화 요지를 매일 렌더링합니다.

---

# 2) 자동 수집 항목(필수·권장)

- **Git/PR**: Conventional Commits 분류, PR 링크, 변경 라인 수, Hotspot 파일  
- **UE5 빌드·쿠킹 요약**: `UnrealBuildTool` 로그에서 Target 성공/실패, `Cook` 결과 요약(에러·스킵·패키지 크기)  
- **테스트**: 단위/에디터 테스트 결과(JUnit XML 등)  
- **정적분석**: `clang-tidy`/`cppcheck`/`MSVC` 경고 증감, 신규 High 심각도  
- **성능 계측**: 로딩 시간, 패킷 RTT, 프레임 타임의 중앙값/상위 백분위  
- **Doxygen 변화**: added/changed/removed 심볼 카운트, 주목 API 목록  
- **Next/미진**: 규칙화된 TODO 태그(`TODO(dopple): …`) 스캔

---

# 3) GitHub Actions 예시(yml)

`.github/workflows/daily-devlog.yml`

```yaml
name: Daily DevLog

on:
  schedule:
    - cron: '0 2 * * *'  # KST 11:00 기준 조정 권장
  workflow_dispatch:

jobs:
  daily:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.11'

      - name: Install deps
        run: |
          pip install jinja2 pyyaml lxml

      - name: Build UE (optional, summary only)
        if: ${{ false }} # CI 시간 이슈 시 비활성화, 내부 빌드 아티팩트 레포에서 메트릭만 가져와도 됨
        run: |
          echo "Run UBT or fetch last build summary"

      - name: Run Doxygen (delta only)
        run: |
          doxygen Doxyfile || true
          # delta 계산 스크립트에서 html/xml/json을 요약치로만 사용

      - name: Generate Daily DevLog
        run: |
          python Tools/devlog/generate_daily.py \
            --since "24 hours" \
            --branch "$(git rev-parse --abbrev-ref HEAD)" \
            --base "origin/main" \
            --out "docs/Daily/$(date +%F).md" \
            --doxygen "Docs/Doxygen/xml" \
            --ubt_log "Artifacts/Build/last_build.log" \
            --cook_log "Artifacts/Build/last_cook.log" \
            --test_xml "Artifacts/Test/results.xml" \
            --static_report "Artifacts/Static/summary.json" \
            --metrics "Artifacts/Metrics/daily.json"

      - name: Update SUMMARY.md for HonKit
        run: |
          python Tools/devlog/update_summary.py docs > docs/SUMMARY.md

      - name: Commit DevLog
        run: |
          git config user.name "AutoDevLog"
          git config user.email "bot@example.com"
          git add docs/Daily/*.md docs/SUMMARY.md
          git commit -m "chore(devlog): daily $(date +%F)" || echo "No changes"
          git push


#!/usr/bin/env python3
import argparse, subprocess, json, re, datetime, statistics, os
from pathlib import Path
from jinja2 import Template

TEMPLATE = Path(__file__).with_name("daily_template.md").read_text(encoding="utf-8")

def sh(cmd):
    return subprocess.check_output(cmd, shell=True, text=True, errors="ignore").strip()

def git_range_since(since):
    return sh(f'git log --since="{since}" --pretty=format:"%H"').splitlines()

def git_stats(commits):
    added=deleted=0
    details=[]
    for h in commits:
        s=sh(f'git show --numstat --format="%s||%h||%an" {h}')
        lines=s.splitlines()
        meta=lines[0]
        for ln in lines[1:]:
            m=re.match(r"(\d+|-)\s+(\d+|-)\s+(.+)", ln)
            if not m: continue
            a,d,f=m.groups()
            if a!='-' and d!='-':
                added+=int(a); deleted+=int(d)
        subject, short, author = meta.split("||")
        details.append({"hash":short, "subject":subject, "author":author})
    return {"added":added,"deleted":deleted,"details":details,"count":len(commits)}

def parse_ubt_summary(path):
    d={"success":None,"target":"","errors":0}
    p=Path(path)
    if not p.exists(): return d
    txt=p.read_text(errors="ignore",encoding="utf-8",newline=None)
    d["success"]=("Error" not in txt and "BUILD FAILED" not in txt)
    m=re.search(r"Building\s+(.+?)\s+-\s+(.+)", txt)
    if m: d["target"]=m.group(1).strip()
    d["errors"]=len(re.findall(r": error:", txt))
    return d

def parse_cook_summary(path):
    d={"cooked":0,"skipped":0,"errors":0}
    p=Path(path)
    if not p.exists(): return d
    txt=p.read_text(errors="ignore",encoding="utf-8")
    m=re.search(r"Cooked\s+(\d+)", txt); d["cooked"]=int(m.group(1)) if m else 0
    m=re.search(r"Skipped\s+(\d+)", txt); d["skipped"]=int(m.group(1)) if m else 0
    d["errors"]=len(re.findall(r"\bError\b", txt))
    return d

def parse_tests(junit_xml):
    p=Path(junit_xml)
    if not p.exists(): return {"pass":0,"fail":0,"coverage":None}
    from lxml import etree
    root=etree.parse(str(p))
    t_pass=int(root.xpath("sum(//testsuite/@tests)")) - int(root.xpath("sum(//testsuite/@failures)")) - int(root.xpath("sum(//testsuite/@errors)"))
    t_fail=int(root.xpath("sum(//testsuite/@failures)")) + int(root.xpath("sum(//testsuite/@errors)"))
    cov=None
    covnode=root.xpath("//properties/property[@name='coverage']/@value")
    if covnode: cov=float(covnode[0])
    return {"pass":t_pass,"fail":t_fail,"coverage":cov}

def parse_static(report_json):
    p=Path(report_json)
    if not p.exists(): return {"warn_now":0,"high_new":0}
    d=json.loads(p.read_text(encoding="utf-8"))
    return {"warn_now":d.get("warnings",0),"high_new":d.get("high_new",0)}

def load_metrics(path):
    p=Path(path)
    return json.loads(p.read_text(encoding="utf-8")) if p.exists() else {}

def render(context, out_path):
    md = Template(TEMPLATE).render(**context)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(md, encoding="utf-8")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--since", default="24 hours")
    ap.add_argument("--branch", required=True)
    ap.add_argument("--base", default="origin/main")
    ap.add_argument("--out", required=True)
    ap.add_argument("--doxygen", default="Docs/Doxygen/xml")
    ap.add_argument("--ubt_log", default="")
    ap.add_argument("--cook_log", default="")
    ap.add_argument("--test_xml", default="")
    ap.add_argument("--static_report", default="")
    ap.add_argument("--metrics", default="")
    args=ap.parse_args()

    commits = git_range_since(args.since)
    gstats  = git_stats(commits)
    ubt     = parse_ubt_summary(args.ubt_log)
    cook    = parse_cook_summary(args.cook_log)
    tests   = parse_tests(args.test_xml)
    srep    = parse_static(args.static_report)
    met     = load_metrics(args.metrics)

    today = datetime.date.today()
    ctx = {
        "date": today.isoformat(),
        "weekday": ["월","화","수","목","금","토","일"][today.weekday()],
        "date_from": (today - datetime.timedelta(days=1)).isoformat(),
        "date_to": today.isoformat(),
        "branch": args.branch,
        "base_branch": args.base,
        "commit_count": gstats["count"],
        "added": gstats["added"],
        "deleted": gstats["deleted"],
        "hotspot_files": met.get("hotspot_files", []),
        "impact_brief": met.get("impact_brief",""),
        "commit_type": met.get("top_type","feat/refactor/fix"),
        "pr_number": met.get("top_pr",""),
        "ubt_target": ubt["target"],
        "cook_summary": f'cooked {cook["cooked"]}, skipped {cook["skipped"]}, errors {cook["errors"]}',
        "pass": tests["pass"], "fail": tests["fail"], "coverage": tests["coverage"],
        "warn_now": srep["warn_now"], "high_new": srep["high_new"],
        "load_time_prev": met.get("load_time_prev"),
        "load_time_now": met.get("load_time_now"),
        "delta_pct": met.get("delta_pct"),
        "crash_prev": met.get("crash_prev"), "crash_now": met.get("crash_now"),
        "rtt_ms": met.get("rtt_ms"), "net_fail_pct": met.get("net_fail_pct"),
        "api_added": met.get("api_added",0),"api_changed":met.get("api_changed",0),"api_removed":met.get("api_removed",0),
        "notable_api_list": met.get("notable_api_list", []),
        "refactor_items": met.get("refactor_items", []),
        "risk_items": met.get("risk_items", []),
        "mitigation": met.get("mitigation", ""),
        "next_items": met.get("next_items", []),
        "pending_items": met.get("pending_items", []),
    }
    render(ctx, args.out)

if __name__ == "__main__":
    main()



참조 필요.


확장


주간/월간 자동 합산기: Daily/*.md에서 정규 패턴을 읽어 핵심 변화와 지표만 집계합니다.


Conventional Commits → 영향 추론 규칙:


feat: 는 기능/위험도 상승 가능성, refactor: 는 성능/안정화 연동, fix: 는 결함 추세 감소로 가중치 부여.




UE5 특정 지표: CookedAssets, ShaderCompile 시간, DDC Hit율을 전일 대비로 표시.


Mermaid 자동 생성 고도화:


전일 대비 변경된 서브시스템만 색상 표시(머메이드 주석 활용), 노드 수 최소화.




품질 루브릭:


정확성: 전일 대비 값이 모두 숫자로 환산되는가.


명료성: Top Changes 3개 이내, 영향·다음 액션 한 줄 요약.


일관성: 헤더·키·단위 표기 통일.


실행가능성: 실패 빌드/테스트에 즉시 액션 항목 자동 생성.




원하면 이 구성에 맞춰 주간·월간 집계 스크립트와 HonKit 사이드바 자동화까지 패키지로 붙여 드리겠습니다.

daily_log로 작성된 코드는 discord_webhook을 이용해 리포트 필요.

이때 리포트는 feedback이 필요한 상태.

10. 특정 큰 단위의 시스템이 생성시에는 시스템 관련된 문서가 honkit에 문서화 해서 들어가야하므로,
관련 로직이 필요함
지금은 yisanloading, network_online_subsystem, dasan npc 관련된 내용,
RPC를 사용한 태스크 전반에 대한 시스템 리뷰 문서가 포함이 되어야함.


1) “시스템 리뷰 문서”의 정의(자동 생성 기준)

일일 로그가 “사실 기록”이라면, 시스템 리뷰는 구조 변화·건전성·위험도·성능·지식 변화를 요약하는 상위 레이어입니다.

✅ 시스템 리뷰 문서가 답해야 하는 질문

무엇이 바뀌었는가?

Subsystem / Module / API / Pipeline 변화

Doxygen Delta 기반 구조적 변경 요약

왜 바뀌었는가?

커밋 타입(feat/refactor/fix)와 연동

PR 설명 요약(LLM 활용 가능)

변화의 영향은 무엇인가?

성능(로딩·프레임·네트워크)

안정성(테스트·크래시·정적분석)

기술부채 변화(Hotspot 파일)

리스크는 어디에 있는가?

갑자기 증가한 파일 변경량

반복되는 리팩터링·버그

신규 API 증가 패턴

다음 단계는 무엇인가?

리스크 대응

구조 개선 계획

테스트 보완

2) 시스템 리뷰 문서 템플릿(주간/월간 자동 생성)

docs/SystemReview/YYYY-MM.md

# System Review – {{period}}

## 1. 아키텍처 변화 요약 (Structural Delta)
- 신규 서브시스템: {{new_subsystems}}
- 삭제/흡수된 모듈: {{removed_modules}}
- 주요 API 변경: {{api_changed}}
- 영향 범위: {{affected_areas}}
- 참고: Doxygen Delta Summary 첨부

## 2. 코드베이스 건강도 (Code Health)
- Hotspot 파일 증가/감소: {{hotspots}}
- 경고 변화(MSVC/clang-tidy): {{warnings_delta}}
- 복잡도(Complexity) 변화: {{complexity_delta}}
- 반복된 수정 패턴: {{recurring_patterns}}

## 3. 성능·안정성 메트릭 (Performance & Stability)
- 로딩 시간: {{load_prev}} → {{load_now}} ({{delta_load}}%)  
- 프레임: {{fps_prev}} → {{fps_now}}  
- 네트워크: RTT {{rtt}}ms / 실패율 {{fail_pct}}%  
- UE 빌드·쿠킹 성공률: {{build_success}}%

## 4. 기능 개발 흐름 (Feature Momentum)
- 신규 기능 흐름: {{feat_summary}}
- 리팩터링 동향: {{refactor_summary}}
- 버그 해결 흐름: {{fix_summary}}

## 5. 리스크 평가 (Risk Evaluation)
- 주요 리스크: {{risk_items}}
- 발생 원인: {{risk_cause}}
- 대응 계획: {{risk_mitigation}}

## 6. 다음 단계 (Next Steps)
- 즉시 조치 필요: {{urgent}}
- 구조 개선 작업: {{refactor_plan}}
- 테스트 보완: {{test_improve}}

## 7. Mermaid 구조도 (자동 생성)
```mermaid
flowchart TB
  A[Subsystems] --> B[Modified APIs]
  B --> C[Performance Impact]
  C --> D[Stability]
  D --> E[Next Steps]


---

# 3) 리뷰 문서를 자동 생성하기 위한 데이터 수집 지점

| 항목 | 자동 수집 방법 |
|------|----------------|
| 아키텍처 변화 | Doxygen JSON diff, 파일 트리 diff |
| 복잡도 · Hotspot | git diff + Lizard(Cyclomatic) |
| 성능·네트워크 | 게임 내부 Metric 로그, CSV 요약 |
| Testrun 결과 | JUnit XML |
| 정적분석 | clang-tidy / cppcheck 요약 |
| 리스크 | 반복 수정 영역 자동 탐지 |
| 기능 흐름 | Conventional Commit 분류(feat/refactor/fix) |

---

# 4) GitHub Actions 단계: “System Review Aggregator”

주간/월간 스케줄:

```yaml
on:
  schedule:
    - cron: '0 3 * * 1'   # Weekly
    - cron: '0 4 1 * *'   # Monthly


핵심 단계:

Daily/*.md 수집

Doxygen Delta 분석

Lizard로 복잡도 분석

Hotspot 파일 자동 추출

성능 로그 집계

Jinja 템플릿으로 Review 문서 생성

HonKit SUMMARY 자동 업데이트

5) 기술적으로 가능한 “더 고급 단계”
✅ (A) 코드 기반 아키텍처 자동 시각화

Doxygen JSON → Mermaid Class Diagram 변환

Module 간 의존성 그래프 자동 생성

변경된 부분에 색 강조

✅ (B) 위험도 자동 판단

Hotspot + Cyclomatic + 커밋 빈도 → 위험 점수 산출

“위험 상위 5개 파일” 자동 표기

✅ (C) 성능·네트워크 변화 자동 관찰

UE 로그에서 “LoadMap”, “CookStats” 추출

RTPC/Audio TTS 지연까지 포함 가능

6) 결과적으로 생성되는 문서의 가치

이 시스템 리뷰 문서는 개발자가 직접 쓰는 문서가 아니라:

현재 코드베이스가 건강한가?

어디가 위험한가?

최근 구조는 어떻게 변했는가?

이번 스프린트의 기술적 성과는 무엇인가?

를 “시스템 상태 리포트”로 보여주게 됩니다.

이 구조는 게임회사에서도 플랫폼팀이나 엔진팀이 사용하는 수준이며,
포트폴리오에도 매우 강한 인상이 남습니다.


일일 리포트는 말그대로 일일 리포트
주간 리포트는 feedback이 추가가 되는 형태
이때 agent가 필요하다면 gpt completion을 통해 질의응답용 을 남겨서
팀원이 스스로 답을 하면서 성장을 할수 있게 해줘야함.



7) 문서 구성 가이드 (자동화 최적화)
✅ 1) 문서의 첫 화면은 반드시 Mermaid

시각 정보가 먼저 와야 읽히는 문서가 된다.

✅ 2) 텍스트는 3~4줄 요약만

나머지는 접을 수 있는 Collapsible 블록으로 넣는다.
(HonKit은 <details> 지원)

✅ 3) GIF/Png 자동 삽입

Blueprint 흐름

네트워크 시퀀스

캐릭터 스킬 처리 등

UE Editor Utility Script로 자동 캡처 가능.

✅ 4) “변화(Delta)”를 강조

이번 주 변경점이 눈에 보이도록 한다.

전반적인 내용이 많은 상태. 



