// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/AT/AT_PlayAnimMontages.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"

#include "Components/SkeletalMeshComponent.h"

#include "GameFramework/Character.h"

#include "ProjectUSA.h"


UAT_PlayAnimMontages* UAT_PlayAnimMontages::GetNewAbilityTask_PlayAnimMontages(UGameplayAbility* OwningAbility, const FPlayAnimMontageData& AnimMontageData)
{
	// 创建新的任务实例
	UAT_PlayAnimMontages* MyObj = NewAbilityTask<UAT_PlayAnimMontages>(OwningAbility);

	// 保存动画蒙太奇数据指针（注意：这里保存的是引用，确保数据在任务生命周期内有效）
	MyObj->PlayAnimMontageData = &AnimMontageData;

	// 标记任务可取消
	MyObj->bIsCancelable = true;

	return MyObj;
}

void UAT_PlayAnimMontages::Activate()
{
	// ========================================================================================
	// 第一步：验证数据有效性
	// ========================================================================================
	if (Ability == nullptr)
	{
		SimpleCancelAbilityTask();
		return;
	}

	if (PlayAnimMontageData == nullptr)
	{
		SimpleCancelAbilityTask();
		return;
	}

	if (PlayAnimMontageData->AnimMontage == nullptr)
	{
		SimpleCancelAbilityTask();
		return;
	}

	// ========================================================================================
	// 第二步：注册游戏标签监听（用于连击系统的章节切换）
	// ========================================================================================
	bool bPlayedMontage = false;

	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();
	ACharacter* MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());

	// 默认起始章节名称
	FName StartSectionName = PlayAnimMontageData->StartAnimMontageSectionName;

	if (ASC)
	{
		// 注册"标签添加"监听器（用于连击系统）
		// 当标签被添加时，切换到对应的动画章节
		TArray<FGameplayTag> KeyArray;
		PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded.GenerateKeyArray(KeyArray);
		for (const FGameplayTag TagAdded : KeyArray)
		{
			// 注册标签事件监听器
			FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(TagAdded).AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagAdded);
			DelegateHandles.Add({ TagAdded, DelegateHandle });

			// 如果标签已经存在，使用对应的章节作为起始章节
			// 例如：如果"连击窗口"标签已存在，直接播放连击章节
			if (ASC->HasMatchingGameplayTag(TagAdded))
			{
				StartSectionName = PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded[TagAdded];
			}
		}

		// 注册"标签移除"监听器（用于恢复动作）
		// 当标签被移除时，切换到对应的动画章节
		PlayAnimMontageData->AnimMontageSectionMapByGameplayTagRemoved.GenerateKeyArray(KeyArray);
		for (const FGameplayTag TagRemoved : KeyArray)
		{
			FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(TagRemoved).AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagRemoved);
			DelegateHandles.Add({ TagRemoved, DelegateHandle });
		}
	}

	// ========================================================================================
	// 第三步：播放动画蒙太奇
	// ========================================================================================
	if (MyCharacter != nullptr)
	{
		// 播放动画蒙太奇，从计算好的起始章节开始
		if (MyCharacter->PlayAnimMontage
		(PlayAnimMontageData->AnimMontage, 
			PlayAnimMontageData->AnimMontageRate, 
			StartSectionName))
		{
			bPlayedMontage = true;
		}
	}

	// 如果播放失败，取消任务
	if (!bPlayedMontage)
	{
		SimpleCancelAbilityTask();
		return;
	}

	// 设置任务等待角色（等待动画播放完成）
	SetWaitingOnAvatar();
}

