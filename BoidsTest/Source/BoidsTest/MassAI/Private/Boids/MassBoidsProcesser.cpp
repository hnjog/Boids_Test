// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAI/Public/Boids/MassBoidsProcesser.h"
#include "MassCommonFragments.h"		// Tranform Fragment
#include "MassMovementFragments.h"		// VelocityFragment
#include "MassExecutionContext.h"
#include "MassAI/Public/Boids/MassBoidsFragment.h"
#include "MassAI/Public/Boids/BoidsTargetFragment.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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

	EntityQuery.AddRequirement<FBoidsTargetFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddSharedRequirement<FMassBoidsFragment>(EMassFragmentAccess::ReadOnly);

	// 쿼리 등록
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsProcesser::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Super가 딱히 의미가 없을듯?
	FVector PlayerLocation = FVector::ZeroVector;
	bool bFoundPlayer = false;

	if (UWorld* World = GetWorld())
	{
		// 로컬 플레이어 0번 가져오기
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			PlayerLocation = PlayerPawn->GetActorLocation();
			bFoundPlayer = true;
		}
	}

	// Entity 순회하며 로직 순회
	EntityQuery.ForEachEntityChunk(Context, [this, PlayerLocation, bFoundPlayer](FMassExecutionContext& Context)
		{
			// 데이터 배열 가져오기 (성능을 위해 청크 단위로)
			const int32 NumEntities = Context.GetNumEntities();

			// 개별 데이터는 배열(ArrayView)로 나옵니다.
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMassVelocityFragment> Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();
			TArrayView<FBoidsTargetFragment> TargetInfos = Context.GetMutableFragmentView<FBoidsTargetFragment>();

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
				FBoidsTargetFragment& TargetInfo = TargetInfos[i];

				if (bFoundPlayer)
				{
					TargetInfo.TargetPosition = PlayerLocation;
					TargetInfo.IsTargetChase = true;
				}
				else
				{
					TargetInfo.IsTargetChase = false;
				}

				FVector CurrentPos = Transform.GetLocation();

				FVector Acceleration = FVector::ZeroVector;

				// 타겟이 있으면 그쪽으로 조향
				if (TargetInfo.IsTargetChase)
				{
					Acceleration += ComputeBounds(CurrentPos, Velocity, TargetInfo.TargetPosition, Settings.BoundsRadius, Settings) * Settings.BoundsWeight;
				}

				FVector SepForce = ComputeSeparation(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += SepForce * Settings.SeparationWeight;

				FVector AliForce = ComputeAlignment(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += AliForce * Settings.AlignmentWeight;

				FVector CohForce = ComputeCohesion(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += CohForce * Settings.CohesionWeight;

				FVector AvoidForce = ComputeObstacleAvoidance(CurrentPos, Velocity, Settings, GetWorld());
				Acceleration += AvoidForce * Settings.ObstacleAvoidanceWeight;

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

	const float Epsilon = 1.0f;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j) 
			continue;

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.AvoidRadius * Settings.AvoidRadius))
		{
			FVector Diff = MyPos - OtherPos;
			float Scale = 1.0f / (FMath::Max(DistSq, Epsilon));

			Steering += Diff.GetSafeNormal() * Scale;
			Count++;
		}
	}

	if (Count > 0)
	{
		//Steering /= (float)Count;
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

FVector UMassBoidsProcesser::ComputeBounds(const FVector& MyPos, const FVector& MyVel, const FVector& CenterPos, float Radius, const FMassBoidsFragment& Settings) const
{
	FVector Offset = CenterPos - MyPos;
	float DistSq = Offset.SizeSquared();
	float RadiusSq = Radius * Radius;

	if (DistSq > RadiusSq)
	{
		FVector PullForce = Offset.GetSafeNormal();

		FVector TangentForce = FVector::CrossProduct(PullForce, FVector::UpVector);

		FVector FinalDir = (PullForce + (TangentForce * Settings.SpiralFactor)).GetSafeNormal();
		return SteerTowards(FinalDir, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcesser::ComputeObstacleAvoidance(const FVector& MyPos, const FVector& MyVel, const FMassBoidsFragment& Settings, const UWorld* World) const
{
	if (!World) return FVector::ZeroVector;

	FVector Forward = MyVel.GetSafeNormal();
	// 속도가 거의 없으면 전방(X)을 기준으로
	if (Forward.IsNearlyZero()) 
		Forward = FVector::ForwardVector;

	FQuat VelocityQuat = Forward.ToOrientationQuat();

	// 감지 거리 (속도가 빠르면 더 멀리 봐야 함)
	float CheckDistance = Settings.ObstacleCheckDistance;

	FVector TotalAvoidForce = FVector::ZeroVector;
	int32 HitCount = 0;

	struct RayConfig
	{
		FVector Direction;
		float LengthScale;
		float WeightMultiplier;
	};

	const float Angle = Settings.ObstacleAvoidanceDegree;

	int32 TotalRays = Settings.ObstacleAvoidanceLineCount;
	if (TotalRays < 1) TotalRays = 1; // 최소 1개는 쏴야 함

	TArray<RayConfig> Rays;
	Rays.Reserve(TotalRays);

	Rays.Add({ FVector::ForwardVector, 1.0f, 2.0f });

	if (TotalRays > 1)
	{
		int32 SideRayCount = TotalRays - 1;

		float AngleStep = 360.0f / (float)SideRayCount;

		FVector BaseConeVec = FRotator(Angle, 0, 0).Vector();

		for (int32 i = 0; i < SideRayCount; ++i)
		{
			float CurrentRoll = i * AngleStep;
			FVector RotatedDir = BaseConeVec.RotateAngleAxis(CurrentRoll, FVector::ForwardVector);
			Rays.Add({ RotatedDir, 0.8f, 1.0f });
		}
	}

	FCollisionQueryParams Params;

	for (const RayConfig& Ray : Rays)
	{
		// 로컬 방향을 월드 방향으로 변환
		FVector WorldDir = VelocityQuat.RotateVector(Ray.Direction);

		FVector Start = MyPos;
		FVector End = MyPos + (WorldDir * (CheckDistance * Ray.LengthScale));

		FHitResult Hit;
		// WorldStatic(벽) 등과 충돌 검사
		bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);

		if (bHit)
		{
			FVector AvoidDir = Hit.ImpactNormal;

			float DistRatio = (Hit.Distance / (CheckDistance * Ray.LengthScale));
			float StrengthByDist = 1.0f - DistRatio;

			TotalAvoidForce += AvoidDir * StrengthByDist * Ray.WeightMultiplier;
			HitCount++;
		}
	}

	if (HitCount > 0)
	{
		return SteerTowards(TotalAvoidForce, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcesser::SteerTowards(const FVector& DesiredDirection, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const
{
	FVector DesiredVelocity = DesiredDirection.GetSafeNormal() * Settings.MaxMoveSpeed;
	FVector Steer = DesiredVelocity - CurrentVel;
	return Steer.GetClampedToMaxSize(Settings.MaxSteerWeight);
}
