// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Styling/SlateStyle.h"

/**
 * @file FToolbarStyle.h
 * @brief Coffee Toolbar 플러그인이 사용하는 Slate 스타일 세트를 선언합니다.
 */

/**
 * @brief 툴바 위젯에 사용할 Slate 브러시 리소스를 등록하고 제공합니다.
 */
class FToolbarStyle
{
public:
        /** @brief 스타일 세트를 Slate 스타일 레지스트리에 등록합니다. */
        static void Initialize();

        /** @brief 스타일 세트를 제거하고 참조된 리소스를 해제합니다. */
        static void Shutdown();

        /** @brief 디스크상의 리소스가 변경된 경우 텍스처를 다시 로드합니다. */
        static void ReloadTextures();

        /** @brief 브러시 조회에 사용할 Slate 스타일 인스턴스를 반환합니다. */
        static const ISlateStyle& Get();

        /** @brief Slate 스타일 세트를 참조할 때 사용할 이름을 반환합니다. */
        static FName GetStyleSetName();

private:
        /** @brief 기본 이미지와 아이콘 등록을 포함한 스타일 세트를 생성합니다. */
        static TSharedRef<class FSlateStyleSet> Create();

private:
        /** @brief 등록된 Slate 스타일을 보관하는 싱글턴 인스턴스입니다. */
        static TSharedPtr<class FSlateStyleSet> Instance;
};