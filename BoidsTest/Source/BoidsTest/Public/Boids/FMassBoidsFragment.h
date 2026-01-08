// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMassBoidsFragment.generated.h"

/**
 * BOIDSTEST_API
 */
USTRUCT()
struct BOIDSTEST_API FMassBoidsFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    // Boids 계산에 필요한 파라미터 (설정값)
    UPROPERTY(EditAnywhere)
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float AlignmentWeight = 0.5f;

    UPROPERTY(EditAnywhere)
    float CohesionWeight = 0.5f;

    // 주변 이웃을 감지할 반경
    UPROPERTY(EditAnywhere)
    float DetectionRadius = 500.0f;
};