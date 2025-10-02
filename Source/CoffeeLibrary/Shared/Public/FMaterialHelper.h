#pragma once

/**
 * @file FMaterialHelper.h
 * @brief 머티리얼 인스턴스와 안전하게 상호작용하기 위한 헬퍼를 제공합니다.
 */
struct FMaterialHelper
{
        /**
         * @brief 머티리얼 인스턴스에서 벡터 파라미터 값을 안전하게 조회합니다.
         * @param MaterialInstance 확인할 머티리얼 인스턴스입니다.
         * @param ParamName 읽어올 벡터 파라미터 이름입니다.
         * @return 값이 존재하면 해당 파라미터를, 인스턴스가 없으면 검정색을 반환합니다.
         */
        static FLinearColor GetVectorParameterValueSafe(
                UMaterialInstanceDynamic* MaterialInstance,
                const FName& ParamName)
	{
		if (MaterialInstance == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetVectorParameterValueSafe: MaterialInstance is null"));
			return FLinearColor::Black;
		}
		return MaterialInstance->K2_GetVectorParameterValue(ParamName);
	}
};