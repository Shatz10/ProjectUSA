// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "USAGameplayAbility.generated.h"

DECLARE_MULTICAST_DELEGATE(FUSAGASimpleDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUSAGASimpleDynamicDelegate);

/**
 * 所有游戏能力的基础类，继承自 UGameplayAbility
 * 
 * 核心功能:
 * - 提供统一的能力生命周期管理（激活、取消、结束）
 * - 管理游戏效果（GameplayEffect）的自动应用：
 *   - ActivateAbilityEffects: 激活时应用的效果
 *   - CancelAbilityEffects: 取消时应用的效果
 *   - EndAbilityEffects: 结束时应用的效果
 * - 提供目标向量计算系统（CalculateTargetVector），用于计算移动和攻击方向
 * - 客户端-服务器同步：ServerRPC_SetTargetVectorAndDoSomething 用于同步目标向量
 * - 提供简单的取消/结束方法：SimpleCancelAbility() 和 SimpleEndAbility()
 */
UCLASS()
class PROJECTUSA_API UUSAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** 激活能力时自动应用的游戏效果列表 */
	UPROPERTY(EditAnywhere, Category = "Custom Active Effect")
	TArray <TSubclassOf<class UGameplayEffect>> ActivateAbilityEffects;

	/** 取消能力时自动应用的游戏效果列表 */
	UPROPERTY(EditAnywhere, Category = "Custom Active Effect")
	TArray <TSubclassOf<class UGameplayEffect>> CancelAbilityEffects;

	/** 取消能力后应用的游戏效果列表 */
	UPROPERTY(EditAnywhere, Category = "Custom Active Effect")
	TArray <TSubclassOf<class UGameplayEffect>> PostCancelAbilityEffects;

	/** 结束能力时自动应用的游戏效果列表 */
	UPROPERTY(EditAnywhere, Category = "Custom Active Effect")
	TArray <TSubclassOf<class UGameplayEffect>> EndAbilityEffects;

	/** 结束能力后应用的游戏效果列表 */
	UPROPERTY(EditAnywhere, Category = "Custom Active Effect")
	TArray <TSubclassOf<class UGameplayEffect>> PostEndAbilityEffects;


public:
	UUSAGameplayAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	/** Client -(TargetData)-> Server 用于同步目标数据的函数 */
	void ActivateAbilityWithTargetData(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	//virtual void CalculateTargetDataVector(FVector& InOut);
	//virtual void DoSomethingWithTargetDataVector(const FVector& InVector);

	/** 使用目标向量激活能力 */
	void ActivateAbilityUsingTargetVector(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	
	/** 计算目标向量（移动方向和攻击方向） */
	virtual void CalculateTargetVector();
	/** 使用计算好的目标向量执行动作 */
	virtual void DoSomethingWithTargetVector();
	/** 检查是否可以激活能力 */
	virtual bool GetIsAbleToActivateCondition();

	FORCEINLINE FVector GetTargetVector_Move() { return TargetVector_Move; }
	FORCEINLINE FVector GetTargetVector_Attack() { return TargetVector_Attack; }
	FORCEINLINE float GetTargetDistance() { return TargetDistance; }

	FUSAGASimpleDynamicDelegate OnActivateAbility;
	FUSAGASimpleDynamicDelegate OnCancelAbility;
	FUSAGASimpleDynamicDelegate OnEndAbility;

	UFUNCTION(BlueprintCallable)
	void SimpleCancelAbility ();

	UFUNCTION(BlueprintCallable)
	void SimpleEndAbility();

private:
	//void ActivateAbilityWithTargetData_Client(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	//void NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	//void ActivateAbilityWithTargetData_ClientServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	/** 服务器RPC：设置目标向量并执行动作（用于客户端-服务器同步） */
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetTargetVectorAndDoSomething
	(const FVector& InVector, const FVector& InVector_Attack, float InDistance = -1.0f);
	
	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastRPC_SetTargetVectorAndDoSomething(const FVector& InVector);


protected:
	/** 移动目标向量 */
	UPROPERTY()
	FVector TargetVector_Move;

	/** 攻击目标向量 */
	UPROPERTY()
	FVector TargetVector_Attack;

	/** 目标距离 */
	UPROPERTY()
	float TargetDistance = -1.0f;

	//UFUNCTION(BlueprintCallable)
	void ApplyEffectsViaArray(const TArray<TSubclassOf<class UGameplayEffect>>& GameplayEffects, 
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		float GameplayEffectLevel = 1.0f, 
		int32 Stacks = 1);

	UFUNCTION(BlueprintCallable)
	void ApplyEffectsViaArray(const TArray<TSubclassOf<class UGameplayEffect>>& GameplayEffects);

	UFUNCTION(Server, Reliable)
	void ServerRPC_ApplyEffectsViaArray(const TArray<TSubclassOf<class UGameplayEffect>>& GameplayEffects);

	// TODO 수정
	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastRPC_ApplyEffectsViaArray(const TArray<TSubclassOf<class UGameplayEffect>>& GameplayEffects);
};
