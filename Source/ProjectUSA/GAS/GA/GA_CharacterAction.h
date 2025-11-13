// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"

#include "GAS/AT/AT_PlayAnimMontages.h"
#include "GAS/AT/AT_SpawnActors.h"
#include "GAS/AT/AT_TraceAttack.h"
#include "GAS/AT/AT_ChangeCharacterMovementInfo.h"

#include "Enum/USAEnums.h"

#include "GA_CharacterAction.generated.h"

/**
 * 角色动作的核心能力类，用于实现攻击、技能等动作
 * 
 * 核心功能:
 * - 方向系统: 支持多种方向类型（输入方向、目标方向、自定义方向）
 * - 移动系统: 支持多种移动类型：
 *   - Move: 基于偏移量的移动
 *   - Walk: 临时改变角色移动参数
 *   - Launch: 发射角色
 *   - Custom: 自定义移动曲线
 *   - Magnet: 自动移动到目标位置
 * - 结束/中断系统: 支持基于时间、游戏标签的结束和中断条件
 * - 动画系统: 播放动画蒙太奇，支持游戏标签驱动的章节切换（用于连击系统）
 * - 生成系统: 在服务器上生成特效或碰撞体
 * - 攻击追踪: 执行攻击碰撞检测
 * - 护甲系统: 临时增加/恢复护甲属性
 * 
 * 使用场景: 所有需要移动、动画、攻击的动作（如普通攻击、技能攻击等）
 */
UCLASS()
class PROJECTUSA_API UGA_CharacterAction : public UUSAGameplayAbility
{
	GENERATED_BODY()
	
public:

	/** 方向类型：输入方向、目标方向、自定义方向等 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Direction")
	ECharacterActionDirectionType DirectionType = ECharacterActionDirectionType::None;

	/** 移动类型：Move、Walk、Launch、Custom、Magnet等 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	ECharacterActionMoveType MoveType = ECharacterActionMoveType::Move;

	/** 结束类型：基于时间、游戏标签等 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Action End")
	ECharacterActionEndType EndType = ECharacterActionEndType::WaitTime;

	/** 动作持续时间（当EndType为WaitTime时使用） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Action End")
	float Period = 0.5f;

	/** 结束游戏标签（当EndType为WaitTag时使用） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Action End")
	FGameplayTag EndGameplayTag;

	/** 中断类型：基于时间、游戏标签等 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Action Interrupt")
	ECharacterActionEndType InterruptType = ECharacterActionEndType::None;

	/** 中断游戏标签（当InterruptType为WaitTag时使用） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Action Interrupt")
	FGameplayTag InterruptGameplayTag;

	// ========================================================================================
	// 移动到目标相关配置（Magnet移动类型）
	// ========================================================================================

	/** 是否启用自动移动到目标功能（当目标在范围内时自动移动到目标位置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	bool bIsMoveToTargetAction = false;

	/** 移动到目标后的速度（相对于角色方向：X=前，Y=右，Z=上） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	FVector MoveToTargetAfterVelocity = FVector::ZeroVector;

	/** 移动到目标的持续时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	float MoveToTargetDuration = 0.5f;

	/** 移动到目标的触发范围（当目标在此范围内时才会触发自动移动） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	float MoveToTargetRange = 100.0f;

	/** 移动到目标时与目标保持的最小距离（防止角色与目标重叠） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	float MoveToTargetGap = 50.0f;

	/** 移动到目标的浮点曲线（用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	UCurveFloat* MoveToTargetCurveFloat;

	/** 移动到目标的向量曲线（用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move To Target")
	class UCurveVector* MoveToTargetCurveVector;

	// ========================================================================================
	// Move移动类型相关配置（基于偏移量的移动）
	// ========================================================================================

	/** 移动偏移量（相对于角色方向：X=前，Y=右，Z=上） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	FVector MoveOffsetLocation = FVector::ZeroVector;

	/** 移动完成后的速度（相对于角色方向：X=前，Y=右，Z=上） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	FVector MoveAfterVelocity = FVector::ZeroVector;

	/** 移动持续时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	float MoveDuration = 0.5f;

	/** 移动速度曲线（浮点，用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	UCurveFloat* MoveCurveFloat;

	/** 移动速度曲线（向量，用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	class UCurveVector* MoveCurveVector;

	// ========================================================================================
	// Launch移动类型相关配置（发射角色）
	// ========================================================================================

	/** 发射向量（相对于角色方向：X=前，Y=右，Z=上） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	FVector MoveLaunchVector;

	/** 发射持续时间（-1表示使用默认值） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	float MoveLaunchPeriod = -1;

	/** 是否覆盖XY轴速度（如果为true，则完全覆盖角色的XY速度） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	bool bMoveLaunchXYOverride;

	/** 是否覆盖Z轴速度（如果为true，则完全覆盖角色的Z速度） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	bool bMoveLaunchZOverride;

	// ========================================================================================
	// Custom移动类型相关配置（自定义移动曲线）
	// ========================================================================================

	/** 自定义移动完成后的速度（相对于角色方向：X=前，Y=右，Z=上） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	FVector CustomMoveAfterVelocity = FVector::ZeroVector;

	/** 自定义移动持续时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	float CustomMoveDuration = 0.5f;

	/** 自定义移动速度曲线（浮点，用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	UCurveFloat* CustomMoveCurveFloat;

	/** 自定义移动速度曲线（向量，用于控制移动速度变化，范围0-1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	class UCurveVector* CustomMoveCurveVector;

	// ========================================================================================
	// Walk移动类型相关配置（临时改变角色移动参数）
	// ========================================================================================

	/** 临时改变的移动参数（速度、摩擦力、重力等） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Move")
	FCharacterMovementWalkInfo WalkMovementInfo;

	//

	/** 动画蒙太奇数据（支持游戏标签驱动的章节切换，用于连击系统） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Animation")
	struct FPlayAnimMontageData ActionAnimMontageData;

	/** 生成Actor数据（在服务器上生成特效或碰撞体） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Spawn")
	struct FSpawnActorData SpawnActorData ;

	/** 攻击追踪数据（执行攻击碰撞检测） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Attack")
	struct FAttackTraceInfos AttackTraceData;

	/** 临时增加的护甲值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Action Info: Attribute")
	float ArmorAttributeAddNumber = 0.0f;

	/** 是否已应用护甲属性 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Action Info: Attribute")
	bool bIsAppliedArmorAttribute = false;
	
	// ========================================================================================
	// 蓝图事件（可在蓝图中重写以实现自定义逻辑）
	// ========================================================================================

	/** 能力激活时在蓝图中调用的函数（用于动画蓝图等） */
	UFUNCTION(BlueprintImplementableEvent)
	void K2_DoSomething_Activate();

