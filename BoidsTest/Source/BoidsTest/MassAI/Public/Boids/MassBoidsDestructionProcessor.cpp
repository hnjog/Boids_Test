// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAI/Public/Boids/MassBoidsDestructionProcessor.h"
#include "MassBoidsHealthFragment.h" 
#include "MassCommonFragments.h"     
#include "MassExecutionContext.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UMassBoidsDestructionProcessor::UMassBoidsDestructionProcessor()
	:EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassBoidsDestructionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsDestructionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 데이터 뷰 가져오기
			TConstArrayView<FMassBoidsHealthFragment> Healths = Context.GetFragmentView<FMassBoidsHealthFragment>();
			TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				// 체력이 0 이하인지 확인
				if (Healths[i].Health <= 0.0f)
				{
					// 현재 엔티티 핸들 가져오기
					FMassEntityHandle Entity = Context.GetEntity(i);
					FVector DeathLocation = Transforms[i].GetTransform().GetLocation();

					// 1. 시각 효과 (Niagara)
					// Fire & Forget 방식: 스폰하고 잊어버림 (가벼움)
					if (ExplosionEffect)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(
							GetWorld(),
							ExplosionEffect,
							DeathLocation,
							FRotator::ZeroRotator,
							EffectScale,
							true, // Auto Destroy
							true, // Auto Activate
							ENCPoolMethod::AutoRelease // 풀링 사용
						);
					}

					// 2. 사운드 효과
					if (ExplosionSound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, DeathLocation);
					}

					// 3. 엔티티 삭제 요청 (Deferred Destroy)
					// 주의: 여기서 EntityManager.DestroyEntity()를 직접 호출하면 안 됩니다!
					// 루프 도중에 배열이 깨지기 때문입니다. Context를 통해 지연 삭제를 요청합니다.
					Context.Defer().DestroyEntity(Entity);
				}
			}
		});
}
