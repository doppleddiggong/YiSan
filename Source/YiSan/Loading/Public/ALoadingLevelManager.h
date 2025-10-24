#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "ALoadingLevelManager.generated.h"


UCLASS()
class YISAN_API ALoadingLevelManager : public AActor
{
	GENERATED_BODY()

public:
	ALoadingLevelManager();

	// game start 에서 버튼을 눌렀을때 traver 을 통해서 맵을 이동합니다
	// start map -> transtition map 으로 이동한뒤 UI 를 띄우고 -> MainMapWP 로 이동하는 구조입니다
	// transtitiion map 은 현재 loadingmap 으로 설정되어있으며 level blue print 에서 widget 을 호출 하는것 이외는 점유되는 오브젝트가 없습니다
	// 항상 loading map 은 가벼워야 합니다.	
	virtual void BeginPlay() override;
};