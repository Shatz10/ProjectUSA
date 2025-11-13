// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "USAAbilityTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUSAATSimpleDelegate);

/**
 * 所有能力任务的基础类，继承自 UAbilityTask
 * 
 * 核心功能:
 * - 提供统一的取消/结束方法
 * - 提供委托系统（OnAbilityTaskCancel, OnAbilityTaskEnd）
 * - 可配置是否可取消（bIsCancelable）
 */
UCLASS()
class PROJECTUSA_API UUSAAbilityTask : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	/** 任务取消时触发的委托 */
	FUSAATSimpleDelegate OnAbilityTaskCancel;
	/** 任务结束时触发的委托 */
	FUSAATSimpleDelegate OnAbilityTaskEnd;

	/** 简单取消任务 */
	UFUNCTION()
	virtual void SimpleCancelAbilityTask();

	/** 简单结束任务 */
	UFUNCTION()
	virtual void SimpleEndAbilityTask();

	/** 广播简单委托 */
	void BroadcastSimpleDelegate(const FUSAATSimpleDelegate& InDelegate);

	virtual void Activate() override;

	/** 是否可取消 */
	bool bIsCancelable = true;
	
};
