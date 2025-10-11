#pragma once

#include "CoreMinimal.h"
#include "FApiSendInfo.generated.h"

/**
 * @file FApiSendInfo.h
 * @brief HTTP API 요청 정보를 정의합니다.
 */

UENUM(BlueprintType)
enum class EApiHttpMethod : uint8
{
	GET  UMETA(DisplayName="GET"),
	POST UMETA(DisplayName="POST")
};

USTRUCT(BlueprintType)
struct FApiSendInfo
{
	GENERATED_BODY()

public:
	/** 드롭다운 메뉴에 표시될 API 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="API Info")
	FString Label;

	/** GET / POST */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="API Info")
	EApiHttpMethod Method = EApiHttpMethod::GET;

	/** 엔드포인트 (예: /health, /test/gpt) 또는 절대 URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="API Info")
	FString Endpoint;

	/** POST일 때만 활성 (콘솔/에디터에서 JSON으로부터 채워질 수 있음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="API Info",
		meta=(EditCondition="Method == EApiHttpMethod::POST", EditConditionHides))
	TMap<FString, FString> BodyParams;
};
