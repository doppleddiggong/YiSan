// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UStatSystem.cpp
 * @brief UStatSystem의 동작을 구현합니다.
 */
#include "UStatSystem.h"

#include "FCharacterInfoData.h"
#include "UGameDataManager.h"

#define MIN_DMG_MUL 0.85f
#define MAX_DMG_MUL 1.15f

UStatSystem::UStatSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UStatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStatSystem::InitStat_Implementation(const bool InIsPlayer, const ECharacterType InCharacterType)
{
	this->bIsPlayer = InIsPlayer;
	this->CharacterType = InCharacterType;
	this->bIsDead = false;

	FCharacterInfoData Params;
	if ( UGameDataManager::Get(GetWorld())->GetCharacterInfoData(CharacterType, Params) )
	{
		this->MaxHP = Params.HP;
		this->CurHP = this->MaxHP;

		this->AttackDamage = Params.ATK;

		this->SightLength= Params.SightLength;
		this->SightAngle = Params.SightAngle;
	}
}

float UStatSystem::GetRandDmg(float Damage)
{
	return FMath::RandRange( Damage * MIN_DMG_MUL, Damage * MAX_DMG_MUL);
}

float UStatSystem::GetAttackDamage()
{
	return AttackDamage;
}	


void UStatSystem::IncreaseHealth_Implementation(float InHealPoint)
{
	this->CurHP += InHealPoint;

	if( CurHP > MaxHP )
		CurHP = MaxHP;
}

bool UStatSystem::DecreaseHealth_Implementation(float InDamagePoint)
{
	this->CurHP -= InDamagePoint;

	if( CurHP < 0 )
	{
		CurHP = 0;
		this->bIsDead = true;
	}

	return bIsDead;
}