# YiSan 멀티플레이 로비 UI 디자인 가이드 (Figma)

**작성일:** 2025-10-20
**버전:** 1.0
**대상:** UI/UX 디자이너, Figma 사용자

---

## 📑 목차

1. [개요](#1-개요)
2. [디자인 스펙](#2-디자인-스펙)
3. [컴포넌트 구조](#3-컴포넌트-구조)
4. [UI 요소 상세](#4-ui-요소-상세)
5. [색상 팔레트](#5-색상-팔레트)
6. [타이포그래피](#6-타이포그래피)
7. [레이아웃 가이드](#7-레이아웃-가이드)
8. [인터랙션 상태](#8-인터랙션-상태)
9. [반응형 디자인](#9-반응형-디자인)
10. [에셋 내보내기](#10-에셋-내보내기)

---

## 1. 개요

### 1.1 디자인 목표

- **명확성**: 사용자가 Host/Join 선택을 쉽게 이해
- **접근성**: 큰 버튼, 명확한 텍스트, 고대비
- **한국 문화 테마**: YiSan 프로젝트의 문화유산 콘셉트 반영
- **반응형**: 다양한 화면 비율 지원 (16:9, 21:9, 4:3)

### 1.2 기술 제약사항

- **엔진**: Unreal Engine 5 (UMG 위젯)
- **해상도**: 1920x1080 기준 (Full HD)
- **스케일 모드**: Scale to Fit
- **폰트**: 한글 지원 필수 (Noto Sans KR 권장)

---

## 2. 디자인 스펙

### 2.1 캔버스 설정

```
Figma Frame 생성:
  - Name: "Lobby UI - Desktop"
  - Width: 1920px
  - Height: 1080px
  - Background: #1A1A2E (어두운 네이비)
```

### 2.2 그리드 시스템

```
Layout Grid:
  - Type: Rows
  - Count: 12
  - Gutter: 20px
  - Margin: 100px (좌우)

Center Guide:
  - Vertical: X = 960px
  - Horizontal: Y = 540px
```

---

## 3. 컴포넌트 구조

### 3.1 Hierarchy (계층 구조)

```
🖼️ Frame: Lobby UI - Desktop (1920x1080)
│
├─ 🌌 Background
│   ├─ Image: 한국 전통 건축물 (블러 처리)
│   └─ Overlay: Gradient (Top: #1A1A2E 80%, Bottom: #0F0F1E 100%)
│
├─ 🎭 Logo Section (상단 중앙)
│   ├─ Logo: "YiSan" (이미지 또는 텍스트)
│   └─ Subtitle: "문화유산 탐험 멀티플레이"
│
├─ 📦 Main Container (중앙)
│   ├─ 🏷️ Title
│   │   └─ Text: "멀티플레이 로비"
│   │
│   ├─ 📊 Status Display
│   │   ├─ Icon: 🔄 (상태에 따라 변경)
│   │   └─ Text: "대기 중..." (동적 변경)
│   │
│   ├─ 📝 IP Input Section
│   │   ├─ Label: "서버 주소"
│   │   └─ TextBox: "192.168.0.10 입력"
│   │
│   └─ 🎮 Button Group
│       ├─ Button: Host Game
│       ├─ Button: Join Game
│       └─ Button: Disconnect (초기 숨김)
│
└─ ℹ️ Footer Section (하단)
    ├─ Text: "도움말"
    └─ Text: "버전 정보"
```

---

## 4. UI 요소 상세

### 4.1 Logo Section

#### 위치 및 크기

```
Position:
  - X: 960px (중앙)
  - Y: 120px (상단에서)

Logo:
  - Width: 300px
  - Height: 80px
  - Format: SVG 또는 PNG (투명 배경)

Subtitle:
  - Font: Noto Sans KR Medium
  - Size: 20px
  - Color: #B0B3C5 (연한 회색)
  - Letter Spacing: 2%
```

#### Figma에서 만들기

1. **Text 툴** (T) 선택
2. 텍스트 입력: `YiSan`
3. 스타일 적용:
   ```
   Font Family: Noto Sans KR (또는 맞춤 폰트)
   Weight: Bold
   Size: 64px
   Color: #FFD700 (금색)
   Effects:
     - Drop Shadow: Y=4px, Blur=8px, Color=#000000 50%
   ```
4. Subtitle 추가:
   ```
   Text: "문화유산 탐험 멀티플레이"
   Font: Noto Sans KR Medium
   Size: 20px
   Color: #B0B3C5
   ```

---

### 4.2 Main Container

#### 위치 및 크기

```
Container:
  - X: 960px (중앙 정렬)
  - Y: 540px (수직 중앙)
  - Width: 600px
  - Height: Auto (내용에 따라)
  - Background: #16213E (어두운 블루)
  - Border Radius: 20px
  - Padding: 40px (내부 여백)
  - Shadow: 0px 20px 60px rgba(0,0,0,0.5)
```

#### Figma에서 만들기

1. **Rectangle 툴** (R) 선택
2. 크기: `600px × Auto`
3. **Auto Layout** 적용:
   ```
   Direction: Vertical
   Spacing: 24px
   Padding: 40px (모든 방향)
   Alignment: Center
   ```
4. **Fill** 설정:
   ```
   Color: #16213E
   ```
5. **Stroke** 설정:
   ```
   Width: 2px
   Color: #FFD700 (금색 테두리)
   ```
6. **Effects** 추가:
   ```
   Drop Shadow:
     - X: 0px
     - Y: 20px
     - Blur: 60px
     - Color: #000000 (Opacity: 50%)
   ```

---

### 4.3 Title (제목)

#### 스펙

```
Text: "멀티플레이 로비"
Font: Noto Sans KR Bold
Size: 36px
Color: #FFFFFF
Letter Spacing: 0%
Line Height: 44px
Alignment: Center
```

#### Figma에서 만들기

1. Text 툴 (T) 선택
2. Main Container 내부에 배치
3. 스타일 적용 (위 스펙 참조)

---

### 4.4 Status Display (상태 표시)

#### 스펙

```
Container:
  - Width: 100% (부모 컨테이너 기준)
  - Height: 60px
  - Background: rgba(255, 255, 255, 0.05) (반투명 흰색)
  - Border Radius: 12px
  - Padding: 16px

Icon:
  - Size: 28px × 28px
  - Color: #FFD700 (금색)
  - 상태에 따라 변경:
    - 대기: 🔄 (회전 애니메이션)
    - 연결 중: ⏳
    - 성공: ✅
    - 오류: ❌

Text:
  - Font: Noto Sans KR Medium
  - Size: 20px
  - Color: #FFFFFF
  - 동적 텍스트:
    - "대기 중..."
    - "호스트 생성 중..."
    - "서버 접속 중..."
    - "연결 성공!"
    - "오류: 서버 연결 실패"
```

#### Figma에서 만들기

1. Rectangle 생성 → Auto Layout 적용
   ```
   Direction: Horizontal
   Spacing: 12px
   Padding: 16px
   Alignment: Left, Center
   ```
2. Icon 추가 (Text 또는 이미지)
3. Status Text 추가

---

### 4.5 IP Input Section

#### 스펙

```
Label:
  - Text: "서버 주소"
  - Font: Noto Sans KR Medium
  - Size: 16px
  - Color: #B0B3C5
  - Margin Bottom: 8px

TextBox:
  - Width: 100%
  - Height: 50px
  - Background: rgba(255, 255, 255, 0.08)
  - Border: 2px solid rgba(255, 255, 255, 0.2)
  - Border Radius: 8px
  - Padding: 12px 16px

  Placeholder:
    - Text: "예: 192.168.0.10"
    - Font: Noto Sans KR Regular
    - Size: 16px
    - Color: rgba(255, 255, 255, 0.4)

  Focus State:
    - Border: 2px solid #FFD700
    - Background: rgba(255, 255, 255, 0.12)

  Input Text:
    - Font: Noto Sans KR Medium
    - Size: 18px
    - Color: #FFFFFF
```

#### Figma에서 만들기

1. **Label 생성**:
   - Text: "서버 주소"
   - 스타일 적용 (위 스펙 참조)

2. **TextBox 생성**:
   - Rectangle 생성 (520px × 50px)
   - Auto Layout 적용:
     ```
     Direction: Horizontal
     Padding: 12px 16px
     Alignment: Left, Center
     ```
   - Fill: `rgba(255, 255, 255, 0.08)`
   - Stroke: `2px, rgba(255, 255, 255, 0.2)`
   - Corner Radius: `8px`

3. **Placeholder Text 추가**:
   - Text: "예: 192.168.0.10"
   - Font: Noto Sans KR Regular
   - Size: 16px
   - Color: `rgba(255, 255, 255, 0.4)`

4. **Component Variant 생성** (선택 사항):
   - Default State
   - Focus State
   - Error State

---

### 4.6 Button Group

#### 공통 스펙

```
Button Container:
  - Width: 100%
  - Auto Layout: Vertical
  - Spacing: 12px
  - Alignment: Center
```

---

#### 4.6.1 Host Button

```
Button:
  - Width: 100%
  - Height: 60px
  - Background: Linear Gradient
    - Start: #4CAF50 (밝은 녹색)
    - End: #2E7D32 (어두운 녹색)
  - Border Radius: 12px
  - Shadow: 0px 4px 12px rgba(76, 175, 80, 0.4)

Text:
  - Content: "🎮 Host Game"
  - Font: Noto Sans KR Bold
  - Size: 22px
  - Color: #FFFFFF
  - Letter Spacing: 1%

Hover State:
  - Background: Linear Gradient
    - Start: #66BB6A
    - End: #388E3C
  - Transform: Scale(1.02)
  - Shadow: 0px 6px 16px rgba(76, 175, 80, 0.6)

Active (Pressed) State:
  - Background: Linear Gradient
    - Start: #388E3C
    - End: #1B5E20
  - Transform: Scale(0.98)
  - Shadow: 0px 2px 8px rgba(76, 175, 80, 0.3)

Disabled State:
  - Background: #424242 (회색)
  - Text Color: rgba(255, 255, 255, 0.3)
  - Shadow: None
```

#### Figma에서 만들기

1. **Rectangle 생성** (520px × 60px)
2. **Auto Layout 적용**:
   ```
   Direction: Horizontal
   Padding: 16px
   Alignment: Center, Center
   ```
3. **Fill 설정** (Gradient):
   - Type: Linear
   - Angle: 90°
   - Stop 1: #4CAF50 (0%)
   - Stop 2: #2E7D32 (100%)
4. **Corner Radius**: `12px`
5. **Effects** (Shadow):
   - Type: Drop Shadow
   - Y: 4px
   - Blur: 12px
   - Color: rgba(76, 175, 80, 0.4)
6. **Text 추가**:
   - Content: "🎮 Host Game"
   - 스타일 적용 (위 스펙 참조)

7. **Component 생성** → **Add Variant**:
   - Default
   - Hover
   - Pressed
   - Disabled

---

#### 4.6.2 Join Button

```
Button:
  - Width: 100%
  - Height: 60px
  - Background: Linear Gradient
    - Start: #2196F3 (밝은 블루)
    - End: #1565C0 (어두운 블루)
  - Border Radius: 12px
  - Shadow: 0px 4px 12px rgba(33, 150, 243, 0.4)

Text:
  - Content: "🔗 Join Game"
  - Font: Noto Sans KR Bold
  - Size: 22px
  - Color: #FFFFFF
  - Letter Spacing: 1%

Hover State:
  - Background: Linear Gradient
    - Start: #42A5F5
    - End: #1976D2
  - Transform: Scale(1.02)
  - Shadow: 0px 6px 16px rgba(33, 150, 243, 0.6)

Active (Pressed) State:
  - Background: Linear Gradient
    - Start: #1976D2
    - End: #0D47A1
  - Transform: Scale(0.98)
  - Shadow: 0px 2px 8px rgba(33, 150, 243, 0.3)

Disabled State:
  - Background: #424242
  - Text Color: rgba(255, 255, 255, 0.3)
  - Shadow: None
```

**Figma에서 만들기:** Host Button과 동일한 방법, Gradient 색상만 변경

---

#### 4.6.3 Disconnect Button

```
Button:
  - Width: 100%
  - Height: 60px
  - Background: Linear Gradient
    - Start: #F44336 (밝은 빨강)
    - End: #C62828 (어두운 빨강)
  - Border Radius: 12px
  - Shadow: 0px 4px 12px rgba(244, 67, 54, 0.4)

Text:
  - Content: "❌ Disconnect"
  - Font: Noto Sans KR Bold
  - Size: 22px
  - Color: #FFFFFF
  - Letter Spacing: 1%

⚠️ 초기 상태: Visibility = Hidden (숨김)

Hover State:
  - Background: Linear Gradient
    - Start: #EF5350
    - End: #D32F2F
  - Transform: Scale(1.02)
  - Shadow: 0px 6px 16px rgba(244, 67, 54, 0.6)

Active (Pressed) State:
  - Background: Linear Gradient
    - Start: #D32F2F
    - End: #B71C1C
  - Transform: Scale(0.98)
  - Shadow: 0px 2px 8px rgba(244, 67, 54, 0.3)
```

**Figma에서 만들기:** 위 버튼과 동일, Gradient는 빨강 계열

---

### 4.7 Footer Section

#### 스펙

```
Container:
  - Position: Fixed Bottom
  - Y: 1020px (하단에서 60px)
  - Width: 100%
  - Height: 40px
  - Background: Transparent

Help Text:
  - Text: "도움말: Host = 서버 생성, Join = 서버 접속"
  - Font: Noto Sans KR Regular
  - Size: 14px
  - Color: rgba(255, 255, 255, 0.6)
  - Alignment: Center

Version Text:
  - Text: "v1.0.0 | YiSan Multiplayer"
  - Font: Noto Sans KR Light
  - Size: 12px
  - Color: rgba(255, 255, 255, 0.4)
  - Alignment: Right
  - X: 1820px (우측 정렬)
```

---

## 5. 색상 팔레트

### 5.1 Primary Colors (주 색상)

```
Brand Gold (브랜드 골드):
  - HEX: #FFD700
  - RGB: 255, 215, 0
  - 용도: 로고, 강조, 테두리

Dark Navy (어두운 네이비):
  - HEX: #1A1A2E
  - RGB: 26, 26, 46
  - 용도: 배경

Deep Blue (깊은 블루):
  - HEX: #16213E
  - RGB: 22, 33, 62
  - 용도: 컨테이너 배경
```

### 5.2 Semantic Colors (의미론적 색상)

```
Success Green (성공 녹색):
  - Primary: #4CAF50
  - Dark: #2E7D32
  - Light: #66BB6A
  - 용도: Host 버튼, 성공 메시지

Info Blue (정보 블루):
  - Primary: #2196F3
  - Dark: #1565C0
  - Light: #42A5F5
  - 용도: Join 버튼, 정보 메시지

Danger Red (위험 빨강):
  - Primary: #F44336
  - Dark: #C62828
  - Light: #EF5350
  - 용도: Disconnect 버튼, 오류 메시지

Warning Yellow (경고 노랑):
  - Primary: #FFC107
  - Dark: #F57C00
  - Light: #FFD54F
  - 용도: 경고 메시지
```

### 5.3 Neutral Colors (중립 색상)

```
White (흰색):
  - HEX: #FFFFFF
  - RGB: 255, 255, 255
  - 용도: 텍스트 (주요)

Light Gray (연한 회색):
  - HEX: #B0B3C5
  - RGB: 176, 179, 197
  - 용도: 텍스트 (보조)

Dark Gray (어두운 회색):
  - HEX: #424242
  - RGB: 66, 66, 66
  - 용도: 비활성화 상태
```

---

## 6. 타이포그래피

### 6.1 폰트 패밀리

```
Primary Font: Noto Sans KR
  - Source: Google Fonts
  - Weights: Regular (400), Medium (500), Bold (700)
  - Language: 한국어, English
  - License: SIL Open Font License

Fallback Font: Malgun Gothic (맑은 고딕)
  - System Font (Windows)
```

### 6.2 Text Styles

#### Heading 1 (제목 1)

```
Font: Noto Sans KR Bold
Size: 64px
Line Height: 80px
Letter Spacing: 0%
Color: #FFD700
Use Case: 로고 텍스트
```

#### Heading 2 (제목 2)

```
Font: Noto Sans KR Bold
Size: 36px
Line Height: 44px
Letter Spacing: 0%
Color: #FFFFFF
Use Case: 메인 제목 ("멀티플레이 로비")
```

#### Heading 3 (제목 3)

```
Font: Noto Sans KR Bold
Size: 22px
Line Height: 28px
Letter Spacing: 1%
Color: #FFFFFF
Use Case: 버튼 텍스트
```

#### Body 1 (본문 1)

```
Font: Noto Sans KR Medium
Size: 20px
Line Height: 28px
Letter Spacing: 0%
Color: #FFFFFF
Use Case: 상태 메시지
```

#### Body 2 (본문 2)

```
Font: Noto Sans KR Regular
Size: 16px
Line Height: 24px
Letter Spacing: 0%
Color: #B0B3C5
Use Case: Label, Placeholder
```

#### Caption (캡션)

```
Font: Noto Sans KR Regular
Size: 14px
Line Height: 20px
Letter Spacing: 0%
Color: rgba(255, 255, 255, 0.6)
Use Case: 도움말, 부가 정보
```

---

## 7. 레이아웃 가이드

### 7.1 Spacing System (간격 시스템)

```
Spacing Scale (8pt Grid):
  - 4px   (0.5 unit) → 아주 작은 여백
  - 8px   (1 unit)   → 작은 여백
  - 12px  (1.5 unit) → 버튼 내부 여백
  - 16px  (2 units)  → 기본 여백
  - 24px  (3 units)  → 컴포넌트 간 여백
  - 32px  (4 units)  → 섹션 간 여백
  - 40px  (5 units)  → 컨테이너 패딩
  - 48px  (6 units)  → 큰 여백
  - 64px  (8 units)  → 섹션 구분
```

### 7.2 Component Sizes

```
Button Heights:
  - Small: 40px
  - Medium: 50px
  - Large: 60px ← 로비 UI 사용

Input Heights:
  - Text Input: 50px

Container Widths:
  - Narrow: 400px
  - Medium: 600px ← 로비 UI 사용
  - Wide: 800px
```

---

## 8. 인터랙션 상태

### 8.1 버튼 상태 정의

#### Default (기본)

```
Opacity: 100%
Transform: Scale(1)
Cursor: Pointer
```

#### Hover (마우스 오버)

```
Transition: 0.2s ease-out
Transform: Scale(1.02)
Shadow: Increased (더 큰 그림자)
Brightness: +10%
Cursor: Pointer
```

#### Active/Pressed (클릭)

```
Transition: 0.1s ease-in
Transform: Scale(0.98)
Shadow: Decreased (작은 그림자)
Brightness: -10%
```

#### Disabled (비활성화)

```
Opacity: 50%
Background: Grayscale
Cursor: Not-Allowed
Pointer Events: None
```

---

### 8.2 Input 상태 정의

#### Default

```
Border: 2px solid rgba(255, 255, 255, 0.2)
Background: rgba(255, 255, 255, 0.08)
```

#### Focus (포커스)

```
Border: 2px solid #FFD700
Background: rgba(255, 255, 255, 0.12)
Glow: 0px 0px 8px rgba(255, 215, 0, 0.4)
```

#### Error (오류)

```
Border: 2px solid #F44336
Background: rgba(244, 67, 54, 0.1)
```

---

## 9. 반응형 디자인

### 9.1 브레이크포인트

```
Desktop (데스크톱):
  - Min Width: 1920px
  - Container Width: 600px

Laptop (노트북):
  - Min Width: 1366px
  - Container Width: 500px

Tablet (태블릿):
  - Min Width: 1024px
  - Container Width: 400px
  - Font Scale: 90%

Mobile (모바일):
  - Min Width: 768px
  - Container Width: 340px
  - Font Scale: 85%
```

### 9.2 Figma에서 반응형 설정

1. **Container에 Constraints 설정**:
   ```
   Horizontal: Center
   Vertical: Center
   ```

2. **Auto Layout 사용**:
   - Main Container에 적용
   - Direction: Vertical
   - Resizing: Hug Contents

3. **Component Variants 생성**:
   - Desktop Variant
   - Laptop Variant
   - Tablet Variant
   - Mobile Variant

---

## 10. 에셋 내보내기

### 10.1 이미지 에셋

#### 로고

```
Format: PNG
Size: 300px × 80px
DPI: 300 (고해상도)
Background: Transparent
Naming: logo_yisan.png

Export Settings (Figma):
  - 1x: PNG (기본)
  - 2x: PNG (레티나 디스플레이)
  - SVG: Vector (확대/축소)
```

#### 아이콘

```
Format: SVG (벡터)
Size: 24px × 24px, 32px × 32px, 48px × 48px
Naming: icon_[name].svg

Export Settings:
  - SVG
  - Outline Stroke (선을 패스로 변환)
```

#### 배경 이미지

```
Format: JPEG
Size: 1920px × 1080px
Quality: 90%
Blur: Gaussian Blur 20px (적용 후 내보내기)
Naming: bg_lobby.jpg
```

---

### 10.2 UI 컴포넌트 내보내기

#### Figma → Unreal Engine

**방법 1: PNG 스프라이트 시트**

```
1. 모든 버튼 상태를 나란히 배치
   (Default, Hover, Pressed, Disabled)

2. Figma에서 선택 → Export

3. 설정:
   - Format: PNG
   - Scale: 2x (고해상도)
   - Naming: btn_host_spritesheet.png

4. Unreal Engine에서:
   - Import PNG
   - Paper2D Sprite 생성
   - 상태별로 분할
```

**방법 2: 개별 PNG**

```
각 버튼 상태를 개별 파일로 내보내기:
  - btn_host_default.png
  - btn_host_hover.png
  - btn_host_pressed.png
  - btn_host_disabled.png

Unreal Engine에서:
  - 각 이미지를 Texture2D로 Import
  - UMG Button Style에 적용
```

---

### 10.3 내보내기 체크리스트

```
✅ 로고 (PNG, SVG)
✅ 배경 이미지 (JPEG, 블러 처리)
✅ 버튼 상태별 이미지 (PNG × 4 states × 3 buttons = 12개)
✅ 아이콘 (SVG)
✅ 텍스트 스타일 문서 (폰트, 크기, 색상)
✅ 색상 팔레트 문서 (HEX, RGB)
```

---

## 11. Figma 플러그인 추천

### 11.1 Color Palette Generator

- **Coolors**: 자동 색상 팔레트 생성
- **Stark**: 접근성 체크 (대비 확인)

### 11.2 Icon Libraries

- **Iconify**: 무료 아이콘 라이브러리
- **Material Design Icons**: Google Material 아이콘

### 11.3 Export Tools

- **Figma to HTML**: 웹 코드 생성 (참고용)
- **Image Optimizer**: 이미지 압축

---

## 12. Figma 파일 구조 예시

```
📁 YiSan Multiplayer Lobby UI
│
├─ 📄 Cover (커버 페이지)
│   └─ 프로젝트 소개, 버전 정보
│
├─ 📄 Design System (디자인 시스템)
│   ├─ Color Palette (색상 팔레트)
│   ├─ Typography (타이포그래피)
│   ├─ Spacing (간격)
│   └─ Icons (아이콘)
│
├─ 📄 Components (컴포넌트)
│   ├─ 🔘 Buttons (버튼)
│   │   ├─ Host Button (4 states)
│   │   ├─ Join Button (4 states)
│   │   └─ Disconnect Button (4 states)
│   ├─ 📝 Input (입력)
│   │   └─ IP TextBox (3 states)
│   ├─ 📊 Status Display (상태 표시)
│   └─ 🏷️ Labels (레이블)
│
├─ 📄 Screens (화면)
│   ├─ Lobby UI - Desktop (1920x1080)
│   ├─ Lobby UI - Laptop (1366x768)
│   ├─ Lobby UI - Tablet (1024x768)
│   └─ Lobby UI - Mobile (768x1024)
│
└─ 📄 Prototypes (프로토타입)
    ├─ Flow: Host 선택
    ├─ Flow: Join 선택
    └─ Flow: Disconnect
```

---

## 13. 프로토타입 인터랙션

### 13.1 Figma Prototype 설정

#### Host 버튼 클릭

```
Trigger: On Click
Action: Navigate to
Destination: Frame "MainGame Screen" (메인 게임 화면)
Animation: Smart Animate
Duration: 300ms
Easing: Ease Out
```

#### Join 버튼 클릭

```
Trigger: On Click
Action: Navigate to
Destination: Frame "MainGame Screen"
Animation: Smart Animate
Duration: 300ms
Easing: Ease Out
```

#### Disconnect 버튼 클릭

```
Trigger: On Click
Action: Navigate to
Destination: Frame "Lobby UI" (로비 화면)
Animation: Smart Animate
Duration: 300ms
Easing: Ease In
```

---

## 14. 접근성 (Accessibility)

### 14.1 색상 대비

```
WCAG 2.1 Level AA 준수:
  - 텍스트 대비: 4.5:1 이상
  - 큰 텍스트 대비: 3:1 이상

체크 도구:
  - Figma Plugin: Stark
  - Web: WebAIM Contrast Checker
```

### 14.2 폰트 크기

```
최소 폰트 크기: 14px
권장 본문 크기: 16px 이상
버튼 텍스트: 22px (충분히 큼)
```

### 14.3 터치 타겟

```
최소 터치 타겟 크기: 44px × 44px (Apple HIG)
로비 UI 버튼: 520px × 60px ✅ (충분히 큼)
```

---

## 15. 참고 자료

### 15.1 Figma 튜토리얼

- [Figma 공식 문서](https://help.figma.com/)
- [Auto Layout Guide](https://www.figma.com/best-practices/everything-you-need-to-know-about-layout-grids/)
- [Component Variants](https://help.figma.com/hc/en-us/articles/360056440594-Create-and-use-variants)

### 15.2 디자인 영감

- [Dribbble - Game UI](https://dribbble.com/tags/game-ui)
- [Behance - UI Design](https://www.behance.net/search/projects?search=game%20lobby%20ui)

### 15.3 한국 문화 디자인 참고

- 전통 문양: 구름 문양, 당초 문양
- 전통 색상: 오방색 (청, 적, 황, 백, 흑)
- 전통 건축: 한옥 처마, 기와 패턴

---

## 부록: Figma 단축키

```
필수 단축키:
  - Frame 생성: F
  - Rectangle: R
  - Text: T
  - Select: V (또는 Esc)
  - Hand Tool: H (또는 Space + 드래그)
  - Zoom In/Out: Ctrl + Plus/Minus
  - Auto Layout: Shift + A
  - Component 생성: Ctrl + Alt + K
  - Export: Ctrl + Shift + E
```

---

**Figma 디자인 가이드 끝**

이 가이드를 따라 YiSan 멀티플레이 로비 UI를 디자인하세요! 🎨
