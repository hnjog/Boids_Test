// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassBoidsFragment.generated.h"

/**
 * BOIDSTEST_API
 */
USTRUCT()
struct BOIDSTEST_API FMassBoidsFragment : public FMassSharedFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    float MinMoveSpeed = 100.0f;

    UPROPERTY(EditAnywhere)
    float MaxMoveSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationSpeed = 5.0f;
    
    // 주변 이웃을 감지할 반경
    UPROPERTY(EditAnywhere)
    float DetectionRadius = 500.0f;

    // 주변 이웃 피하는 반경
    UPROPERTY(EditAnywhere)
    float AvoidRadius = 150.0f;

    // 계산 중, 최댓값에 대한 제한용도
    UPROPERTY(EditAnywhere)
    float MaxSteerWeight = 50.0f;

    // 장애물 관련 확인 Dist
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float ObstacleCheckDistance = 500.0f;

    // 장애물 피하는 우선순위
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float ObstacleAvoidanceWeight = 10.0f;

    // 장애물 CollisionType
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    TEnumAsByte<ECollisionChannel> ObstacleTraceChannel = ECC_WorldStatic;


    // 무작위 방향 이동을 위한 변수들
    UPROPERTY(EditAnywhere, Category = "Wander")
    float WanderWeight = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Wander")
    float WanderJitter = 100.0f;

public:
    // Boids 계산에 필요한 파라미터 (설정값)
    UPROPERTY(EditAnywhere)
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float TargetWeight = 1.0f;
};