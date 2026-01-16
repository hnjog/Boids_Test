// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBoidsProcesser.generated.h"

struct FMassBoidsFragment;
struct FTransformFragment;
struct FMassVelocityFragment;

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
    FVector ComputeSeparation(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    FVector ComputeAlignment(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    FVector ComputeCohesion(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    
    FVector ComputeWander(const FVector& MyVel, const FMassBoidsFragment& Settings, float DT) const;

    FVector SteerTowards(const FVector& TargetLoc, const FVector& CurrentLoc, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const;

private:
    // 사용할 쿼리 객체
    FMassEntityQuery EntityQuery;
};
