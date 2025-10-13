#pragma once

#include "CoreMinimal.h"

class FFolderFeature
{
public:
    TSharedRef<class SWidget> GenerateFolderMenu();

private:
    void OpenFolder(FString Path);
};