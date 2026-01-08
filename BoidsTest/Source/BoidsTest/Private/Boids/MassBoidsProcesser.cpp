// Fill out your copyright notice in the Description page of Project Settings.


#include "Boids/MassBoidsProcesser.h"
#include "MassCommonFragments.h"		// Tranform Fragment
#include "MassMovementFragments.h"		// VelocityFragment
#include "MassExecutionContext.h"

UMassBoidsProcesser::UMassBoidsProcesser()
{
	// 실행 순서 결정 - 이동 로직이므로 물리 엔진 전 (or 후)
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// 쿼리 설정을 생성자에서 등록 (5.6 기준?)
	// 다른 버전이라면 관련 함수가 존재
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

	// 쿼리 등록
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsProcesser::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Super가 딱히 의미가 없을듯?

	// Entity 순회하며 로직 순회
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
        {
            // 데이터 배열 가져오기 (성능을 위해 청크 단위로 가져옵니다)
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            TArrayView<FMassVelocityFragment> Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();

            // Boids 로직 루프
            for (int32 i = 0; i < NumEntities; ++i)
            {
                // GetMutableTransform 가 사실 Transform 수정과 동일...
                FTransform& Transform = Transforms[i].GetMutableTransform();
                FVector& Velocity = Velocities[i].Value;

                // Boids
                // 예: Velocity += ...;
                //     Transform.AddToTranslation(Velocity * Context.GetDeltaTime());
            }
        });
}
