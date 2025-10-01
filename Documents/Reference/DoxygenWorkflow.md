# Project YiSan – Doxygen 워크플로

## 목적
- Unreal Engine 5.6 기반 코드베이스의 API 문서를 통일된 방식으로 생성
- 리플렉션 매크로와 모듈 매크로가 포함된 환경에서도 일관된 결과 확보

## 산출물 경로
- HTML: `Documents/Doxygen/html/index.html`
- 경고 로그: `doxygen_warnings.log` (Git 무시 대상)

## 실행 절차
1. PowerShell에서 저장소 루트로 이동
2. 스크립트 실행
   ```powershell
   powershell -ExecutionPolicy Bypass -File Tools/Doxygen/RunDoxygen.ps1
   ```
   - `-Clean` 옵션을 추가하면 기존 문서와 경고 로그를 삭제 후 재생성
3. 경고 로그를 확인하여 누락된 심볼이나 매크로를 점검

## 구성 파일
- `Doxyfile` : 저장소 루트에 위치, Unreal 리플렉션 매크로를 `PREDEFINED`로 치환하여 문서화 가능하도록 설정
- Git에 포함되므로 수정 시 PR 검토를 거쳐 팀 합의를 반영

## 유지 관리 원칙
- 스크립트와 설정 파일은 Git으로 관리, 산출물은 `.gitignore`로 제외
- 경고가 새로 발생하면 체크리스트에 기록하고 원인 파악 후 설정 혹은 코드 주석을 보완
- CI 파이프라인 연동 시 `Tools/Doxygen/RunDoxygen.ps1`를 재사용해 동일한 동작을 보장
