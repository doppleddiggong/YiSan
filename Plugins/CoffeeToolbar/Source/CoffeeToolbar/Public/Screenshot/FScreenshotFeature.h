#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h" // Not strictly needed here, but good practice for UI features

class FScreenshotFeature
{
public:
    FScreenshotFeature();

    void OnCaptureScreenshot();
    void OnOpenScreenShotDir();
};