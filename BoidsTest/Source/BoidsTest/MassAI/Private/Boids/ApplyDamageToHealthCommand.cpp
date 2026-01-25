#include "MassAI/Public/Boids/ApplyDamageToHealthCommand.h"

#include "MassEntityManager.h"   // FMassEntityManager (명시적으로 포함하는 게 안전)
#include "MassEntityView.h"      // FMassEntityView
#include "MassAI/Public/Boids/MassBoidsHealthFragment.h"

FApplyDamageToHealthCommand::FApplyDamageToHealthCommand()
	: FMassBatchedCommand(EMassCommandOperationType::Set, FName(TEXT("ApplyDamageToHealth")))
{
}

FApplyDamageToHealthCommand::FApplyDamageToHealthCommand(const FMassEntityHandle InEntity, const float InDamage)
	: FMassBatchedCommand(EMassCommandOperationType::Set, FName(TEXT("ApplyDamageToHealth")))
	, Entity(InEntity)
	, Damage(InDamage)
{
}

void FApplyDamageToHealthCommand::Run(FMassEntityManager& EntityManager)
{
	if (!EntityManager.IsEntityValid(Entity))
	{
		return;
	}

	if (FMassEntityView View = FMassEntityView::TryMakeView(EntityManager, Entity); View.IsValid())
	{
		if (FMassBoidsHealthFragment* HealthFrag = View.GetFragmentDataPtr<FMassBoidsHealthFragment>())
		{
			HealthFrag->Health = FMath::Clamp(HealthFrag->Health - Damage, 0.f, HealthFrag->MaxHealth);
		}
	}
}