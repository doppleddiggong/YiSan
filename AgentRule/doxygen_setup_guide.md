# Unreal Engine C++ 프로젝트 Doxygen 문서화 가이드

## 1. 개요

이 가이드는 Unreal Engine C++ 프로젝트에서 Doxygen을 사용하여 코드 문서를 생성하고, "Awesome Doxygen Theme"을 적용하며, GitHub Actions를 통해 자동으로 GitHub Pages에 배포하는 방법을 설명합니다. 이 시스템은 프로젝트의 C++ API 문서를 최신 상태로 유지하고, 개발자 및 협업자에게 코드 베이스에 대한 명확한 이해를 제공하는 데 기여합니다.

## 2. 전제 조건

*   **GitHub 저장소:** 문서화할 C++ 코드가 푸시되어 있는 GitHub 저장소.
*   **Doxygen:** 로컬에서 Doxygen을 테스트하려면 설치가 필요합니다. (예: `sudo apt-get install doxygen`)
*   **Graphviz:** 클래스 다이어그램 등 그래프 생성을 위해 필요합니다. (예: `sudo apt-get install graphviz`)

## 3. Doxyfile 설정

`Doxyfile`은 Doxygen이 문서를 생성하는 방법을 정의하는 핵심 설정 파일입니다. 프로젝트 루트에 위치한 `Doxyfile`을 통해 문서 생성 규칙을 정의합니다.

*   **주요 설정:**
    *   `PROJECT_NAME`: 문서의 프로젝트 이름.
    *   `OUTPUT_DIRECTORY`: 생성된 문서가 저장될 디렉토리 (예: `docs`).
    *   `INPUT`: 문서화할 소스 코드의 경로 (예: `Source`).
    *   `FILE_PATTERNS`: 문서화할 파일 확장자 (예: `*.cpp *.h`).
    *   `RECURSIVE`: `INPUT` 경로를 재귀적으로 탐색할지 여부.
    *   `HAVE_DOT`: Graphviz 설치 여부 (클래스 다이어그램 생성).
    *   `HTML_EXTRA_STYLESHEET`: "Awesome Doxygen CSS" 테마 적용을 위한 스타일시트 경로.

프로젝트의 `Doxyfile`은 [여기](Doxyfile)에서 확인할 수 있습니다.

## 4. GitHub Actions 워크플로우 (`doxygen.yml`) 설정

`.github/workflows/doxygen.yml` 파일을 생성하여 Doxygen 문서 생성 및 GitHub Pages 배포를 자동화합니다.

```yaml
name: Generate Doxygen Documentation

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]
  workflow_dispatch: # 수동 실행 허용

permissions:
  contents: write # gh-pages 브랜치에 푸시 권한 부여

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - name: Checkout repository
      uses: actions/checkout@v4

    - name: Install Doxygen and Graphviz
      run: sudo apt-get update && sudo apt-get install -y doxygen graphviz

    - name: Download Doxygen Awesome CSS
      run: |
        wget https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome.css -O doxygen-awesome.css
        mkdir -p docs/html
        mv doxygen-awesome.css docs/html/

    - name: Generate Doxygen documentation
      run: doxygen Doxyfile

    - name: Deploy to GitHub Pages
      uses: peaceiris/actions-gh-pages@v4 # 최신 버전 사용
      if: ${{ github.ref == 'refs/heads/main' }}
      with:
        github_token: ${{ secrets.GITHUB_TOKEN }}
        publish_dir: ./docs/html
        destination_dir: . # gh-pages 브랜치의 루트에 배포
```

## 5. GitHub 저장소 설정

### 5.1. GitHub Actions 권한 설정

`github-actions[bot]`이 `gh-pages` 브랜치에 푸시할 수 있도록 권한을 부여해야 합니다.
1.  GitHub 저장소 **"Settings" (설정)** 탭으로 이동.
2.  왼쪽 사이드바에서 **"Actions"** 아래의 **"General"** 클릭.
3.  **"Workflow permissions" (워크플로우 권한)** 섹션에서 **"Read and write permissions" (읽기 및 쓰기 권한)** 옵션을 선택하고 **"Save" (저장)**.

