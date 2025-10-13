// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "NetworkTest/FApiSendInfo.h"
#include "Settings/UToolbarSettings.h"

// 기본 네트워크 설정
static const FString DefaultAccept = TEXT("application/json");
static const FString DefaultContentType = TEXT("application/json");

// 유틸: URL 합성 (절대 URL이면 그대로, 아니면 BaseUrl + Endpoint)
static FString BuildFullUrl(const FString& Input)
{
    if (Input.StartsWith(TEXT("http://")) || Input.StartsWith(TEXT("https://")))
    {
        return Input;
    }

    const UToolbarSettings* Settings = GetDefault<UToolbarSettings>();
    const FString Base = Settings ? Settings->NetworkTestUrl : TEXT("http://127.0.0.1:8000");

    if (Base.IsEmpty())
    {
        // 베이스가 비어있으면 입력을 그대로 사용
        return Input;
    }
    // 슬래시 중복 방지
    if (Base.EndsWith(TEXT("/")) && Input.StartsWith(TEXT("/")))
    {
        return Base.LeftChop(1) + Input;
    }
    if (!Base.EndsWith(TEXT("/")) && !Input.StartsWith(TEXT("/")))
    {
        return Base + TEXT("/") + Input;
    }
    return Base + Input;
}

// 유틸: JSON 문자열 → FString 바디 그대로 / TMap 변환(선택)
static bool ParseJsonToMap(const FString& JsonText, TMap<FString, FString>& OutMap)
{
    TSharedPtr<FJsonObject> Obj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
    {
        return false;
    }

    for (const auto& It : Obj->Values)
    {
        if (It.Value.IsValid())
        {
            // 단순 문자열만 TMap에 채움 (필요 시 숫자/불리언 등 추가 변환 가능)
            if (It.Value->Type == EJson::String)
            {
                OutMap.Add(It.Key, It.Value->AsString());
            }
            else
            {
                // 비문자형은 JSON으로 직렬화해 저장
                FString AsString;
                TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&AsString);
                FJsonSerializer::Serialize(It.Value, TEXT(""), W);
                OutMap.Add(It.Key, AsString);
            }
        }
    }
    return true;
}

// 콘솔 커맨드: curl <method> <url_or_endpoint> [json_body...]
static FAutoConsoleCommand GCurlCommand(
    TEXT("curl"),
    TEXT("Usage:\n  curl get <url_or_endpoint>\n  curl post <url_or_endpoint> {\"k\":\"v\"}"),
    FConsoleCommandWithArgsDelegate::CreateStatic([](const TArray<FString>& Args)
    {
        if (Args.Num() < 2)
        {
            UE_LOG(LogTemp, Warning, TEXT("[curl] Usage:\n  curl get <url_or_endpoint>\n  curl post <url_or_endpoint> {json}"));
            return;
        }

        // 1) 메서드
        const FString MethodLower = Args[0].ToLower();
        EApiHttpMethod Method = EApiHttpMethod::GET;
        if (MethodLower == TEXT("post"))
        {
            Method = EApiHttpMethod::POST;
        }
        else if (MethodLower != TEXT("get"))
        {
            UE_LOG(LogTemp, Warning, TEXT("[curl] Unsupported method: %s"), *Args[0]);
            return;
        }

        // 2) URL/엔드포인트
        const FString UrlOrEndpoint = Args[1];
        const FString FullUrl = BuildFullUrl(UrlOrEndpoint);

        // 3) JSON Body (POST일 때만)
        FString JsonBodyText;
        if (Method == EApiHttpMethod::POST)
        {
            if (Args.Num() < 3)
            {
                UE_LOG(LogTemp, Warning, TEXT("[curl] POST requires JSON body.\nExample: curl post /test/gpt {\"text\":\"안녕\"}"));
                return;
            }
            // Args[2..]를 공백으로 이어 붙여 원본 JSON을 복원
            TArray<FString> Tail;
            for (int32 i = 2; i < Args.Num(); ++i)
            {
                Tail.Add(Args[i]);
            }
            JsonBodyText = FString::Join(Tail, TEXT(" "));

            // 간단한 밸리데이션
            if (!JsonBodyText.StartsWith(TEXT("{")) || !JsonBodyText.EndsWith(TEXT("}")))
            {
                UE_LOG(LogTemp, Warning, TEXT("[curl] JSON body must start with '{' and end with '}'. Given: %s"), *JsonBodyText);
                return;
            }
        }

        // HTTP 요청 생성
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(FullUrl);
        Request->SetTimeout(60);

        // Accept 헤더
        Request->SetHeader(TEXT("Accept"), DefaultAccept);

        // 메서드별 설정
        if (Method == EApiHttpMethod::GET)
        {
            Request->SetVerb(TEXT("GET"));
        }
        else // POST
        {
            Request->SetVerb(TEXT("POST"));
            Request->SetHeader(TEXT("Content-Type"), DefaultContentType);
            Request->SetContentAsString(JsonBodyText);
        }

        // 응답 처리
        Request->OnProcessRequestComplete().BindLambda(
            [](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
            {
                if (bSuccess && Res.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("[curl] %s %s -> %d\n[RES] %s"),
                        *Req->GetVerb(),
                        *Req->GetURL(),
                        Res->GetResponseCode(),
                        *Res->GetContentAsString());
                }
                else
                {
                    const int32 Code = (Res.IsValid() ? Res->GetResponseCode() : -1);
                    UE_LOG(LogTemp, Error, TEXT("[curl] Request failed. Code=%d URL=%s"),
                        Code, Req.IsValid() ? *Req->GetURL() : TEXT("<null>"));
                }
            }
        );

        // 요청 로그 출력
        if (Method == EApiHttpMethod::POST && !JsonBodyText.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("[curl] POST %s\n[BODY] %s"),
                *FullUrl,
                *JsonBodyText);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[curl] %s %s"),
                (Method == EApiHttpMethod::GET ? TEXT("GET") : TEXT("POST")),
                *FullUrl);
        }

        Request->ProcessRequest();
    })
);
