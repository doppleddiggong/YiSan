@echo off
echo ========================================
echo YiSan DDC Build Script (UE 5.6 호환)
echo ========================================
echo.

REM UE 5.6 경로 설정
set UE_PATH=C:\Program Files\Epic Games\UE_5.6
set PROJECT_PATH=%~dp0YiSan.uproject

echo Unreal Engine Path: %UE_PATH%
echo Project Path: %PROJECT_PATH%
echo.

REM 프로젝트 파일 존재 확인
if not exist "%PROJECT_PATH%" (
    echo [ERROR] 프로젝트 파일을 찾을 수 없습니다: %PROJECT_PATH%
    pause
    exit /b 1
)

REM UnrealEditor-Cmd.exe 존재 확인
if not exist "%UE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    echo [ERROR] UnrealEditor-Cmd.exe를 찾을 수 없습니다.
    pause
    exit /b 1
)

echo ========================================
echo [옵션] 기존 DDC 캐시 삭제하시겠습니까?
echo ========================================
echo 권장: 처음 실행이거나 오류가 있으면 Y
choice /C YN /M "DDC 캐시를 삭제하시겠습니까" /T 10 /D N
if %ERRORLEVEL% EQU 1 (
    echo.
    echo DDC 캐시 삭제 중...
    
    if exist "%~dp0DerivedDataCache" (
        rmdir /s /q "%~dp0DerivedDataCache"
        echo [OK] 로컬 DDC 삭제 완료
    )
    
    if exist "%~dp0Saved\Cooked" (
        rmdir /s /q "%~dp0Saved\Cooked"
        echo [OK] Cooked 데이터 삭제 완료
    )
    
    if exist "%~dp0Saved\ShaderDebugInfo" (
        rmdir /s /q "%~dp0Saved\ShaderDebugInfo"
        echo [OK] 셰이더 디버그 정보 삭제 완료
    )
    
    if exist "%~dp0Intermediate\ShaderAutogen" (
        rmdir /s /q "%~dp0Intermediate\ShaderAutogen"
        echo [OK] 셰이더 임시 파일 삭제 완료
    )
    
    echo [완료] DDC 캐시 정리 완료
    echo.
)

echo ========================================
echo [1단계] DDC 캐시 빌드 시작
echo ========================================
echo 참고: 이 과정은 10~30분 소요될 수 있습니다.
echo.

REM UE 5.6에서는 DerivedDataCache만 사용 (CompileAllShaders는 제거됨)
"%UE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "%PROJECT_PATH%" ^
    -run=DerivedDataCache ^
    -fill ^
    -targetplatform=Windows ^
    -unattended ^
    -nopause ^
    -NoLogTimes ^
    -UTF8Output

set DDC_RESULT=%ERRORLEVEL%

echo.
if %DDC_RESULT% EQU 0 (
    echo [OK] DDC 빌드 완료
) else (
    echo [경고] DDC 빌드 중 일부 오류 발생 (에러 코드: %DDC_RESULT%)
    echo 일부 셰이더는 런타임에 컴파일됩니다.
)
echo.

echo ========================================
echo [2단계] 특정 맵 Cook (선택)
echo ========================================
choice /C YN /M "Main_MapWP 맵을 사전 Cook 하시겠습니까" /T 10 /D N
if %ERRORLEVEL% EQU 1 (
    echo.
    echo Main_MapWP Cook 중...
    
    "%UE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
        "%PROJECT_PATH%" ^
        -run=Cook ^
        -Map=Main_MapWP ^
        -TargetPlatform=Windows ^
        -iterate ^
        -unversioned ^
        -unattended ^
        -nopause ^
        -UTF8Output
    
    if %ERRORLEVEL% EQU 0 (
        echo [OK] 맵 Cook 완료
    ) else (
        echo [경고] Cook 중 일부 오류 발생 (정상일 수 있음)
    )
)

echo.
echo ========================================
echo 빌드 프로세스 완료!
echo ========================================
echo.
echo 다음 단계:
echo 1. 에디터를 열고 Output Log 확인
echo 2. PIE 실행 시 텍스처 스트리밍 속도 확인
echo 3. 처음 실행 시 일부 셰이더가 컴파일될 수 있음 (정상)
echo.
echo 문제가 계속되면:
echo - Editor Preferences에서 "Force Compilation at Startup" 체크
echo - 프로젝트 설정에서 r.ShaderCompiler.SkipFailedShaders=1 확인
echo.
pause