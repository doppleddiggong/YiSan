// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkLog.cpp
 * @brief FDateTime의 동작을 구현합니다.
 */
#include "NetworkLog.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

// 로그 카테고리를 실제로 정의합니다.
DEFINE_LOG_CATEGORY(LogCoffeeNetwork);

// 로그 파일의 전체 경로를 저장하는 정적 변수입니다.
static FString GLogFilePath;

/**
 * @brief 로그 파일의 경로를 초기화하고 첫 로그 메시지를 기록합니다.
 */
void InitializeLogFile()
{
    if (GLogFilePath.IsEmpty())
    {
        FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
        FString LogDirectory = FPaths::ProjectSavedDir() / TEXT("NetworkLogs");
        IFileManager::Get().MakeDirectory(*LogDirectory, true);
        GLogFilePath = LogDirectory / FString::Printf(TEXT("NetworkLog_%s.log"), *Timestamp);

        const FString StartMessage = FString::Printf(TEXT("--- Log session started at %s ---"), *FDateTime::Now().ToString());
        FFileHelper::SaveStringToFile(StartMessage + TEXT("\n"), *GLogFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Append);
    }
}

/**
 * @brief 주어진 메시지를 로그 파일에 기록합니다.
 * @param Message 파일에 쓸 로그 메시지입니다.
 */
void WriteToGameLog(const FString& Message)
{
    // 로그 파일 경로가 설정되지 않았다면 초기화합니다.
    if (GLogFilePath.IsEmpty())
    {
        InitializeLogFile();
    }

    // 현재 시간을 포함한 전체 로그 라인을 구성합니다.
    FString LogLine = FString::Printf(TEXT("[%s] %s\n"), *FDateTime::Now().ToString(TEXT("%H:%M:%S")), *Message);

    // 파일에 로그를 추가합니다.
    FFileHelper::SaveStringToFile(LogLine, *GLogFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Append);
}
