// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBoidsProcesser.generated.h"

/**
 * 
 */
UCLASS()
class BOIDSTEST_API UMassBoidsProcesser : public UMassProcessor
{
	GENERATED_BODY()
public:
    UMassBoidsProcesser();

protected:
    // 이거 사용해야 함 (인자 없는 버전은 5.6 에서 폐기 처리(final))
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

    // 실제 매 프레임(Tick) 돌아가는 로직
    // - Entity이기에 Actor보다 부담은 적은 편
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // 사용할 쿼리 객체
    FMassEntityQuery EntityQuery;
};
