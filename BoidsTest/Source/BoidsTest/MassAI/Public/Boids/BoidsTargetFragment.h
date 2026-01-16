// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "BoidsTargetFragment.generated.h"

/**
 * BOIDSTEST_API
 */
USTRUCT()
struct BOIDSTEST_API FBoidsTargetFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    FVector TargetPosition = FVector(0.0f, 0.0f, 3000.0f);
    bool IsTargetChase = true;
};