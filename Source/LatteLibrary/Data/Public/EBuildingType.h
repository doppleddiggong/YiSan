#pragma once
/**
 * @file EBuildingType.h
 * @brief EBuildingType 클래스를 선언합니다.
 */

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	None,
	
	Yeomingak      		UMETA(DisplayName = "여민각"),
	Bongsudang     		UMETA(DisplayName = "봉수당"),
	Naknamhyeon    		UMETA(DisplayName = "낙남헌"),
	Yuyotaek       		UMETA(DisplayName = "유여택"),
	Jwaikmun      		UMETA(DisplayName = "좌익문"),
	Uhwagwan      		UMETA(DisplayName = "우화관"),
	Byeolju       		UMETA(DisplayName = "별주"),
	Bijangcheol   		UMETA(DisplayName = "비장청"),
	Seoricheong   		UMETA(DisplayName = "서리청"),
	Namgunyeong   		UMETA(DisplayName = "남군영"),
	Sinpungnu     		UMETA(DisplayName = "신풍루"),
	Jibsacheong   		UMETA(DisplayName = "집사청"),
	Bukgunyeong   		UMETA(DisplayName = "북군영"),
	Eojeong       		UMETA(DisplayName = "어정"),
	Iancheong     		UMETA(DisplayName = "이안청"),
	Hyangdaecheong		UMETA(DisplayName = "행대청"),
	Unhangak			UMETA(DisplayName = "운한각"),
	Jaesil				UMETA(DisplayName = "재실"),

	Max				UMETA(Hidden)
};