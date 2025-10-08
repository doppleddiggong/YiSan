# CoffeeToolbar

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.0-blue.svg)](https://www.unrealengine.com/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/Docs-GitHub%20Pages-lightgrey.svg)](https://doppleddiggong.github.io/CoffeeToolbar/)

🌐 Languages: [한국어](README.ko.md) | [English](README.en.md)

**Version: 1.0.0**

![Coffee Toolbar Icon](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/icon_coffee_cover_type3.png)

`Coffee Toolbar`는 언리얼 엔진 에디터의 개발 편의성을 향상시키기 위해 제작된 유틸리티 툴바 플러그인입니다. 자주 사용하는 기능들을 원클릭으로 실행하여 반복적인 작업을 줄이고 개발 속도를 높이는 것을 목표로 합니다.

![사용법](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/use_guide.png?raw=true)

## 주요 기능 (Features)

*   **커스텀 콘솔 명령어 실행**: 설정 파일에 미리 정의된 다수의 콘솔 명령어들을 버튼 하나로 실행할 수 있습니다.
*   **레벨 관련 기능**: 현재 레벨의 모든 액터를 빌드하거나, 라이팅을 빌드하는 등 레벨 관리에 유용한 기능들을 제공합니다.
*   **고해상도 스크린샷**: 지정된 배율(예: 2x, 4x)로 고해상도 스크린샷을 간편하게 촬영하고 저장 폴더를 바로 열 수 있습니다.
*   **설정 기반 확장성**: `.ini` 파일을 통해 툴바에 표시될 기능과 순서를 사용자가 직접 커스터마이징할 수 있습니다.
*   **폴더 바로가기**: `Logs`, `Documents` 등 사전에 정의된 프로젝트 폴더를 드롭다운 메뉴에서 직접 열 수 있습니다.

## 설치 방법 (Installation)

### 1. GitHub Release를 통한 설치 (권장)

1.  [GitHub Releases 페이지](https://github.com/doppleddiggong/CoffeeToolbar/releases)에서 최신 버전의 `CoffeeToolbar_X.X.X.zip` 파일을 다운로드합니다.
2.  다운로드한 ZIP 파일의 압축을 해제합니다.
3.  압축 해제된 `CoffeeToolbar` 폴더를 언리얼 엔진 프로젝트의 `Plugins` 폴더 (예: `YourProject/Plugins/`) 또는 언리얼 엔진 설치 경로의 `Engine/Plugins` 폴더 (예: `C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/`) 안에 복사합니다.
4.  언리얼 엔진 에디터를 재시작합니다.
5.  에디터 상단 메뉴에서 `Edit > Plugins`를 선택하고, "CoffeeToolbar"를 검색하여 활성화합니다.
6.  플러그인 활성화 후 에디터 재시작 메시지가 나타나면 재시작합니다.

### 2. 프로젝트에 수동 설치

1.  프로젝트 폴더 내에 `Plugins` 폴더가 없다면 생성합니다.
2.  `CoffeeToolbar` 폴더를 `Plugins` 폴더 안으로 복사합니다.
3.  프로젝트를 재시작한 후, `Edit > Plugins` 메뉴에서 `CoffeeToolbar`를 찾아 활성화합니다.

**경로 예시**: `[YourProjectDirectory]/Plugins/CoffeeToolbar`

### 3. 엔진에 수동 설치

1.  언리얼 엔진 설치 경로의 `Engine/Plugins` 폴더로 이동합니다.
2.  `CoffeeToolbar` 폴더를 `Engine/Plugins` 폴더 안으로 복사합니다.
3.  엔진을 재시작한 후, `Edit > Plugins` 메뉴에서 `CoffeeToolbar`를 찾아 활성화합니다.

**경로 예시**: `C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/CoffeeToolbar`

## 사용법 (How to Use)

1.  플러그인이 활성화되면 에디터 상단 메인 툴바에 새로운 **커피 아이콘**이 나타납니다.
2.  툴바 아이콘을 클릭하면 등록된 기능 버튼들이 드롭다운 메뉴 형식으로 표시됩니다.
3.  원하는 기능의 버튼을 클릭하여 즉시 실행합니다.

## 설정 가이드 (Configuration Guide)

`Coffee Toolbar` 플러그인은 언리얼 엔진의 **Project Settings**를 통해 다양한 기능을 설정하고 커스터마이징할 수 있습니다.

![설정 수정](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/modify_guide.png?raw=true)

### 1. 플러그인 설정 접근

언리얼 에디터에서 **Edit > Project Settings**로 이동한 후, 왼쪽 메뉴의 **Plugins** 섹션에서 **Coffee Toolbar**를 선택합니다.

### 2. 기본 설정 (Default Settings)

여기서 툴바에 표시될 버튼의 순서, 아이콘, 실행할 명령어 등을 설정할 수 있습니다.

![기본 설정](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_defaultSetting.png?raw=true)

### 3. 커스텀 명령어 추가 (Adding Custom Commands)

**Custom Commands** 섹션에서 새로운 요소를 추가하여 원하는 콘솔 명령어 또는 에디터 기능을 툴바 버튼으로 등록할 수 있습니다.

![커맨드 설정](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_command.png?raw=true)

### 4. 레벨 셀렉터 설정 (Level Selector Configuration)

**Level Selector** 기능을 통해 자주 사용하는 레벨들을 등록하고 툴바에서 빠르게 로드할 수 있습니다.


**주요 버튼 설명:**

*   **Add Current Level**: 현재 에디터에 로드된 레벨을 목록에 추가합니다.
*   **Load Selected Level**: 목록에서 선택된 레벨을 에디터에 로드합니다.
*   **Remove Selected Level**: 목록에서 선택된 레벨을 제거합니다.
*   **Clear All Levels**: 목록의 모든 레벨을 제거합니다.

![레벨 셀렉터 설정](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_levelselector.png?raw=true)

### 5. 스크린샷 설정 (Screenshot Capture Feature)

**Screenshot** 기능을 통해 현재 활성 뷰포트의 고해상도 스크린샷을 촬영하거나, 캡처 폴더를 바로 열 수 있습니다.

#### 제공 기능
- **1배 촬영** : 현재 해상도로 캡처
- **2배 촬영** : 2배 배율로 캡처
- **4배 촬영** : 4배 배율로 캡처
- **8배 촬영** : 8배 배율로 캡처
- **캡처 폴더 열기** : 저장된 스크린샷 폴더를 바로 탐색기에서 엽니다

![스크린샷 설정](https://github.com/doppleddiggong/CoffeeToolbar/blob/main/Documents/Reference/guide_screenshot.png?raw=true)

### 6. 폴더 바로가기 설정 (Quick Folder Access)

이 기능은 `Logs`, `Saved`, `Documents` 등 사전 정의된 프로젝트 폴더를 빠르게 열 수 있는 드롭다운 메뉴를 추가합니다.

#### 설정 옵션
- **`bEnableFolderFeature`**: 툴바에 이 기능을 활성화하거나 비활성화합니다.
- **`FoldersToOpen`**: 목록에 표시할 폴더를 정의하는 배열입니다.
  - **`Label`**: 드롭다운 메뉴에 표시될 이름입니다.
  - **`Path`**: 프로젝트 루트 기준의 상대 경로입니다. (예: "Saved/Logs")

#### .ini 설정 예시
프로젝트의 `Config/DefaultToolbarSettings.ini` 파일에서 직접 설정할 수도 있습니다:
```ini
bEnableFolderFeature=True
+FoldersToOpen=(Label="로그",Path="Saved/Logs")
+FoldersToOpen=(Label="문서",Path="Documents")
```

---

## 설정 상세 (Configuration Details)

`Project Settings > Plugins > Coffee Toolbar` 메뉴를 통해 플러그인의 동작을 상세하게 설정할 수 있습니다.

*   **콘솔 명령어 등록**: 툴바를 통해 실행할 커스텀 콘솔 명령어 목록을 추가하거나 수정할 수 있습니다.
*   **툴바 기능 활성화/비활성화**: 툴바에 표시할 개별 기능들을 선택적으로 켜고 끌 수 있습니다.

설정된 내용은 프로젝트의 `Config/ToolbarSettings.ini` 파일에 저장됩니다.


## 라이선스 (License)

본 플러그인은 MIT 라이선스 하에 배포됩니다. 자세한 내용은 `LICENSE` 파일을 참고하십시오.

## Author

**dopple**

- GitHub: [doppleddiggong](https://github.com/doppleddiggong)  
- LinkedIn: [주백 배](https://www.linkedin.com/in/주백-배-4a814527b/)  
- Email: ju100.bae@gmail.com  
