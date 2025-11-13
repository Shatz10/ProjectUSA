// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"
#include "AT_CheckCharacterIsFalling.generated.h"

//DECLARE_MULTICAST_DELEGATE(FOnPositiveFalling);
//DECLARE_MULTICAST_DELEGATE(FOnNegativeFalling);
//DECLARE_MULTICAST_DELEGATE(FOnGrounded);
//DECLARE_MULTICAST_DELEGATE(FOnFinished);

/**
 * 检查角色是否在掉落的任务
 * 
 * 核心功能:
 * - 持续检测角色移动组件的掉落状态
 * - 触发事件：
 *   - OnPositiveFalling: 正向掉落（向下）
 *   - OnNegativeFalling: 负向掉落（向上，如被击飞）
 *   - OnGrounded: 落地
 *   - OnFinished: 任务完成
 * 
 * 使用场景: 需要根据掉落状态改变行为的能力（如空中技能、落地检测）
 */
UCLASS()
class PROJECTUSA_API UAT_CheckCharacterIsFalling : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAT_CheckCharacterIsFalling* GetNewAbilityTask_CheckCharacterIsFalling(UGameplayAbility* OwningAbility, /*class ACharacter* InCharacter,*/ class UCharacterMovementComponent* InCharacterMovement);

	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;
	

	FUSAATSimpleDelegate OnPositiveFalling;
	FUSAATSimpleDelegate OnNegativeFalling;
	FUSAATSimpleDelegate OnGrounded;
	FUSAATSimpleDelegate OnFinished;


protected:
	//UPROPERTY ()
	//TObjectPtr <class ACharacter> MyCharacter;

	UPROPERTY()
	TObjectPtr <class UCharacterMovementComponent> MyCharacterMovementComponent;

	UPROPERTY ()
	bool bIsFalling = false;
};
