// Fill out your copyright notice in the Description page of Project Settings.


#include "Boids/MassBoidsProcesser.h"
#include "MassCommonFragments.h"		// Tranform Fragment
#include "MassMovementFragments.h"		// VelocityFragment
#include "MassExecutionContext.h"
#include "Boids/MassBoidsFragment.h"
#include "Boids/BoidsTargetFragment.h"

UMassBoidsProcesser::UMassBoidsProcesser()
{
	// 실행 순서 결정 - 이동 로직이므로 물리 엔진 전 (or 후)
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMassBoidsProcesser::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 쿼리 설정 등록 (5.6 기준 - 해당 매개변수 처리 필요)
	// 다른 버전이라면 매개변수 없는 함수 존재
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBoidsTargetFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.AddSharedRequirement<FMassBoidsFragment>(EMassFragmentAccess::ReadOnly);

	// 쿼리 등록
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsProcesser::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Super가 딱히 의미가 없을듯?

	// Entity 순회하며 로직 순회
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
		{
			// 데이터 배열 가져오기 (성능을 위해 청크 단위로)
			const int32 NumEntities = Context.GetNumEntities();

			// 개별 데이터는 배열(ArrayView)로 나옵니다.
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMassVelocityFragment> Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();
			TConstArrayView<FBoidsTargetFragment> TargetInfos = Context.GetFragmentView<FBoidsTargetFragment>();

			// 단일 용도로 가져옴 (어차피 다 같은 내용)
			const FMassBoidsFragment& Settings = Context.GetSharedFragment<FMassBoidsFragment>();

			// DeltaTime
			const float DT = Context.GetDeltaTimeSeconds();

			// Boids 로직 루프
			for (int32 i = 0; i < NumEntities; ++i)
			{
				// GetMutableTransform 가 사실 Transform 수정과 동일...
				FTransform& Transform = Transforms[i].GetMutableTransform();
				FVector& Velocity = Velocities[i].Value;
				const FBoidsTargetFragment& TargetInfo = TargetInfos[i];

				FVector CurrentPos = Transform.GetLocation();

				FVector Acceleration = FVector::ZeroVector;

				// 타겟이 있으면 그쪽으로 조향
				if (TargetInfo.IsTargetChase == true)
				{
					FVector DirToTarget = TargetInfo.TargetPosition - CurrentPos; // Offset
					Acceleration += SteerTowards(TargetInfo.TargetPosition, CurrentPos, Velocity, Settings) * Settings.TargetWeight;
				}

				// TODO : Boids

				// TODO : Avoid

				Velocity += Acceleration * DT;

				float Speed = Velocity.Size();
				if (FMath::IsNearlyZero(Speed) == false)
				{
					float NewSpeed = FMath::Clamp(Speed, Settings.MinMoveSpeed, Settings.MaxMoveSpeed);
					Velocity = (Velocity / Speed) * NewSpeed;
				}

				Transform.AddToTranslation(Velocity * DT);

				// 진행 방향 바라보기
				if (Velocity.IsNearlyZero() == false)
				{
					Transform.SetRotation(Velocity.ToOrientationQuat());
				}
			}
		});
}

FVector UMassBoidsProcesser::SteerTowards(const FVector& TargetLoc, const FVector& CurrentLoc, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const
{
	FVector VectorToTarget = TargetLoc - CurrentLoc;
	FVector DesiredVelocity = VectorToTarget.GetSafeNormal() * Settings.MaxMoveSpeed;

	FVector Steer = DesiredVelocity - CurrentVel;

	return Steer.GetClampedToMaxSize(Settings.MaxSteerWeight);
}