	/** 能力取消时在蓝图中调用的函数 */
	UFUNCTION(BlueprintImplementableEvent)
	void K2_DoSomething_Cancel();

	/** 能力结束时在蓝图中调用的函数 */
	UFUNCTION(BlueprintImplementableEvent)
	void K2_DoSomething_End();


public:
	// ========================================================================================
	// GAS系统核心函数重写
	// ========================================================================================

	/** 
	 * 激活能力（GAS系统调用）
	 * 流程：
	 * 1. 检查激活条件
	 * 2. 计算目标向量（方向）
	 * 3. 应用护甲属性
	 * 4. 检查中断条件
	 * 5. 调用蓝图事件
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 
	 * 取消能力（GAS系统调用）
	 * 流程：
	 * 1. 恢复护甲属性
	 * 2. 调用蓝图取消事件
	 */
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	/** 
	 * 结束能力（GAS系统调用）
	 * 流程：
	 * 1. 恢复护甲属性
	 * 2. 调用蓝图结束事件
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	// ========================================================================================
	// 目标向量计算系统（继承自USAGameplayAbility）
	// ========================================================================================

	/** 
	 * 计算目标向量（移动方向和攻击方向）
	 * 根据DirectionType计算：
	 * - Input: 使用输入方向
	 * - Target: 使用目标方向
	 * - None: 使用角色朝向
	 * 同时处理移动到目标（Magnet）和自定义移动的逻辑
	 */
	virtual void CalculateTargetVector() override;

	/** 
	 * 使用计算好的目标向量执行动作
	 * 这是核心执行函数，负责：
	 * 1. 设置角色朝向
	 * 2. 执行移动（根据MoveType）
	 * 3. 设置结束条件（根据EndType）
	 * 4. 设置中断条件（根据InterruptType）
	 * 5. 在服务器上生成Actor和攻击追踪
	 * 6. 播放动画蒙太奇
	 */
	virtual void DoSomethingWithTargetVector() override;

	/** 
	 * 检查是否可以激活能力
	 * 检查角色和移动组件是否存在
	 */
	virtual bool GetIsAbleToActivateCondition() override;

protected:
	// ========================================================================================
	// 护甲属性管理
	// ========================================================================================

	/** 
	 * 从基础护甲值增加护甲
	 * 在能力激活时调用，临时增加角色的护甲值
	 * @param InAddArmor 要增加的护甲值
	 */
	UFUNCTION()
	void AddArmorAttributeFromBase(float InAddArmor);

	/** 
	 * 将护甲值重置为基础值
	 * 在能力取消或结束时调用，恢复角色的原始护甲值
	 */
	UFUNCTION()
	void ResetArmorAttributeToBase();

};
