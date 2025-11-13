// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "AbilitySystemComponent.h"

#include "USAAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
 GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUSAAttributeSimpleDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUSAAttributeSimpleDynamicDelegateFloat, float, InFloat);

/**
 * 角色属性集，管理角色的所有游戏属性
 * 
 * 核心属性:
 * - CurrentHealth: 当前生命值（可复制）
 * - MaxHealth: 最大生命值（可复制）
 * - CurrentArmor: 当前护甲值（可复制）
 * - BaseArmor: 基础护甲值（可复制）
 * - Damage: 伤害值（用于应用伤害）
 * 
 * 核心功能:
 * - 属性变化回调:
 *   - PreAttributeChange: 属性改变前的处理
 *   - PostAttributeChange: 属性改变后的处理
 *   - PreGameplayEffectExecute: 游戏效果执行前的处理
 *   - PostGameplayEffectExecute: 游戏效果执行后的处理
 * - 生命值管理:
 *   - OnOutOfHealth: 生命值耗尽时触发
 *   - OnRevive: 复活时触发
 *   - OnCurrentHealthChanged: 当前生命值改变时触发
 *   - OnMaxHealthChanged: 最大生命值改变时触发
 * - 网络复制: 关键属性支持网络复制，确保多人游戏同步
 * 
 * 使用场景: 所有需要管理角色属性的系统（生命值、护甲、伤害等）
 */
UCLASS()
class PROJECTUSA_API UUSAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UUSAAttributeSet();

	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, CurrentHealth);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, CurrentArmor);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, BaseArmor);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, Damage);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/** 生命值耗尽时触发 */
	mutable FUSAAttributeSimpleDynamicDelegate OnOutOfHealth;
	/** 复活时触发 */
	mutable FUSAAttributeSimpleDynamicDelegate OnRevive;
	/** 当前生命值改变时触发 */
	mutable FUSAAttributeSimpleDynamicDelegateFloat OnCurrentHealthChanged;
	/** 最大生命值改变时触发 */
	mutable FUSAAttributeSimpleDynamicDelegateFloat OnMaxHealthChanged;

protected:
	/** 当前生命值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentHealth;

	/** 最大生命值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	/** 当前护甲值（可复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentArmor;

	/** 基础护甲值（可复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseArmor;

	/** 伤害值（用于应用伤害） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Damage;

	bool bOutOfHealth = false;


protected:

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_MaxHealth();

	
	// 체력 관련 어트리뷰트 접근 위함
	friend class AUSACharacterBase;
};
