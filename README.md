# YiSan (이산: 正祖) — 기술 중심의 역사 체험 프로젝트

<p align="center">
  <a href="https://www.unrealengine.com/">
    <img src="https://img.shields.io/badge/Unreal_Engine-5.6-blue.svg" alt="UE5.6">
  </a>
  <a href="https://isocpp.org/">
    <img src="https://img.shields.io/badge/C++-20-blue.svg" alt="C++20">
  </a>
  <a href="https://doppleddiggong.github.io/YiSan/doxygen/">
    <img src="https://img.shields.io/badge/API_Docs-Doxygen-blue.svg" alt="API Docs">
  </a>
  <a href="https://doppleddiggong.github.io/YiSan/docs/">
    <img src="https://img.shields.io/badge/개발_문서-웹사이트-orange.svg" alt="Dev Docs">
  </a>
</p>

<p align="center">
  <a href="https://youtu.be/iJ_77rZhphI">
    <img src="https://github.com/doppleddiggong/YiSan/blob/main/Documents/Reference/Title.png?raw=true"
         width="500"
         alt="Game Title">
  </a>
  <br/>
  <sub>이미지를 클릭하면 시연 영상으로 이동합니다</sub>
</p>

`YiSan`은 언리얼 엔진 5와 C++20 기반으로 개발된 프로젝트 소개용 기술 데모로서,  
**정조대왕 능행차**를 배경으로 한 **대규모 AI 상호작용**, **모듈형 C++ 아키텍처**,  
**멀티플레이 시스템**을 갖춘 기술 중심 프로젝트입니다.

---

## 🎥 프로젝트 핵심 기능 최종 시연 영상  
👉 https://youtu.be/iJ_77rZhphI

---

## ✨ 주요 특징
- **AI 기반 음성 대화:** OpenAI GPT 연동을 통해 플레이어와 실시간으로 대화하는 지능형 NPC 구현.
- **모듈형 C++ 아키텍처:** CoffeeLibrary·LatteLibrary 기반으로 확장성과 유지보수성을 극대화한 구조 설계.

---

## 🎯 핵심 기술 과제 및 해결

### 1. 깊이 있는 NPC 상호작용
- **도전 과제:** 스크립트 기반 NPC는 반복 응답으로 몰입감이 떨어짐.  
- **적용 기술:** GPT API + STT/TTS 통합 C++ 상호작용 시스템 구축.  
- **결과:** 플레이어 의도 파악 및 매번 다른 음성 응답 생성이 가능한 지능형 NPC 완성.

### 2. World Partition 기반 멀티플레이어 동기화
- **도전 과제:** 동적 스트리밍 환경에서 NPC·퀘스트 상태를 난입 플레이어에게 일관성 있게 복제하기 어려움.  
- **적용 기술:** Listen Server + Replication 기반 GameState/액터 상태 동기화 로직 설계.  
- **결과:** 언제든 세션 참여가 가능한 끊김 없는 멀티플레이 탐험 환경 구축.

---

## 🛠️ 기술 스택

| 구분 | 기술 | 목적 |
|------|------|------|
| **Engine** | Unreal Engine 5.6 | World Partition·Nanite 기반 고품질 월드 구현 |
| **Core System** | C++20, CoffeeLibrary, LatteLibrary | 모듈형·고성능 게임플레이 아키텍처 |
| **AI** | OpenAI GPT | 생성형 AI 기반 NPC 상호작용 |
| **Networking** | Listen Server, Replication | Join-in-Progress 멀티플레이 지원 |
| **Automation** | GitHub Actions, PowerShell | CI/CD, 빌드, 테스트, 문서 자동화 |

---

## ⚙️ 자동화된 개발 워크플로우
- C++ API 문서 자동 생성 (Doxygen)  
- 개발 문서 웹사이트 자동 배포  
- Daily DevLog 자동 수집 및 작성  
- Pull Request 자동 리뷰 (AI 활용)

---

📚 **개발 문서 및 로그:** https://doppleddiggong.github.io/YiSan/docs/  
🔍 **API 참조:** https://doppleddiggong.github.io/YiSan/doxygen/