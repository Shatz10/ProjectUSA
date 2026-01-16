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
 * - CurrentPosture: 当前架势值（可复制）
 * - MaxPosture: 最大架势值（可复制）
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
 *   - OnCurrentPostureChanged: 当前架势值改变时触发
 *   - OnMaxPostureChanged: 最大架势值改变时触发
 *   - OnPostureBroken: 架势条被击破时触发
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
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, CurrentPosture);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, MaxPosture);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, PostureRecoverRate);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, CurrentSpiritEmblems);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, MaxSpiritEmblems);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, ResurrectionPower);
	ATTRIBUTE_ACCESSORS(UUSAAttributeSet, MaxResurrectionPower);

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
	/** 当前架势值改变时触发 */
	mutable FUSAAttributeSimpleDynamicDelegateFloat OnCurrentPostureChanged;
	/** 最大架势值改变时触发 */
	mutable FUSAAttributeSimpleDynamicDelegateFloat OnMaxPostureChanged;
	/** 架势条击破时触发 */
	mutable FUSAAttributeSimpleDynamicDelegate OnPostureBroken;

public:
	/** 当前生命值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentHealth;

	/** 最大生命值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxHealth;

	/** 当前护甲值（可复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentArmor;

	/** 基础护甲值（可复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData BaseArmor;

	/** 伤害值（用于应用伤害） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Damage;

	/** 当前架势值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPosture, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentPosture;

	/** 最大架势值（可复制） */
	UPROPERTY(ReplicatedUsing = OnRep_MaxPosture, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxPosture;

	/** 架势恢复速度（可复制） */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData PostureRecoverRate;

	/** 当前纸人数量 (Spirit Emblems) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSpiritEmblems, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData CurrentSpiritEmblems;

	/** 最大纸人数量 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxSpiritEmblems, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxSpiritEmblems;

	/** 当前起死回生之力 (Resurrection Power) */
	UPROPERTY(ReplicatedUsing = OnRep_ResurrectionPower, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData ResurrectionPower;

	/** 最大起死回生之力 (通常为3.0) */
	UPROPERTY(ReplicatedUsing = OnRep_MaxResurrectionPower, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData MaxResurrectionPower;

protected:
	bool bOutOfHealth = false;


	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_MaxHealth();

	UFUNCTION()
	void OnRep_CurrentPosture();

	UFUNCTION()
	void OnRep_MaxPosture();

	UFUNCTION()
	void OnRep_CurrentSpiritEmblems();

	UFUNCTION()
	void OnRep_MaxSpiritEmblems();

	UFUNCTION()
	void OnRep_ResurrectionPower();

	UFUNCTION()
	void OnRep_MaxResurrectionPower();
	
	//To access attributes related to physical strength
	friend class AUSACharacterBase;
};
