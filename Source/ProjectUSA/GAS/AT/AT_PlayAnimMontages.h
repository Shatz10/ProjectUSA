// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AT/USAAbilityTask.h"

#include "Struct/USAStructs.h"

#include "AT_PlayAnimMontages.generated.h"


/**
 * 播放动画蒙太奇任务
 * 
 * 核心功能:
 * - 播放指定的动画蒙太奇
 * - 游戏标签驱动的章节切换：
 *   - AnimMontageSectionMapByGameplayTagAdded: 标签添加时切换到对应章节（用于连击系统）
 *   - AnimMontageSectionMapByGameplayTagRemoved: 标签移除时切换到对应章节（用于恢复动作）
 * - 监听蒙太奇事件（完成、混合、中断、取消）
 * - 支持动态章节切换，实现灵活的连击系统
 * 
 * 使用场景: 所有需要播放动画的能力
 */
UCLASS()
class PROJECTUSA_API UAT_PlayAnimMontages : public UUSAAbilityTask
{
	GENERATED_BODY()
	
public:
	// ========================================================================================
	// 动画蒙太奇事件委托（可在蓝图中绑定）
	// ========================================================================================

	/** 动画蒙太奇播放完成时触发 */
	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	OnCompleted;

	/** 动画蒙太奇开始混合退出时触发 */
	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	OnBlendOut;

	/** 动画蒙太奇被中断时触发 */
	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	OnInterrupted;

	/** 动画蒙太奇被取消时触发 */
	UPROPERTY(BlueprintAssignable)
	FUSAATSimpleDelegate	OnCancelled;

	// ========================================================================================
	// 任务创建和激活
	// ========================================================================================

	/** 
	 * 创建新的播放动画蒙太奇任务
	 * @param OwningAbility 拥有此任务的能力
	 * @param AnimMontageData 动画蒙太奇数据（包含动画、章节映射等）
	 * @return 创建的任务实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (/*DisplayName = "PlayMontageAndWait",*/
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_PlayAnimMontages* GetNewAbilityTask_PlayAnimMontages
	(UGameplayAbility* OwningAbility, const FPlayAnimMontageData& AnimMontageData);

	/** 
	 * 激活任务
	 * 流程：
	 * 1. 注册游戏标签监听（用于章节切换）
	 * 2. 根据当前标签状态确定起始章节
	 * 3. 播放动画蒙太奇
	 */
	virtual void Activate() override;

	/** 
	 * 正常结束任务
	 * 根据配置决定是停止动画还是播放结束动画
	 */
	virtual void SimpleEndAbilityTask() override;

	/** 
	 * 取消任务
	 * 立即停止播放中的动画蒙太奇
	 */
	virtual void SimpleCancelAbilityTask() override;

	// ========================================================================================
	// 游戏标签驱动的章节切换（用于连击系统）
	// ========================================================================================

	/** 
	 * 当游戏标签被添加时调用
	 * 用于连击系统：当玩家按下攻击键时添加标签，触发切换到连击章节
	 * @param InTag 被添加的标签
	 * @param NewCount 标签的新计数
	 */
	UFUNCTION()
	void OnAnimSectionGameplayTagAdded(const FGameplayTag InTag, int32 NewCount);

	/** 
	 * 当游戏标签被移除时调用
	 * 用于恢复动作：当连击窗口关闭时移除标签，触发恢复到默认章节
	 * @param InTag 被移除的标签
	 * @param NewCount 标签的新计数
	 */
	UFUNCTION()
	void OnAnimSectionGameplayTagRemoved(const FGameplayTag InTag, int32 NewCount);


	//FTimerHandle CallSectionTimerHandle;
	//void OnSectionTimerHandleEnd();

protected:
	// ========================================================================================
	// 任务生命周期管理
	// ========================================================================================

	/** 
	 * 任务销毁时调用
	 * 清理所有注册的游戏标签监听器
	 */
	virtual void OnDestroy(bool AbilityIsEnding) override;
	
	/** 
	 * 停止播放中的动画蒙太奇
	 * @return 如果成功停止了动画返回true，否则返回false
	 */
	bool StopPlayingMontage();

	// ========================================================================================
	// 内部数据
	// ========================================================================================

	/** 动画蒙太奇数据指针（由创建函数传入） */
	const FPlayAnimMontageData* PlayAnimMontageData;

	/** 动画混合退出委托（未使用，保留用于未来扩展） */
	FOnMontageBlendingOutStarted BlendingOutDelegate;

	/** 动画结束委托（未使用，保留用于未来扩展） */
	FOnMontageEnded MontageEndedDelegate;

	/** 中断委托句柄（未使用，保留用于未来扩展） */
	FDelegateHandle InterruptedHandle;

	/** 存储所有注册的游戏标签监听器的句柄（用于清理） */
	TMap<FGameplayTag, FDelegateHandle> DelegateHandles;
};
