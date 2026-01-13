// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassBoidsFragment.h"
#include "MassBoidsTrait.generated.h"

/**
 * 
 */
UCLASS()
class BOIDSTEST_API UMassBoidsTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	// Pure라서 내부 구현해야 함
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
