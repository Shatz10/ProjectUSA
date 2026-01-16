// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"

#include "Struct/USAStructs.h"

#include "AT_ChangeCharacterMovementInfo.generated.h"


/**
 * 改变角色移动参数的任务
 * 
 * 核心功能:
 * - 临时改变角色的移动参数（速度、摩擦力、重力等）
 * - 任务结束时自动恢复原始参数
 * - 支持取消时恢复
 * 
 * 使用场景: 滑行、冲刺、特殊移动状态
 */
UCLASS()
class PROJECTUSA_API UAT_ChangeCharacterMovementInfo : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAT_ChangeCharacterMovementInfo* GetNewAbilityTask_ChangeCharacterMovementInfo(UGameplayAbility* OwningAbility, class ACharacter* InCharacter, const FCharacterMovementWalkInfo& InInfo);

	virtual void Activate() override;
	//virtual void TickTask(float DeltaTime) override;

	FCharacterMovementWalkInfo ChangeCharacterMovementWalkInfo;
	FCharacterMovementWalkInfo DefaultCharacterMovementWalkInfo;

	TObjectPtr<class ACharacter> MyCharacter;


	void SetCharacterMovementInfo();
	void ResetCharacterMovementInfo();

	virtual void SimpleCancelAbilityTask() override;
	virtual void SimpleEndAbilityTask() override;
	
};