### 5.2. GitHub Pages 소스 설정

GitHub Pages가 `gh-pages` 브랜치에서 문서를 제공하도록 설정합니다.
1.  GitHub 저장소 **"Settings" (설정)** 탭으로 이동.
2.  왼쪽 사이드바에서 **"Pages"** 클릭.
3.  "Build and deployment" 섹션에서 **"Source"** 드롭다운 메뉴를 클릭하여 **"Deploy from a branch"**를 선택.
4.  **"Branch"** 드롭다운 메뉴에서 **`gh-pages`**를 선택하고, 폴더는 **`/ (root)`**로 그대로 둡니다.
5.  **"Save" (저장)** 버튼을 클릭.

## 6. Doxygen 주석 가이드라인

Doxygen을 통해 일관되고 유용한 문서를 생성하기 위해 다음 주석 가이드라인을 준수합니다.

### 6.1. 일반 원칙

*   모든 공개(Public) 클래스, 구조체, 열거형, 함수, 멤버 변수에는 Doxygen 주석을 추가합니다.
*   주석은 코드의 목적, 사용법, 중요한 세부 사항을 명확하게 설명해야 합니다.
*   간결하면서도 정보가 풍부하도록 작성합니다.

### 6.2. 주석 스타일 및 위치

*   **블록 주석 (Block Comments):** `/** ... */` 스타일을 사용하며, 여러 줄에 걸친 설명이나 Doxygen 명령어를 포함할 때 사용합니다.
    *   클래스, 구조체, 열거형, 함수 정의 바로 위에 위치합니다.
*   **한 줄 주석 (Brief Comments):** `///` 스타일을 사용하며, 짧은 설명이나 멤버 변수 옆에 인라인으로 사용할 때 유용합니다.

### 6.3. 주요 Doxygen 명령어

*   `@brief <설명>`: 요소에 대한 간결한 한 줄 요약입니다.
*   `@details <설명>`: 요소에 대한 상세한 설명입니다.
*   `@param <매개변수명> <설명>`: 함수의 각 매개변수에 대한 설명입니다.
*   `@return <설명>`: 함수의 반환 값에 대한 설명입니다.
*   `@see <다른 요소>`: 관련 있는 다른 클래스, 함수, 파일 등을 참조할 때 사용합니다.
*   `@note <내용>`: 특별히 주의해야 할 사항을 명시합니다.
*   `@warning <내용>`: 잠재적인 문제점이나 위험을 경고합니다.
*   `@todo <내용>`: 향후 구현 또는 개선이 필요한 사항을 기록합니다.
*   `@file <파일명>`: 헤더 파일의 최상단에 파일의 목적을 설명할 때 사용합니다.

### 6.4. 예시

#### 클래스 주석 예시

```cpp
/**
 * @brief 플레이어 캐릭터의 기본 액터 클래스입니다.
 *
 * @details 이 클래스는 플레이어의 움직임, 상호작용, 전투 등 핵심 게임플레이 로직을 담당합니다.
 *          주요 기능으로는 입력 처리, 스탯 관리 시스템과의 연동 등이 있습니다.
 *
 * @see UStatSystem
 * @see UKnockbackSystem
 */
UCLASS()
class LATTELIBRARY_API APlayerActor : public ACharacter
{
    // ...
};
```

#### 함수 주석 예시

```cpp
/**
 * @brief 캐릭터의 현재 체력을 반환합니다.
 *
 * @param bIncludeTemporaryHealth 임시 체력을 포함할지 여부입니다.
 * @return 현재 체력 값을 반환합니다.
 */
float GetCurrentHealth(bool bIncludeTemporaryHealth) const;
```

#### 멤버 변수 주석 예시

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
float MaxHealth; ///< 캐릭터의 최대 체력입니다.
```

---

**참고:** Doxygen 설정 및 GitHub Actions 워크플로우는 프로젝트의 요구사항과 GitHub 정책 변경에 따라 업데이트될 수 있습니다. 최신 정보는 Doxygen 공식 문서 및 GitHub Actions 문서를 참조하십시오.