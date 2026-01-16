// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"

#include "Struct/USAStructs.h"

#include "AT_SpawnActors.generated.h"


/**
 * 生成 Actor 的任务（用于特效、碰撞体等）
 * 
 * 核心功能:
 * - 基于时间段生成多个 Actor
 * - 每个时间段可以配置：
 *   - Actor 类
 *   - 生成位置偏移
 *   - 旋转
 *   - 缩放
 * - 支持目标向量方向计算生成位置
 * - 服务器端执行（权威性）
 * 
 * 使用场景: 生成特效、生成碰撞体、生成投射物
 */
UCLASS()
class PROJECTUSA_API UAT_SpawnActors : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_SpawnActors* GetNewAbilityTask_SpawnActors
	(UGameplayAbility* OwningAbility, const FSpawnActorData& InSpawnActorData, const FVector& InTargetVector);

	virtual void Activate() override;

	void SpawnActorAndSetNextTimer();

public:
	const FSpawnActorData* SpawnActorData;

	FTimerHandle SpawnActorTimerHandle;

	float PrevSpawnActorTime = 0.0f;
	int CurrentSpwanActorIndex = 0;

	// 방향
	FVector TargetVector;
};
