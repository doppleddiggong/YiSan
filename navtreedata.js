/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "YiSan Project Documentation", "index.html", [
    [ "Project YiSan (이산) - 개발 문서", "index.html", "index" ],
    [ "Project YiSan 에이전트 가이드 (Project YiSan Agent Guide)", "md_GEMINI.html", [
      [ "AI 에이전트 페르소나 (Persona)", "md_GEMINI.html#autotoc_md79", null ],
      [ "기본 운영 원칙 (Core Rules)", "md_GEMINI.html#autotoc_md81", null ],
      [ "문서화 모드 (Documentation Mode)", "md_GEMINI.html#autotoc_md83", null ],
      [ "관찰 로그/텔레메트리 정책 (Observability & Logging)", "md_GEMINI.html#autotoc_md85", null ],
      [ "커밋 메시지 및 코딩 규칙", "md_GEMINI.html#autotoc_md87", null ],
      [ "디버그 에이전트 워크플로우 (Debug Agent Workflow)", "md_GEMINI.html#autotoc_md89", null ],
      [ "DevLog 에이전트 워크플로우 (DevLog Agent Workflow)", "md_GEMINI.html#autotoc_md91", null ],
      [ "Project YiSan 개요", "md_GEMINI.html#autotoc_md93", [
        [ "1) 프로젝트 정보", "md_GEMINI.html#autotoc_md94", null ],
        [ "2) 개발 환경 / 도구", "md_GEMINI.html#autotoc_md95", null ],
        [ "3) 사용 언어 / 미들웨어 / API", "md_GEMINI.html#autotoc_md96", null ],
        [ "4) 모듈 구조", "md_GEMINI.html#autotoc_md97", [
          [ "4-1) 게임 모듈: <tt>YiSan</tt> (Runtime)", "md_GEMINI.html#autotoc_md98", null ],
          [ "4-2) 서브 모듈: <tt>CoffeeLibrary</tt> (Runtime)", "md_GEMINI.html#autotoc_md99", null ],
          [ "4-3) 서브 모듈: <tt>LatteLibrary</tt> (Runtime)", "md_GEMINI.html#autotoc_md100", null ],
          [ "4-4) 에디터 플러그인: <tt>CoffeeToolbar</tt> (Editor, Win64)", "md_GEMINI.html#autotoc_md101", null ]
        ] ],
        [ "5) 핵심 시스템 (게임 모듈 및 서브 모듈 기준)", "md_GEMINI.html#autotoc_md102", null ],
        [ "6) 빌드/타깃 설정", "md_GEMINI.html#autotoc_md103", null ],
        [ "7) 입력 / 맵 / 게임모드 기본값", "md_GEMINI.html#autotoc_md104", null ],
        [ "8) 아트/에셋 파이프라인", "md_GEMINI.html#autotoc_md105", null ],
        [ "9) 빌드 / 실행", "md_GEMINI.html#autotoc_md106", null ]
      ] ],
      [ "Agent QA 로그 정책", "md_GEMINI.html#autotoc_md108", null ]
    ] ],
    [ "AI 에이전트와 효과적으로 상호작용하는 가이드", "md_AgentRule_2agent__interaction__guide.html", [
      [ "1. 개요", "md_AgentRule_2agent__interaction__guide.html#autotoc_md2", null ],
      [ "2. 핵심 원칙", "md_AgentRule_2agent__interaction__guide.html#autotoc_md3", null ],
      [ "3. 효율적인 요청 방법", "md_AgentRule_2agent__interaction__guide.html#autotoc_md4", [
        [ "3.1. 작업 목표 명확화", "md_AgentRule_2agent__interaction__guide.html#autotoc_md5", null ],
        [ "3.2. 관련 파일/경로 지정", "md_AgentRule_2agent__interaction__guide.html#autotoc_md6", null ],
        [ "3.3. 기존 코드/컨벤션 참조", "md_AgentRule_2agent__interaction__guide.html#autotoc_md8", null ],
        [ "3.4. 예상 결과 제시", "md_AgentRule_2agent__interaction__guide.html#autotoc_md10", null ],
        [ "3.5. 문제 발생 시 상세 정보 제공", "md_AgentRule_2agent__interaction__guide.html#autotoc_md12", null ]
      ] ],
      [ "4. 에이전트의 질문에 대한 응답", "md_AgentRule_2agent__interaction__guide.html#autotoc_md16", null ],
      [ "5. 커밋 메시지 작성 요청 시", "md_AgentRule_2agent__interaction__guide.html#autotoc_md18", null ]
    ] ],
    [ "Commit Message Agent", "md_AgentRule_2commit__agent.html", [
      [ "1. 역할 (Persona)", "md_AgentRule_2commit__agent.html#autotoc_md7", null ],
      [ "2. 핵심 규칙 (Core Rules)", "md_AgentRule_2commit__agent.html#autotoc_md9", null ],
      [ "3. 스코프 결정 규칙", "md_AgentRule_2commit__agent.html#autotoc_md11", null ],
      [ "4. 입력 포맷", "md_AgentRule_2commit__agent.html#autotoc_md13", null ],
      [ "5. 출력 포맷", "md_AgentRule_2commit__agent.html#autotoc_md14", null ],
      [ "6. 동작", "md_AgentRule_2commit__agent.html#autotoc_md15", null ],
      [ "7. 참고", "md_AgentRule_2commit__agent.html#autotoc_md17", null ],
      [ "8. 알려진 문제 및 해결책", "md_AgentRule_2commit__agent.html#autotoc_md197", [
        [ "8.1. 문제: <tt>git commit -m</tt> 사용 시 한글 본문 누락", "md_AgentRule_2commit__agent.html#autotoc_md198", null ]
      ] ],
      [ "9. 커밋 메시지 생성 방식 개선 (Commit Message Generation Improvement)", "md_AgentRule_2commit__agent.html#autotoc_md199", null ]
    ] ],
    [ "Unreal Engine C++ 코딩 컨벤션 가이드 (Project YiSan)", "md_AgentRule_2conventions__agent.html", [
      [ "1. 네이밍 규칙 (Naming Conventions)", "md_AgentRule_2conventions__agent.html#autotoc_md22", [
        [ "1.1. 클래스 및 파일명 (Classes and Filenames)", "md_AgentRule_2conventions__agent.html#autotoc_md23", null ],
        [ "1.2. 변수 (Variables)", "md_AgentRule_2conventions__agent.html#autotoc_md24", null ],
        [ "1.3. 함수 (Functions)", "md_AgentRule_2conventions__agent.html#autotoc_md25", null ]
      ] ],
      [ "2. UPROPERTY / UFUNCTION 매크로", "md_AgentRule_2conventions__agent.html#autotoc_md27", null ],
      [ "3. 파일 구조 및 주석", "md_AgentRule_2conventions__agent.html#autotoc_md32", [
        [ "3.1. 헤더 파일", "md_AgentRule_2conventions__agent.html#autotoc_md34", null ],
        [ "3.2. 주석 (Comments)", "md_AgentRule_2conventions__agent.html#autotoc_md36", null ]
      ] ],
      [ "4. 기타 규칙", "md_AgentRule_2conventions__agent.html#autotoc_md39", [
        [ "4.1. 로깅 (Logging)", "md_AgentRule_2conventions__agent.html#autotoc_md40", null ],
        [ "4.2. 기타", "md_AgentRule_2conventions__agent.html#autotoc_md42", null ]
      ] ],
      [ "5. 네트워크 모듈 컨벤션 (Network Module Conventions)", "md_AgentRule_2conventions__agent.html#autotoc_md47", null ],
      [ "6. Doxygen 주석 스타일 가이드 (Doxygen Comment Style Guide)", "md_AgentRule_2conventions__agent.html#autotoc_md50", [
        [ "6.1. 주석 형식", "md_AgentRule_2conventions__agent.html#autotoc_md51", null ],
        [ "6.2. 필수 태그 및 예시", "md_AgentRule_2conventions__agent.html#autotoc_md52", null ],
        [ "6.3. 코드 예시", "md_AgentRule_2conventions__agent.html#autotoc_md53", null ]
      ] ]
    ] ],
    [ "Debug Agent Guide", "md_AgentRule_2debug__guide.html", [
      [ "1. 목적 (Purpose)", "md_AgentRule_2debug__guide.html#autotoc_md28", null ],
      [ "2. 디버깅 전략 (Debugging Strategy)", "md_AgentRule_2debug__guide.html#autotoc_md29", [
        [ "2.1. 문제 이해 (Understanding the Problem)", "md_AgentRule_2debug__guide.html#autotoc_md30", null ],
        [ "2.2. 디버그 포인트 식별 (Identifying Debug Points)", "md_AgentRule_2debug__guide.html#autotoc_md33", null ],
        [ "2.3. 디버그 코드 삽입 (Inserting Debug Code)", "md_AgentRule_2debug__guide.html#autotoc_md35", null ],
        [ "2.4. 로그 분석 (Analyzing Logs)", "md_AgentRule_2debug__guide.html#autotoc_md37", null ],
        [ "2.5. 수정 방안 제안 (Proposing Solutions)", "md_AgentRule_2debug__guide.html#autotoc_md41", null ],
        [ "2.6. 디버그 코드 정리 (Cleaning Up Debug Code)", "md_AgentRule_2debug__guide.html#autotoc_md43", null ]
      ] ],
      [ "3. 반복적인 문제 처리 (Handling Recurring Issues)", "md_AgentRule_2debug__guide.html#autotoc_md44", null ],
      [ "4. 로그 파일 자동 분석 (규칙 추가)", "md_AgentRule_2debug__guide.html#autotoc_md48", null ]
    ] ],
    [ "DevLog Agent", "md_AgentRule_2devlog__agent.html", [
      [ "1. 역할 (Persona)", "md_AgentRule_2devlog__agent.html#autotoc_md56", null ],
      [ "2. 핵심 규칙 (Core Rules)", "md_AgentRule_2devlog__agent.html#autotoc_md57", null ],
      [ "3. 동작 조건 및 범위", "md_AgentRule_2devlog__agent.html#autotoc_md58", null ],
      [ "4. 일일 업무 일지 (Daily DevLog) 형식", "md_AgentRule_2devlog__agent.html#autotoc_md59", [
        [ "Daily DevLog — {{YYYY-MM-DD}} (KST 09:00 경계)", "md_AgentRule_2devlog__agent.html#autotoc_md60", null ],
        [ "요약", "md_AgentRule_2devlog__agent.html#autotoc_md61", null ],
        [ "하이라이트", "md_AgentRule_2devlog__agent.html#autotoc_md62", null ],
        [ "커밋 기반 작업 로그", "md_AgentRule_2devlog__agent.html#autotoc_md63", [
          [ "완료", "md_AgentRule_2devlog__agent.html#autotoc_md64", null ],
          [ "진행", "md_AgentRule_2devlog__agent.html#autotoc_md65", null ],
          [ "미흡(주의/후속)", "md_AgentRule_2devlog__agent.html#autotoc_md66", null ],
          [ "TODO(커밋 본문 추출)", "md_AgentRule_2devlog__agent.html#autotoc_md67", null ]
        ] ],
        [ "오늘 계획", "md_AgentRule_2devlog__agent.html#autotoc_md68", null ],
        [ "메트릭(선택)", "md_AgentRule_2devlog__agent.html#autotoc_md69", null ]
      ] ],
      [ "5. 30일 요약 보고서 (30-Day Briefing) 형식", "md_AgentRule_2devlog__agent.html#autotoc_md70", [
        [ "30-Day Briefing — {{YYYY-MM-DD}}", "md_AgentRule_2devlog__agent.html#autotoc_md71", null ],
        [ "속도(Velocity)", "md_AgentRule_2devlog__agent.html#autotoc_md72", null ],
        [ "핫스팟(Top Scopes)", "md_AgentRule_2devlog__agent.html#autotoc_md73", null ],
        [ "미해결 TODO Top N", "md_AgentRule_2devlog__agent.html#autotoc_md74", null ],
        [ "리스크/차단요인", "md_AgentRule_2devlog__agent.html#autotoc_md75", null ],
        [ "다음 우선순위(가이드)", "md_AgentRule_2devlog__agent.html#autotoc_md76", null ]
      ] ],
      [ "6. 참고 사항", "md_AgentRule_2devlog__agent.html#autotoc_md77", null ]
    ] ],
    [ "Doxygen 문서화 및 GitHub Pages 배포 설정 가이드 (Project YiSan)", "md_AgentRule_2doxygen__setup__guide.html", [
      [ "1. 개요", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md118", null ],
      [ "2. 전제 조건", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md119", null ],
      [ "3. Doxyfile 생성 및 설정", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md120", null ],
      [ "4. GitHub Actions 워크플로우 (<tt>doxygen.yml</tt>) 설정", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md122", null ],
      [ "5. GitHub 저장소 설정", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md123", [
        [ "5.1. GitHub Actions 권한 설정", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md124", null ],
        [ "5.2. GitHub Pages 소스 설정", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md125", null ]
      ] ],
      [ "6. 배포 확인 및 문제 해결 (Troubleshooting)", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md126", [
        [ "6.1. 문제 1: <tt>wget</tt> 404 Not Found 오류", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md127", null ],
        [ "6.2. 문제 2: \"Deploy to GitHub Pages\" 단계가 실행되지 않음", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md128", null ],
        [ "6.3. 문제 3: <tt>Permission denied</tt> 오류 (403 Forbidden)", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md131", null ],
        [ "6.4. 문제 4: GitHub Pages 404 오류 (사이트 주소는 맞는데 내용이 안 보임)", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md133", null ]
      ] ],
      [ "7. 결론", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md138", null ],
      [ "8. Doxygen 주석 가이드라인 (Doxygen Commenting Guidelines)", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md141", [
        [ "8.1. 일반 원칙", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md144", null ],
        [ "8.2. 주석 스타일 및 위치", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md147", null ],
        [ "8.3. 주요 Doxygen 명령어", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md152", null ],
        [ "8.4. 예시", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md153", [
          [ "클래스 주석 예시", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md154", null ],
          [ "함수 주석 예시", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md155", null ],
          [ "멤버 변수 주석 예시", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md156", null ]
        ] ],
        [ "8.5. 파일 분할 고려사항", "md_AgentRule_2doxygen__setup__guide.html#autotoc_md157", null ]
      ] ]
    ] ],
    [ "Daily DevLog 2025-09-30 (KST 09:00 boundary)", "md_Documents_2DevLog_22025-09-30.html", [
      [ "일일 개발 로그 2025-09-30 (KST 09:00 경계)", "md_Documents_2DevLog_22025-09-30.html#autotoc_md129", [
        [ "Summary / 요약", "md_Documents_2DevLog_22025-09-30.html#autotoc_md130", null ],
        [ "Highlights / 하이라이트", "md_Documents_2DevLog_22025-09-30.html#autotoc_md132", null ],
        [ "Commit-based Work Log", "md_Documents_2DevLog_22025-09-30.html#autotoc_md134", null ],
        [ "커밋 기반 작업 로그", "md_Documents_2DevLog_22025-09-30.html#autotoc_md135", [
          [ "Done", "md_Documents_2DevLog_22025-09-30.html#autotoc_md136", null ],
          [ "완료", "md_Documents_2DevLog_22025-09-30.html#autotoc_md137", null ],
          [ "In Progress", "md_Documents_2DevLog_22025-09-30.html#autotoc_md139", null ],
          [ "진행 중", "md_Documents_2DevLog_22025-09-30.html#autotoc_md140", null ],
          [ "Needs Attention", "md_Documents_2DevLog_22025-09-30.html#autotoc_md142", null ],
          [ "주의 필요", "md_Documents_2DevLog_22025-09-30.html#autotoc_md143", null ],
          [ "TODO (from commits)", "md_Documents_2DevLog_22025-09-30.html#autotoc_md145", null ],
          [ "커밋 기반 TODO", "md_Documents_2DevLog_22025-09-30.html#autotoc_md146", null ]
        ] ],
        [ "Commit Details", "md_Documents_2DevLog_22025-09-30.html#autotoc_md148", null ],
        [ "커밋 상세", "md_Documents_2DevLog_22025-09-30.html#autotoc_md149", null ],
        [ "Metrics (approx)", "md_Documents_2DevLog_22025-09-30.html#autotoc_md150", null ],
        [ "메트릭(추정)", "md_Documents_2DevLog_22025-09-30.html#autotoc_md151", null ]
      ] ]
    ] ],
    [ "Daily DevLog 2025-10-01 (KST 09:00 boundary)", "md_Documents_2DevLog_22025-10-01.html", [
      [ "일일 개발 로그 2025-10-01 (KST 09:00 경계)", "md_Documents_2DevLog_22025-10-01.html#autotoc_md159", [
        [ "Summary / 요약", "md_Documents_2DevLog_22025-10-01.html#autotoc_md160", null ],
        [ "Highlights / 하이라이트", "md_Documents_2DevLog_22025-10-01.html#autotoc_md161", null ],
        [ "Commit-based Work Log", "md_Documents_2DevLog_22025-10-01.html#autotoc_md162", null ],
        [ "커밋 기반 작업 로그", "md_Documents_2DevLog_22025-10-01.html#autotoc_md163", [
          [ "Done", "md_Documents_2DevLog_22025-10-01.html#autotoc_md164", null ],
          [ "완료", "md_Documents_2DevLog_22025-10-01.html#autotoc_md165", null ],
          [ "In Progress", "md_Documents_2DevLog_22025-10-01.html#autotoc_md166", null ],
          [ "진행", "md_Documents_2DevLog_22025-10-01.html#autotoc_md167", null ],
          [ "Needs Attention", "md_Documents_2DevLog_22025-10-01.html#autotoc_md168", null ],
          [ "주의 필요", "md_Documents_2DevLog_22025-10-01.html#autotoc_md169", null ],
          [ "TODO (from commits)", "md_Documents_2DevLog_22025-10-01.html#autotoc_md170", null ],
          [ "커밋 기반 TODO", "md_Documents_2DevLog_22025-10-01.html#autotoc_md171", null ]
        ] ],
        [ "Commit Details", "md_Documents_2DevLog_22025-10-01.html#autotoc_md172", null ],
        [ "커밋 상세", "md_Documents_2DevLog_22025-10-01.html#autotoc_md173", null ],
        [ "Metrics (approx)", "md_Documents_2DevLog_22025-10-01.html#autotoc_md174", null ],
        [ "메트릭(추정)", "md_Documents_2DevLog_22025-10-01.html#autotoc_md175", null ]
      ] ]
    ] ],
    [ "Daily DevLog 2025-10-02 (KST 09:00 boundary)", "md_Documents_2DevLog_22025-10-02.html", [
      [ "일일 개발 로그 2025-10-02 (KST 09:00 경계)", "md_Documents_2DevLog_22025-10-02.html#autotoc_md177", [
        [ "Summary / 요약", "md_Documents_2DevLog_22025-10-02.html#autotoc_md178", null ],
        [ "Highlights / 하이라이트", "md_Documents_2DevLog_22025-10-02.html#autotoc_md179", null ],
        [ "Commit-based Work Log", "md_Documents_2DevLog_22025-10-02.html#autotoc_md180", null ],
        [ "커밋 기반 작업 로그", "md_Documents_2DevLog_22025-10-02.html#autotoc_md181", [
          [ "Done", "md_Documents_2DevLog_22025-10-02.html#autotoc_md182", null ],
          [ "완료", "md_Documents_2DevLog_22025-10-02.html#autotoc_md183", null ],
          [ "In Progress", "md_Documents_2DevLog_22025-10-02.html#autotoc_md184", null ],
          [ "진행", "md_Documents_2DevLog_22025-10-02.html#autotoc_md185", null ],
          [ "Needs Attention", "md_Documents_2DevLog_22025-10-02.html#autotoc_md186", null ],
          [ "주의 필요", "md_Documents_2DevLog_22025-10-02.html#autotoc_md187", null ],
          [ "TODO (from commits)", "md_Documents_2DevLog_22025-10-02.html#autotoc_md188", null ],
          [ "커밋 기반 TODO", "md_Documents_2DevLog_22025-10-02.html#autotoc_md189", null ]
        ] ],
        [ "Commit Details", "md_Documents_2DevLog_22025-10-02.html#autotoc_md190", null ],
        [ "커밋 상세", "md_Documents_2DevLog_22025-10-02.html#autotoc_md191", null ],
        [ "Metrics (approx)", "md_Documents_2DevLog_22025-10-02.html#autotoc_md192", null ],
        [ "메트릭(추정)", "md_Documents_2DevLog_22025-10-02.html#autotoc_md193", null ]
      ] ]
    ] ],
    [ "30-Day Briefing 2025-10-02", "md_Documents_2DevLog_2__Last30Summary.html", null ],
    [ "Project YiSan – Doxygen 워크플로", "md_Documents_2Reference_2DoxygenWorkflow.html", [
      [ "목적", "md_Documents_2Reference_2DoxygenWorkflow.html#autotoc_md201", null ],
      [ "산출물 경로", "md_Documents_2Reference_2DoxygenWorkflow.html#autotoc_md202", null ],
      [ "실행 절차", "md_Documents_2Reference_2DoxygenWorkflow.html#autotoc_md203", null ],
      [ "구성 파일", "md_Documents_2Reference_2DoxygenWorkflow.html#autotoc_md204", null ],
      [ "유지 관리 원칙", "md_Documents_2Reference_2DoxygenWorkflow.html#autotoc_md205", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "네임스페이스", "namespaces.html", [
      [ "네임스페이스 목록", "namespaces.html", "namespaces_dup" ],
      [ "네임스페이스 멤버", "namespacemembers.html", [
        [ "모두", "namespacemembers.html", null ],
        [ "함수", "namespacemembers_func.html", null ],
        [ "변수", "namespacemembers_vars.html", null ]
      ] ]
    ] ],
    [ "클래스", "annotated.html", [
      [ "클래스 목록", "annotated.html", "annotated_dup" ],
      [ "클래스 색인", "classes.html", null ],
      [ "클래스 계통도", "hierarchy.html", "hierarchy" ],
      [ "클래스 멤버", "functions.html", [
        [ "모두", "functions.html", "functions_dup" ],
        [ "함수", "functions_func.html", "functions_func" ],
        [ "변수", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "파일들", "files.html", [
      [ "파일 목록", "files.html", "files_dup" ],
      [ "파일 멤버", "globals.html", [
        [ "모두", "globals.html", null ],
        [ "함수", "globals_func.html", null ],
        [ "변수", "globals_vars.html", null ],
        [ "열거형 타입", "globals_enum.html", null ],
        [ "매크로", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"AGameCharacter_8cpp.html",
"UGameDataManager_8h.html",
"classAPlayerActor.html#ae7ea1c9e16d4110e77104a2b80364579",
"dir_4471e61838d7f432f5552df1230e81d8.html",
"md_GEMINI.html#autotoc_md101"
];

var SYNCONMSG = '패널 동기화를 비활성화하기 위해 클릭하십시오';
var SYNCOFFMSG = '패널 동기화를 활성화하기 위해 클릭하십시오';