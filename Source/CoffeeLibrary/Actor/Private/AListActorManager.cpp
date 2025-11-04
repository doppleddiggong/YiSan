// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file AListActorManager.cpp
 * @brief AListActorManager의 동작을 구현합니다.
 */
#include "AListActorManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Engine/Selection.h"
#include "ScopedTransaction.h"
#include "Algo/Unique.h"
#endif

#if WITH_EDITOR
/** @brief 현재 에디터 선택에서 조건을 만족하는 액터를 수집한다. */
int32 AListActorManager::GatherSelectedItem(TArray<AActor*>& Out) const
{
	Out.Reset();
	if (!GEditor)
		return 0;

	TArray<AActor*> Selected;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);

	for (AActor* A : Selected)
	{
		if (!IsValid(A))
			continue;

		if (ClassFilter && !A->IsA(ClassFilter))
			continue;
		Out.Add(A);
	}

	// 일관성: 이름 정렬 + 중복 제거
	Out.Sort([](const AActor& L, const AActor& R){
		return L.GetFName().LexicalLess(R.GetFName());
	});

	Out.SetNum(Algo::Unique(Out));

	return Out.Num();
}

/** @brief 선택된 액터를 배열에 추가하거나 교체한다. */
void AListActorManager::AssignInternal(const bool bAppend)
{
	TArray<AActor*> Picked;
	const int32 Count = GatherSelectedItem(Picked);
	if (Count <= 0)
		return;

	const FScopedTransaction Tx(NSLOCTEXT("ListActorManager", "AssignFromSelection", "Assign Items From Selection"));
	Modify();

	if (!bAppend)
	{
		ArrayActors.Reset();
	}

	for (AActor* A : Picked)
	{
		if (!IsValid(A))
			continue;
		
		ArrayActors.AddUnique(A);
	}

	if (UPackage* Pkg = GetOutermost())
	{
		Pkg->SetDirtyFlag(true);
	}
}
#endif // WITH_EDITOR

/** @brief 배열을 선택 항목으로 대체한다. */
void AListActorManager::AssignItemReplace()
{
#if WITH_EDITOR
	AssignInternal(false);
#endif
}

/** @brief 선택 항목을 배열 끝에 추가한다. */
void AListActorManager::AssignItemAppend()
{
#if WITH_EDITOR
	AssignInternal(true);
#endif
}


/** @brief 액터 라벨을 기준으로 오름차순 정렬한다. */
void AListActorManager::SortByNameAsc()
{
#if WITH_EDITOR
	const FScopedTransaction Tx(
		NSLOCTEXT("ListActorManager", "SortByNameAsc", "Sort ArrayActors by Name Asc")
	);
	Modify();

	ArrayActors.Sort([](const AActor& L, const AActor& R) {
		return L.GetActorLabel() < R.GetActorLabel();
	});

	if (UPackage* Pkg = GetOutermost())
	{
		Pkg->SetDirtyFlag(true);
	}
#endif
}

/** @brief 액터 라벨을 기준으로 내림차순 정렬한다. */
void AListActorManager::SortByNameDesc()
{
#if WITH_EDITOR
	const FScopedTransaction Tx(
		NSLOCTEXT("ListActorManager", "SortByNameDesc", "Sort ArrayActors by Name Desc")
	);
	Modify();

	ArrayActors.Sort([](const AActor& L, const AActor& R) {
		return L.GetActorLabel() > R.GetActorLabel();
	});
	
	if (UPackage* Pkg = GetOutermost())
	{
		Pkg->SetDirtyFlag(true);
	}
#endif
}
