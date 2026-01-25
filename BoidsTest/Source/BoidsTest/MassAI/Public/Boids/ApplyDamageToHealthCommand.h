#pragma once

#include "CoreMinimal.h"
#include "MassCommands.h"        // FMassBatchedCommand
#include "MassEntityTypes.h"     // FMassEntityHandle

class FApplyDamageToHealthCommand final : public FMassBatchedCommand
{
public:
	FApplyDamageToHealthCommand();
	FApplyDamageToHealthCommand(const FMassEntityHandle InEntity, const float InDamage);

	virtual void Run(FMassEntityManager& EntityManager) override;

private:
	FMassEntityHandle Entity;
	float Damage = 0.f;
};
