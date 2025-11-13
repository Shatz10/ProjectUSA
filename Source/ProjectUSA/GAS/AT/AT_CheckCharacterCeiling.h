// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"
#include "AT_CheckCharacterCeiling.generated.h"

//DECLARE_MULTICAST_DELEGATE(FOnCeilingTrue)
//DECLARE_MULTICAST_DELEGATE(FOnCeilingFalse)

//DECLARE_MULTICAST_DELEGATE(FOnSimpleDelegate)


/**
 * 检查角色是否碰到天花板的任务
 * 
 * 核心功能:
 * - 使用球体检测角色上方是否有天花板
 * - 触发事件：
 *   - OnCeilingTrue: 检测到天花板
 *   - OnCeilingFalse: 没有天花板
 * 
 * 使用场景: 滑行能力、防止角色卡在天花板
 */
UCLASS()
class PROJECTUSA_API UAT_CheckCharacterCeiling : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAT_CheckCharacterCeiling* GetNewAbilityTask_CheckCharacterCeiling(UGameplayAbility* OwningAbility, class ACharacter* InCharacter, float InCharacterHeight, float InCharacterRadius);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	FUSAATSimpleDelegate OnCeilingTrue;
	FUSAATSimpleDelegate OnCeilingFalse;

public:
	TObjectPtr <class ACharacter> MyCharacter;
	float DetectCharacterHeight;
	float DetectCharacterRadius;

public:
	bool bIsCharacterCeilingDetected;
	FORCEINLINE bool GetIsCharacterCeilingDetected() const { return bIsCharacterCeilingDetected; }
};
