@echo off
setlocal enabledelayedexpansion
cd /d %~dp0

:: 1. Описание проекта
if exist project_name.txt (
    set /p PROJECT_DESC=<project_name.txt
) else (
    set "PROJECT_DESC=VTE General Series"
)

:: 2. Получаем версию с пометкой, если есть незакоммиченные правки
for /f "tokens=*" %%i in ('git describe --tags --always --dirty^=-M') do set "GIT_VERSION=%%i"

:: 3. Дата коммита (пересборка из YYYY-MM-DD в DD.MM.YYYY)
for /f "tokens=1,2" %%a in ('git log -1 --format^=%%ai') do (
    set "RAW_DATE=%%a"
    set "RAW_TIME=%%b"
)
if NOT "%RAW_DATE%"=="" (
    set "YYYY=!RAW_DATE:~0,4!"
    set "MM=!RAW_DATE:~5,2!"
    set "DD=!RAW_DATE:~8,2!"
    set "GIT_COMMIT_DATE=!DD!.!MM!.!YYYY! !RAW_TIME!"
) else (
    set "GIT_COMMIT_DATE=unknown"
)

:: 4. Время сборки без миллисекунд
set "CURRENT_DATE=%date%"
set "CURRENT_TIME=%time%"
:: Берем только первые 8 символов времени (ЧЧ:ММ:СС)
set "BUILD_TIME_SHORT=!CURRENT_TIME:~0,8!"
set "BUILD_DATE=!CURRENT_DATE! !BUILD_TIME_SHORT!"

:: 5. Заглушка для версии
if "%GIT_VERSION%"=="" set "GIT_VERSION=unknown"

:: 6. Формируем короткую версию
for /f "tokens=1-2 delims=-" %%a in ("%GIT_VERSION%") do (
    if "%%b"=="" (
        set "SHORT_VER=%%a"
    ) else (
        set "SHORT_VER=%%a-%%b"
    )
)

:: 7. Если есть "-M", добавляем его
if not "%GIT_VERSION%"=="%GIT_VERSION:-M=%" (
    set "SHORT_VER=!SHORT_VER!-M"
)

:: 8. Генерация файла
(
    echo #pragma once
    echo namespace BuildInfo {
    echo     inline constexpr const char* Description = "%PROJECT_DESC%";
    echo     inline constexpr const char* Version     = "%SHORT_VER%";
    echo     inline constexpr const char* CommitDate  = "%GIT_COMMIT_DATE%";
    echo     inline constexpr const char* BuildDate   = "%BUILD_DATE%";
    echo }
) > version.hpp

echo [SUCCESS] version.hpp ready.
echo CommitDate: %GIT_COMMIT_DATE%
echo BuildDate:  %BUILD_DATE%
echo GitVersion: %SHORT_VER%

