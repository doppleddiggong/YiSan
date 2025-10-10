// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

/**
 * 언리얼에 없는 FHttpMultipartFormData를 가볍게 대체하는 클래스입니다.
 * - 경계(boundary)를 자동으로 생성합니다.
 * - AddFile / AddText를 지원합니다.
 * - RFC 7578을 준수하는 multipart/form-data 본문을 빌드합니다.
 */
class FHttpMultipartFormData
{
public:
	FHttpMultipartFormData();

	/** 간단한 텍스트 필드를 추가합니다. */
	void AddText(const FString& FieldName, const FString& Value);

	/**
	 * 파일 필드를 추가합니다.
	 * @param FieldName  폼 필드 이름 (예: "file")
	 * @param FilePath   절대 또는 상대 파일 경로
	 * @param MimeType   예: "audio/mpeg", "audio/wav" (비어있을 경우 자동으로 감지)
	 * @return 파일이 로드되어 추가되었으면 true를 반환합니다.
	 */
	bool AddFile(const FString& FieldName, const FString& FilePath, const FString& MimeType = TEXT(""));

	/** 모든 파트를 지우고 경계를 다시 생성합니다. */
	void Reset();

	/**
	 * 주어진 리퀘스트에 헤더와 본문을 빌드하고 첨부합니다.
	 * 설정: Content-Type: multipart/form-data; boundary=...
	 */
	void SetupHttpRequest(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request);

	/** 필요할 경우를 위한 접근자입니다. */
	const FString& GetBoundary() const { return Boundary; }
	const TArray<uint8>& GetBuiltBody() const { return BuiltBody; }

private:
	struct FTextPart
	{
		FString Name;
		FString Value;
	};

	struct FFilePart
	{
		FString Name;
		FString FileName;
		FString MimeType;
		TArray<uint8> Data;
	};

private:
	static FString DetectMimeFromExtension(const FString& FileName);

	static void AppendUtf8(TArray<uint8>& Dest, const FString& Str);
	void BuildBody(); // BuiltBody를 채웁니다.

private:
	FString Boundary;
	TArray<FTextPart> TextParts;
	TArray<FFilePart> FileParts;
	TArray<uint8> BuiltBody;
};
