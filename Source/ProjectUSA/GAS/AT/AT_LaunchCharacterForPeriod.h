// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"
#include "AT_LaunchCharacterForPeriod.generated.h"

/**
 * 在指定时间内发射角色的任务
 * 
 * 核心功能:
 * - 可配置 XY 和 Z 轴的速度覆盖
 * - 在指定时间内持续应用发射速度
 * - 支持 Tick 更新，精确控制发射时间
 * 
 * 使用场景: 击飞、跳跃、技能位移
 */
UCLASS()
class PROJECTUSA_API UAT_LaunchCharacterForPeriod : public UUSAAbilityTask
{
	GENERATED_BODY()
public:
	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAT_LaunchCharacterForPeriod* GetNewAbilityTask_LaunchCharacterForPeriod(UGameplayAbility* OwningAbility, FVector InVelocity, bool InOverrideXY, bool InOverrideZ, float InPeriod = -1.0f);

	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;

	virtual void SimpleCancelAbilityTask() override;

	FUSAATSimpleDelegate OnFinished;


protected:
	UPROPERTY ()
	FVector LaunchVelocity;

	UPROPERTY()
	bool bXYOverride;

	UPROPERTY()
	bool bZOverride;

	UPROPERTY()
	float Period;


	UPROPERTY()
	float StartTime;

	UPROPERTY()
	float EndTime;
};