void UAT_PlayAnimMontages::SimpleEndAbilityTask()
{
	if (PlayAnimMontageData == nullptr)
	{
		return;
	}

	//UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	//if (ASC)
	//{
	//	if (ASC->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
	//	{
	//		if (PlayAnimMontageData->bHasEndSection)
	//		{
	//			ASC->CurrentMontageJumpToSection(PlayAnimMontageData->EndAnimMontageSectionDetail.SectionName);
	//		}
	//		else
	//		{
	//			ASC->CurrentMontageStop();
	//		}
	//	}
	//}

	ACharacter* MyCharacter = nullptr;
	
	if (Ability != nullptr)
	{
		MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
	}
	
	if (MyCharacter != nullptr
		&& PlayAnimMontageData->AnimMontage != nullptr)
	{
		if (MyCharacter->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
		{
			if (PlayAnimMontageData->bIsStopWhenFinished == true)
			{
				MyCharacter->StopAnimMontage();
			}
			else
			{
				if (IsValid (PlayAnimMontageData->EndAnimMontage) == true)
				{
					//MyCharacter->StopAnimMontage();
					MyCharacter->PlayAnimMontage
					(PlayAnimMontageData->EndAnimMontage,
						PlayAnimMontageData->AnimMontageRate);
				}
				else
				{
					// ...
				}
			}
		}
	}

	Super::SimpleEndAbilityTask();
}

void UAT_PlayAnimMontages::SimpleCancelAbilityTask()
{
	StopPlayingMontage();
	
	Super::SimpleCancelAbilityTask();
}

void UAT_PlayAnimMontages::OnAnimSectionGameplayTagAdded(FGameplayTag InTag, int32 NewCount)
{
	// 当标签计数大于0时（标签被添加或计数增加）
	if (NewCount > 0)
	{
		ACharacter* MyCharacter = nullptr;

		if (Ability != nullptr)
		{
			MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
		}

		if (MyCharacter == nullptr)
		{
			return;
		}

		// 查找标签对应的动画章节
		FName SectionName = NAME_None;

		if (PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded.Contains(InTag))
		{
			SectionName = PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded[InTag];
		}

		// 停止当前动画并切换到新章节
		// 这是连击系统的核心：当玩家在连击窗口内按下攻击键时，添加标签，触发切换到连击章节
		MyCharacter->StopAnimMontage();
		MyCharacter->PlayAnimMontage
		(PlayAnimMontageData->AnimMontage,
			PlayAnimMontageData->AnimMontageRate,
			SectionName);
	}
}

void UAT_PlayAnimMontages::OnAnimSectionGameplayTagRemoved(const FGameplayTag InTag, int32 NewCount)
{
	// 当标签计数小于等于0时（标签被移除或计数减少到0）
	if (NewCount <= 0)
	{
		ACharacter* MyCharacter = nullptr;

		if (Ability != nullptr)
		{
			MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
		}

		if (MyCharacter == nullptr)
		{
			return;
		}

		// 查找标签对应的动画章节
		FName SectionName = NAME_None;

		if (PlayAnimMontageData->AnimMontageSectionMapByGameplayTagRemoved.Contains(InTag))
		{
			SectionName = PlayAnimMontageData->AnimMontageSectionMapByGameplayTagRemoved[InTag];
		}

		// 停止当前动画并切换到新章节
		// 这是恢复动作的核心：当连击窗口关闭时，移除标签，触发恢复到默认章节
		MyCharacter->StopAnimMontage();
		MyCharacter->PlayAnimMontage
		(PlayAnimMontageData->AnimMontage,
			PlayAnimMontageData->AnimMontageRate,
			SectionName);
	}
}

bool UAT_PlayAnimMontages::StopPlayingMontage()
{
	if (PlayAnimMontageData == nullptr)
	{
		return false;
	}

	if (PlayAnimMontageData->AnimMontage == nullptr)
	{
		return false;
	}

	//// Check if the montage is still playing
	//// The ability would have been interrupted, in which case we should automatically stop the montage
	//UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();

	//if (ASC)
	//{
	//	if (ASC->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
	//	{
	//		ASC->CurrentMontageStop();
	//		ASC->ClearAnimatingAbility(Ability);
	//		
	//		return true;
	//	}
	//}

	ACharacter* MyCharacter = nullptr;

	if (Ability != nullptr)
	{
		MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
	}

	if (MyCharacter != nullptr)
	{
		if (MyCharacter->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
		{
			MyCharacter->StopAnimMontage();
			
			return true;
		}
	}

	return false;
}

void UAT_PlayAnimMontages::OnDestroy(bool AbilityIsEnding)
{
	// 调用父类销毁函数
	Super::OnDestroy(AbilityIsEnding);

	// ========================================================================================
	// 清理所有注册的游戏标签监听器（防止内存泄漏）
	// ========================================================================================
	UAbilitySystemComponent* ASC = nullptr;
	if (Ability != nullptr)
	{
		ASC = Ability->GetAbilitySystemComponentFromActorInfo();
	}

	if (ASC)
	{
		// 获取所有注册的标签
		TArray<FGameplayTag> KeyArray;
		DelegateHandles.GenerateKeyArray(KeyArray);

		// 移除所有监听器
		for (FGameplayTag Tag : KeyArray)
		{
			ASC->RegisterGameplayTagEvent(Tag).Remove(DelegateHandles[Tag]);
		}
	}
}


//void UAT_PlayAnimMontages::OnSectionTimerHandleEnd()
//{
//	if (PlayAnimMontageData == nullptr)
//	{
//		return;
//	}
//
//	//UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
//	//if (ASC)
//	//{
//	//	if (ASC->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
//	//	{
//	//		ASC->CurrentMontageJumpToSection(PlayAnimMontageData->MiddleAnimMontageSectionDetails[CurrentPlayAnimMontageIndex].SectionName);
//	//	}
//	//}
//
//	ACharacter* MyCharacter = nullptr;
//
//	if (Ability != nullptr)
//	{
//		MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
//	}
//
//	if (MyCharacter != nullptr)
//	{
//		if (MyCharacter->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
//		{
//			if (PlayAnimMontageData->bHasEndSection)
//			{
//				MyCharacter->StopAnimMontage();
//				MyCharacter->PlayAnimMontage
//				(PlayAnimMontageData->AnimMontage,
//					PlayAnimMontageData->AnimMontageRate,
//					PlayAnimMontageData->MiddleAnimMontageSectionDetails[CurrentPlayAnimMontageIndex].SectionName);
//			}
//		}
//	}
//
//	CurrentPlayAnimMontageIndex += 1;
//	if (PlayAnimMontageData->MiddleAnimMontageSectionDetails.Num() <= CurrentPlayAnimMontageIndex)
//	{
//		return;
//	}
//
//	float WaitTime = 0.0f;
//	if (CurrentPlayAnimMontageIndex == 0)
//	{
//		WaitTime = PlayAnimMontageData->MiddleAnimMontageSectionDetails[CurrentPlayAnimMontageIndex].SectionPlayTime;
//	}
//	else
//	{
//		WaitTime = PlayAnimMontageData->MiddleAnimMontageSectionDetails[CurrentPlayAnimMontageIndex].SectionPlayTime
//			- PlayAnimMontageData->MiddleAnimMontageSectionDetails[CurrentPlayAnimMontageIndex-1].SectionPlayTime;
//	}
//
//	GetWorld()->GetTimerManager().ClearTimer(CallSectionTimerHandle);
//	GetWorld()->GetTimerManager().SetTimer(CallSectionTimerHandle, this, &UAT_PlayAnimMontages::OnSectionTimerHandleEnd, WaitTime, false);
//
//}
