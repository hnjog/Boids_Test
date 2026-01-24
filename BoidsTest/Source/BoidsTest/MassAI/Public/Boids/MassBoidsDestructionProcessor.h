// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBoidsDestructionProcessor.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class BOIDSTEST_API UMassBoidsDestructionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassBoidsDestructionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	// 쿼리 객체
	FMassEntityQuery EntityQuery;

public:
	// 에디터에서 할당할 폭발 이펙트 (Niagara)
	UPROPERTY(EditAnywhere, Category = "FX")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	// 에디터에서 할당할 폭발 사운드
	UPROPERTY(EditAnywhere, Category = "FX")
	TObjectPtr<USoundBase> ExplosionSound;

	// 폭발 스케일 조정 (선택 사항)
	UPROPERTY(EditAnywhere, Category = "FX")
	FVector EffectScale = FVector(1.0f);
};