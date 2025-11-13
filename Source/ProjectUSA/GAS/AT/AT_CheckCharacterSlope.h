// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"
#include "AT_CheckCharacterSlope.generated.h"

//DECLARE_MULTICAST_DELEGATE(FOnSlopeTrue)
//DECLARE_MULTICAST_DELEGATE(FOnSlopeFalse)
//DECLARE_MULTICAST_DELEGATE(FOnGroundOut)
//

/**
 * 检查角色是否在斜坡上的任务
 * 
 * 核心功能:
 * - 检测角色前方是否有斜坡（基于角度）
 * - 触发事件：
 *   - OnSlopeTrue: 检测到斜坡
 *   - OnSlopeFalse: 不在斜坡上
 *   - OnGroundOut: 离开地面
 * 
 * 使用场景: 滑行能力、斜坡相关动作
 */
UCLASS()
class PROJECTUSA_API UAT_CheckCharacterSlope : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAT_CheckCharacterSlope* GetNewAbilityTask_CheckCharacterSlope(UGameplayAbility* OwningAbility, class ACharacter* InCharacter, float InStartSlopeAngle);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	FUSAATSimpleDelegate OnSlopeTrue;
	FUSAATSimpleDelegate OnSlopeFalse;
	FUSAATSimpleDelegate OnGroundOut;
	
public:
	virtual void SimpleEndAbilityTask() override;

public:
	TObjectPtr <class ACharacter> MyCharacter;

	float StartSlopeAngle;

public:
	bool bIsCharacterOnSlopeWithForwardDirection;

	FORCEINLINE bool GetIsCharacterOnSlopeWithForwardDirection() const { return bIsCharacterOnSlopeWithForwardDirection; }


};
