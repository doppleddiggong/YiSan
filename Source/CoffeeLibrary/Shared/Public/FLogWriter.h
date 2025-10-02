/**
 * @file FLogWriter.h
 * @brief FLogWriter 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"

class FArchive;

class COFFEELIBRARY_API FLogWriter
{
public:
    FLogWriter();
    ~FLogWriter();

    void WriteLog(const FString& Message);

    static FLogWriter& Get();

private:
    FArchive* LogFile = nullptr;
};
