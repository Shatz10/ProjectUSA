// Fill out your copyright notice in the Description page of Project Settings.

// 해당 어빌리티 태스크는 기본으로 제공하는 UAbilityTask_MoveToLocation을 상속한 것임

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_MoveToLocation.h"
#include "AT_MoveToLocationByVelocity.generated.h"

/**
 * 通过速度移动到指定位置的任务
 * 
 * 核心功能:
 * - 基于曲线（CurveFloat/CurveVector）的平滑移动
 * - 移动完成后应用后续速度（AfterVelocity）
 * - 继承自 UAbilityTask_MoveToLocation，提供网络同步支持
 * 
 * 使用场景: 冲刺、技能位移、移动到目标位置
 */
UCLASS()
class PROJECTUSA_API UAT_MoveToLocationByVelocity : public UAbilityTask_MoveToLocation
{
	GENERATED_BODY()

public:
	/** Move to the specified location, using the vector curve (range 0 - 1) if specified, otherwise the float curve (range 0 - 1) or fallback to linear interpolation */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_MoveToLocationByVelocity* GetNewAbilityTask_MoveToLocationByVelocity
	(UGameplayAbility* OwningAbility, 
		FName TaskInstanceName, 
		FVector Location, 
		FVector AfterVelocity,
		float Duration, 
		UCurveFloat* OptionalInterpolationCurve, 
		UCurveVector* OptionalVectorInterpolationCurve);

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;

	virtual void Activate() override;

	/** Tick function for this task, if bTickingTask == true */
	virtual void TickTask(float DeltaTime) override;

	UFUNCTION()
	void OnCancelTaskCallback ();

	UFUNCTION()
	void OnEndTaskCallback();

protected:
	UPROPERTY(Replicated)
	FVector PrevLocation;

	UPROPERTY(Replicated)
	FVector AfterVelocity;

	//

	UPROPERTY()
	FVector OffsetLocation;

	//UPROPERTY()
	//FVector OffsetLocationDelta;

	UPROPERTY()
	float PrevTime; 

	//UPROPERTY()
	//bool bIsPassesFirstTick;

	const float OffsetDeltaNumber = 1.3f;
	//UPROPERTY()
	//FVector StartCharacterLocation;
};
