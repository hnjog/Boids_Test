// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BoidsTest : ModuleRules
{
	public BoidsTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",

			// --- Mass AI 필수 모듈 ---
            "MassEntity",       // ECS 코어
            "MassCommon",       // 공통 유틸
            "MassMovement",     // 이동 관련
            "MassActors",       // 액터 연동 (필요시)
            "MassSpawner",      // 스포너
            "MassRepresentation", // ISM 렌더링 (시각화)
            "MassSignals",      // 신호 처리
            "StructUtils",      // FInstancedStruct 등 데이터 처리 (필수!)
            "MassNavigation",   // 네비게이션/ZoneGraph 연동
            
            // AI 관련
            "MassAIBehavior",
            "StateTreeModule",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
