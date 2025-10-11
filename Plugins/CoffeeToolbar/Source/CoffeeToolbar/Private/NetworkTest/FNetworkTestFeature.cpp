#include "NetworkTest/FNetworkTestFeature.h"
#include "Settings/UToolbarSettings.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "FNetworkTestFeature"

DEFINE_LOG_CATEGORY_STATIC(LogCoffeeToolbarNetworkTest, Log, All);

// 유틸: URL 합성 (절대 URL이면 그대로, 아니면 BaseUrl + Endpoint)
static FString BuildFullUrl(const FString BaseUrl, const FString& Input)
{
	if (Input.StartsWith(TEXT("http://")) || Input.StartsWith(TEXT("https://")))
	{
		return Input;
	}

	if (BaseUrl.IsEmpty())
	{
		return Input;
	}

	// 슬래시 중복 방지
	if (BaseUrl.EndsWith(TEXT("/")) && Input.StartsWith(TEXT("/")))
	{
		return BaseUrl.LeftChop(1) + Input;
	}
	if (!BaseUrl.EndsWith(TEXT("/")) && !Input.StartsWith(TEXT("/")))
	{
		return BaseUrl + TEXT("/") + Input;
	}
	return BaseUrl + Input;
}

TSharedRef<SWidget> FNetworkTestFeature::GenerateNetworkTestMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();

	if (Settings && !Settings->NetworkTests.IsEmpty())
	{
		for (int32 i = 0; i < Settings->NetworkTests.Num(); ++i)
		{
			const FApiSendInfo& ApiInfo = Settings->NetworkTests[i];

			if (ApiInfo.Endpoint.IsEmpty())
			{
				continue;
			}

			FUIAction Action(FExecuteAction::CreateRaw(this, &FNetworkTestFeature::ExecuteApiRequest, i));

			const FString DisplayLabel = ApiInfo.Label.IsEmpty()
				? FString::Printf(TEXT("%s %s"),
					ApiInfo.Method == EApiHttpMethod::GET ? TEXT("GET") : TEXT("POST"),
					*ApiInfo.Endpoint)
				: ApiInfo.Label;

			MenuBuilder.AddMenuEntry(
				FText::FromString(DisplayLabel),
				FText::FromString(FString::Printf(TEXT("Execute %s %s"),
					ApiInfo.Method == EApiHttpMethod::GET ? TEXT("GET") : TEXT("POST"),
					*ApiInfo.Endpoint)),
				FSlateIcon(),
				Action
			);
		}
	}
	else
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("NoApisConfigured", "No APIs configured"),
			LOCTEXT("NoApisConfigured_Tooltip", "Add API tests in Project Settings > Plugins > Level Selector > Network Test"),
			FSlateIcon(),
			FUIAction()
		);
	}

	return MenuBuilder.MakeWidget();
}

void FNetworkTestFeature::ExecuteApiRequest(int32 ApiIndex)
{
	const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
	if (!Settings || ApiIndex < 0 || ApiIndex >= Settings->NetworkTests.Num())
	{
		UE_LOG(LogCoffeeToolbarNetworkTest, Error, TEXT("Invalid API index: %d"), ApiIndex);
		return;
	}

	const FString BaseUrl = Settings ? Settings->NetworkTestUrl : TEXT("http://127.0.0.1:8000");
	const FApiSendInfo& ApiInfo = Settings->NetworkTests[ApiIndex];
	const FString FullUrl = BuildFullUrl(BaseUrl, ApiInfo.Endpoint);

	// HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FullUrl);
	Request->SetTimeout(60);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	// POST 바디 저장용
	FString JsonBodyString;

	// 메서드별 설정
	if (ApiInfo.Method == EApiHttpMethod::GET)
	{
		Request->SetVerb(TEXT("GET"));
	}
	else // POST
	{
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		// BodyParams를 JSON으로 변환
		if (ApiInfo.BodyParams.Num() > 0)
		{
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
			for (const auto& Pair : ApiInfo.BodyParams)
			{
				JsonObject->SetStringField(Pair.Key, Pair.Value);
			}

			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBodyString);
			FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

			Request->SetContentAsString(JsonBodyString);
		}
	}

	// 응답 처리
	Request->OnProcessRequestComplete().BindLambda(
		[ApiInfo](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
		{
			if (bSuccess && Res.IsValid())
			{
				UE_LOG(LogCoffeeToolbarNetworkTest, Log,
					TEXT("[Network Test] %s %s -> %d\n[RES] %s"),
					*Req->GetVerb(),
					*Req->GetURL(),
					Res->GetResponseCode(),
					*Res->GetContentAsString());
			}
			else
			{
				const int32 Code = (Res.IsValid() ? Res->GetResponseCode() : -1);
				UE_LOG(LogCoffeeToolbarNetworkTest, Error,
					TEXT("[Network Test] Request failed. Code=%d URL=%s"),
					Code, Req.IsValid() ? *Req->GetURL() : TEXT("<null>"));
			}
		}
	);

	// 요청 로그 출력
	if (ApiInfo.Method == EApiHttpMethod::POST && !JsonBodyString.IsEmpty())
	{
		UE_LOG(LogCoffeeToolbarNetworkTest, Log,
			TEXT("[Network Test] POST %s\n[BODY] %s"),
			*FullUrl,
			*JsonBodyString);
	}
	else
	{
		UE_LOG(LogCoffeeToolbarNetworkTest, Log,
			TEXT("[Network Test] %s %s"),
			ApiInfo.Method == EApiHttpMethod::GET ? TEXT("GET") : TEXT("POST"),
			*FullUrl);
	}

	Request->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
