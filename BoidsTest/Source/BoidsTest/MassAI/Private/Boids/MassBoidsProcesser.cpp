// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAI/Public/Boids/MassBoidsProcesser.h"
#include "MassCommonFragments.h"		// Tranform Fragment
#include "MassMovementFragments.h"		// VelocityFragment
#include "MassExecutionContext.h"
#include "MassAI/Public/Boids/MassBoidsFragment.h"
#include "MassAI/Public/Boids/BoidsTargetFragment.h"

UMassBoidsProcesser::UMassBoidsProcesser()
	:EntityQuery(*this)
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
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
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
				if (TargetInfo.IsTargetChase)
				{
					FVector OffsetToTarget = TargetInfo.TargetPosition - CurrentPos;
					FVector SteerForce = SteerTowards(OffsetToTarget, Velocity, Settings);
					Acceleration += SteerForce * Settings.TargetWeight;
				}
				//else
				//{
				//	// 타겟이 없을 때만 Wander(배회) 작동
				//	FVector WanderForce = ComputeWander(Velocity, Settings, DT);
				//	Acceleration += WanderForce * Settings.WanderWeight;
				//}

				FVector SepForce = ComputeSeparation(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += SepForce * Settings.SeparationWeight;

				FVector AliForce = ComputeAlignment(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += AliForce * Settings.AlignmentWeight;

				FVector CohForce = ComputeCohesion(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += CohForce * Settings.CohesionWeight;

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
					FQuat TargetRotation = Velocity.ToOrientationQuat();
					FQuat CurrentRotation = Transform.GetRotation();
					Transform.SetRotation(FQuat::Slerp(CurrentRotation, TargetRotation, Settings.RotationSpeed * DT));
				}
			}
		});
}

FVector UMassBoidsProcesser::ComputeSeparation(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector Steering = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j) continue;

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.AvoidRadius * Settings.AvoidRadius))
		{
			FVector Diff = MyPos - OtherPos;
			Diff.Normalize();
			Diff /= FMath::Sqrt(DistSq);

			Steering += Diff;
			Count++;
		}
	}

	if (Count > 0)
	{
		Steering /= (float)Count;
		return SteerTowards(Steering, MyVel, Settings);
	}

	return Steering;
}

FVector UMassBoidsProcesser::ComputeAlignment(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector AvgVel = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j) continue;

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.DetectionRadius * Settings.DetectionRadius))
		{
			AvgVel += Velocities[j].Value;
			Count++;
		}
	}

	if (Count > 0)
	{
		AvgVel /= (float)Count;
		
		return SteerTowards(AvgVel, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcesser::ComputeCohesion(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j) continue;

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.DetectionRadius * Settings.DetectionRadius))
		{
			CenterOfMass += OtherPos;
			Count++;
		}
	}

	if (Count > 0)
	{
		CenterOfMass /= (float)Count;

		return SteerTowards(CenterOfMass - MyPos, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcesser::ComputeWander(const FVector& MyVel, const FMassBoidsFragment& Settings, float DT) const
{
	FVector CircleCenter = MyVel.GetSafeNormal() * Settings.DetectionRadius;
	FVector RandomPoint = FMath::VRand();
	FVector DesiredDir = (CircleCenter + (RandomPoint * Settings.WanderJitter)).GetSafeNormal();
	FVector Steer = (DesiredDir * Settings.MaxMoveSpeed) - MyVel;

	return Steer;
}

FVector UMassBoidsProcesser::SteerTowards(const FVector& DesiredDirection, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const
{
	FVector DesiredVelocity = DesiredDirection.GetSafeNormal() * Settings.MaxMoveSpeed;
	FVector Steer = DesiredVelocity - CurrentVel;
	return Steer.GetClampedToMaxSize(Settings.MaxSteerWeight);
}
