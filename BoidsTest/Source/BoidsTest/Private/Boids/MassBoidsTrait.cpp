// Fill out your copyright notice in the Description page of Project Settings.


#include "Boids/MassBoidsTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Boids/BoidsTargetFragment.h"
#include "StructUtils/SharedStruct.h"
#include <MassActorSubsystem.h>

void UMassBoidsTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 개별 Fragment를 추가하여 엔티티마다 가져야 할 데이터를 정의
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassVelocityFragment>();
	BuildContext.AddFragment<FBoidsTargetFragment>();

	// Visualization 관련 Fragment 추가
	BuildContext.AddFragment<FMassActorFragment>();

	// FSharedStruct
	// - 구조체에서 TSharedPtr 처리하여 사용하고 싶을떄 사용하는 방식
	// (참조 카운팅을 위함)
	// 
	// - SharedFragment이기에 이러한 래핑함수를 통해 SharedFragment를 전달
	//   -> Mass AI에서 '하나'의 공용 Fragment를 사용하므로 이러한 SharedPtr 처리가 필요
	BuildContext.AddSharedFragment(FSharedStruct::Make(BoidsSettings));
}
