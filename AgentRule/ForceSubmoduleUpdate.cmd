@echo off
setlocal ENABLEDELAYEDEXPANSION

REM ==== 옵션 ====
REM 1) 특정 브랜치로 강제 동기화하려면 인자로 브랜치명 전달: ForceSubmoduleUpdate.cmd main
set "TARGET_BRANCH=%~1"

REM ==== 사전 체크 ====
git --version >NUL 2>&1 || (echo [ERROR] Git not found.& exit /b 1)
git rev-parse --is-inside-work-tree >NUL 2>&1 || (echo [ERROR] Not a git repo.& exit /b 1)

echo [INFO] Sync .gitmodules urls...
git submodule sync --recursive || goto :fail

echo [INFO] Init/Update submodules (ensure present)...
git submodule update --init --recursive || goto :fail

if not "%TARGET_BRANCH%"=="" (
  echo [INFO] Force reset all submodules to origin/%TARGET_BRANCH% ...
  REM foreach는 Git for Windows의 sh를 통해 && 체인이 동작합니다.
  git submodule foreach --recursive "git fetch origin && git checkout -B %TARGET_BRANCH% origin/%TARGET_BRANCH% && git reset --hard origin/%TARGET_BRANCH%" || goto :fail
) else (
  echo [INFO] Update submodules to their tracked remote branches...
  git submodule update --remote --recursive || goto :fail
)

echo [INFO] Stage submodule pointer changes...
git add . || goto :fail

echo [INFO] Check if there is anything to commit...
git diff --cached --quiet && (
  echo [INFO] No changes detected. Done.
  goto :eof
)

for /f "tokens=1-3 delims=/ " %%a in ('date /t') do set TODAY=%%a %%b %%c
for /f "tokens=1" %%a in ('time /t') do set NOW=%%a

set "MSG=chore(submodule): bump all"
if not "%TARGET_BRANCH%"=="" set "MSG=chore(submodule): bump all -> origin/%TARGET_BRANCH%"
echo [INFO] Commit: %MSG% ^| %TODAY% %NOW%
git commit -m "%MSG%" || goto :fail

echo [INFO] Push superproject...
git push || goto :fail

echo [INFO] Done.
goto :eof

:fail
echo [ERROR] Failed. See logs above.
exit /b 1
