// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"
#include "AT_WaitGameplayTag.generated.h"

/**
 * 等待游戏标签的基础类
 * 
 * 核心功能:
 * - 监听游戏标签的变化
 * - 支持只触发一次或持续监听
 */
UCLASS()
class PROJECTUSA_API UAT_WaitGameplayTag : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	UAT_WaitGameplayTag();

	virtual void Activate() override;

	UFUNCTION()
	virtual void GameplayTagCallback(const FGameplayTag InTag, int32 NewCount);

	FGameplayTag	Tag;

protected:

	UAbilitySystemComponent* GetTargetASC();

	virtual void OnDestroy(bool AbilityIsEnding) override;
	bool RegisteredCallback;
	bool OnlyTriggerOnce;

	FDelegateHandle DelegateHandle;
};

/// ====================================================================================================

/**
 * 等待游戏标签被添加的任务
 * 
 * 核心功能:
 * - 当指定标签被添加时触发 Added 事件
 * - 如果标签已存在，立即触发
 * 
 * 使用场景: 等待特定状态、等待连击窗口打开
 */
UCLASS()
class PROJECTUSA_API UAT_WaitGameplayTagAdded : public UAT_WaitGameplayTag
{
	GENERATED_BODY()

public:
	UAT_WaitGameplayTagAdded();

	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	Added;

	/**
	 * 	Wait until the specified gameplay tag is Added. By default this will look at the owner of this ability. OptionalExternalTarget can be set to make this look at another actor's tags for changes.
	 *  If the tag is already present when this task is started, it will immediately broadcast the Added event. It will keep listening as long as OnlyTriggerOnce = false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitGameplayTagAdded* GetNewAbilityTask_WaitGameplayTagAdded(UGameplayAbility* OwningAbility, FGameplayTag Tag, bool OnlyTriggerOnce = false);

	virtual void GameplayTagCallback(const FGameplayTag InTag, int32 NewCount) override;
};

/// ====================================================================================================

/**
 * 等待游戏标签被移除的任务
 * 
 * 核心功能:
 * - 当指定标签被移除时触发 Removed 事件
 * 
 * 使用场景: 等待状态结束、等待连击窗口关闭
 */
UCLASS()
class PROJECTUSA_API UAT_WaitGameplayTagRemoved : public UAT_WaitGameplayTag
{
	GENERATED_BODY()

public:
	UAT_WaitGameplayTagRemoved();

	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	Removed;

	/**
	 * 	Wait until the specified gameplay tag is Added. By default this will look at the owner of this ability. OptionalExternalTarget can be set to make this look at another actor's tags for changes.
	 *  If the tag is already present when this task is started, it will immediately broadcast the Added event. It will keep listening as long as OnlyTriggerOnce = false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitGameplayTagRemoved* GetNewAbilityTask_WaitGameplayTagRemoved(UGameplayAbility* OwningAbility, FGameplayTag Tag, bool OnlyTriggerOnce = false);

	virtual void GameplayTagCallback(const FGameplayTag InTag, int32 NewCount) override;
};