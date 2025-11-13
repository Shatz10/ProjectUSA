// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/GA_CharacterAction.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Components/CapsuleComponent.h"

#include "GAS/AT/AT_MoveToLocationByVelocity.h"
#include "GAS/AT/AT_LaunchCharacterForPeriod.h"
#include "GAS/AT/AT_WaitDelay.h"
#include "GAS/AT/AT_PlayAnimMontages.h"
#include "GAS/AT/AT_SpawnActors.h"
#include "GAS/AT/AT_TraceAttack.h"
#include "GAS/AT/AT_WaitGameplayTag.h"

#include "GAS/AttributeSet/USAAttributeSet.h"

#include "Interface/USAAttackableInterface.h"
#include "Interface/USATargetableInterface.h"

#include "Character/USACharacterBase.h"

#include "Kismet/KismetSystemLibrary.h"

#include "AbilitySystemComponent.h"

#include "ProjectUSA.h"


void UGA_CharacterAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 调用父类激活函数（初始化基础数据）
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 检查激活条件（角色和移动组件是否存在）
	if (GetIsAbleToActivateCondition() == false)
	{
		SimpleEndAbility();
		return;
	}

	// 使用目标向量系统激活能力
	// 这会调用 CalculateTargetVector() 计算方向，然后调用 DoSomethingWithTargetVector() 执行动作
	ActivateAbilityUsingTargetVector(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 应用护甲属性（临时增加护甲值，用于某些动作的霸体效果）
	AddArmorAttributeFromBase(ArmorAttributeAddNumber);

	// 检查中断条件（在激活的第一帧就检查，如果满足条件立即结束）
	// 例如：如果设置了"当标签被添加时中断"，且标签已经存在，则立即结束
	if (InterruptGameplayTag.IsValid() == true
		&& IsValid(GetAbilitySystemComponentFromActorInfo()) == true)
	{
		switch (InterruptType)
		{
		case ECharacterActionEndType::WaitTagAdded:
			// 如果中断标签已存在，立即结束能力
			if (GetAbilitySystemComponentFromActorInfo()->GetGameplayTagCount(InterruptGameplayTag) > 0)
			{
				SimpleEndAbility();
			}
			break;

		case ECharacterActionEndType::WaitTagRemoved:
			// 如果中断标签不存在，立即结束能力
			if (GetAbilitySystemComponentFromActorInfo()->GetGameplayTagCount(InterruptGameplayTag) <= 0)
			{
				SimpleEndAbility();
			}
			break;

		case ECharacterActionEndType::WaitTime:
		default:
			// 基于时间的中断在DoSomethingWithTargetVector中处理
			break;
		}
	}

	// 调用蓝图事件（可在蓝图中实现自定义逻辑，如播放音效、特效等）
	K2_DoSomething_Activate();
}

void UGA_CharacterAction::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	// 어트리뷰트 종료
	ResetArmorAttributeToBase();

	// 블루프린트에서 지정한 Cancel 수행
	K2_DoSomething_Cancel();
}

void UGA_CharacterAction::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 어트리뷰트 종료
	ResetArmorAttributeToBase();

	// 블루프린트에서 지정한 End 수행
	K2_DoSomething_End();
}

void UGA_CharacterAction::CalculateTargetVector()
{
	// ========================================================================================
	// 第一步：获取角色和组件引用
	// ========================================================================================
	ACharacter* MyCharacter = nullptr;
	AUSACharacterBase* MyUSACharacter = nullptr;
	UCharacterMovementComponent* MyCharacaterMovementComponent = nullptr;

	if (CurrentActorInfo != nullptr)
	{
		MyCharacter = Cast <ACharacter>(CurrentActorInfo->AvatarActor);
	}

	if (MyCharacter != nullptr)
	{
		MyUSACharacter = Cast <AUSACharacterBase>(MyCharacter);
		MyCharacaterMovementComponent = MyCharacter->GetCharacterMovement();

		// 初始化移动向量为角色朝向，攻击向量为零向量
		TargetVector_Move = MyCharacter->GetActorForwardVector();
		TargetVector_Attack = FVector::ZeroVector;
	}

	// 更新当前可目标化的Actor（用于锁定目标）
	if (MyUSACharacter)
	{	
		MyUSACharacter->UpdateCurrentTargetableActor_Instant();
	}

	// ========================================================================================
	// 第二步：计算目标相关数据（如果有目标的话）
	// ========================================================================================
	bool bIsFinalMoveToTargetAction = false;
	float TempDistance = -1.0f;
	FVector TempVector = MyCharacter->GetActorForwardVector();

	IUSAAttackableInterface* AttackableInterface = Cast<IUSAAttackableInterface>(GetAvatarActorFromActorInfo());
	IUSATargetableInterface* TargetableActorInterface = nullptr;

	// 如果角色实现了攻击接口，尝试获取目标
	if (AttackableInterface != nullptr
		&& AttackableInterface->GetTargetableInterface() != nullptr)
	{
		TargetableActorInterface = AttackableInterface->GetTargetableInterface();

		// 获取目标位置
		FVector TargetableActorLocation = AttackableInterface->GetTargetableActorLocation();

		// 计算攻击方向（从角色指向目标）
		TargetVector_Attack = (TargetableActorLocation - MyCharacter->GetActorLocation());
		TargetVector_Attack.Normalize();

		// 如果角色在地面上，将目标位置Z坐标对齐到角色Z坐标（保持在同一水平面）
		if (IsValid(MyCharacaterMovementComponent) == true
			&& MyCharacaterMovementComponent->IsFalling() == false)
		{
			TargetableActorLocation.Z = MyCharacter->GetActorLocation().Z;
		}

		// 计算到目标的向量和距离
		TempVector = (TargetableActorLocation - MyCharacter->GetActorLocation());
		TempVector.Normalize();
		TempDistance = (TargetableActorLocation - MyCharacter->GetActorLocation()).Length();

		// 判断目标是否在移动范围内
		bIsFinalMoveToTargetAction = (TempDistance <= MoveToTargetRange);
	}

	// ========================================================================================
	// 第三步：根据移动类型和方向类型计算最终移动向量
	// ========================================================================================

	// 情况1：移动到目标（Magnet移动）- 当启用移动到目标且目标在范围内时
	if (bIsMoveToTargetAction == true && bIsFinalMoveToTargetAction == true)
	{
		// 计算目标半径和角色半径
		float TargetRadius = 0.0f;
		if (TargetableActorInterface != nullptr)
		{
			TargetRadius = TargetableActorInterface->GetTargetableCapsuleRadius();
		}

		float SourceRadius = 0.0f;
		if (IsValid(MyCharacter) == true)
		{
			SourceRadius = MyCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		}

		// 计算实际移动距离（减去两个半径和间隙，防止重叠）
		TargetDistance = FMath::Max(TempDistance - TargetRadius - SourceRadius - MoveToTargetGap, 0.0f);
		TargetVector_Move = TempVector; // 移动方向指向目标
	}
	// 情况2：自定义移动 - 移动到角色指定的自定义位置
	else if (MoveType == ECharacterActionMoveType::Custom)
	{
		FVector CustomLocation = MyUSACharacter->GetActionCustomLocation();

		// 计算到自定义位置的向量和距离
		TargetVector_Move = (CustomLocation - MyCharacter->GetActorLocation());
		TargetVector_Move.Normalize();
		TargetDistance = (CustomLocation - MyCharacter->GetActorLocation()).Length();
	}
	// 情况3：其他移动类型（Move、Walk、Launch等）- 根据方向类型计算
	else
	{
		TargetDistance = -1.0f; // 不使用距离

		switch (DirectionType)
		{
		case ECharacterActionDirectionType::Input:
			// 优先使用角色的输入移动方向
			if (MyUSACharacter != nullptr
				&& MyUSACharacter->GetUSACharacterDirection_InputMovement().SquaredLength() > SMALL_NUMBER)
			{
				TargetVector_Move = MyUSACharacter->GetUSACharacterDirection_InputMovement();
			}
			// 其次使用待处理的移动输入
			else if (MyCharacter != nullptr
				&& MyCharacter->GetPendingMovementInputVector().SquaredLength() > SMALL_NUMBER)
			{
				TargetVector_Move = MyCharacter->GetPendingMovementInputVector();
			}
			// 最后使用角色朝向
			else
			{
				TargetVector_Move = MyCharacter->GetActorForwardVector();
			}
			break;

		case ECharacterActionDirectionType::Target:
			// 优先使用目标方向
			if (MyUSACharacter != nullptr
				&& MyUSACharacter->GetUSACharacterDirection_Target().SquaredLength() > SMALL_NUMBER)
			{
				TargetVector_Move = MyUSACharacter->GetUSACharacterDirection_Target();
			}
			// 其次使用输入移动方向
			else if (MyUSACharacter != nullptr
				&& MyUSACharacter->GetUSACharacterDirection_InputMovement().SquaredLength() > SMALL_NUMBER)
			{
				TargetVector_Move = MyUSACharacter->GetUSACharacterDirection_InputMovement();
			}
			// 再次使用待处理的移动输入
			else if (MyCharacter != nullptr
				&& MyCharacter->GetPendingMovementInputVector().SquaredLength() > SMALL_NUMBER)
			{
				TargetVector_Move = MyCharacter->GetPendingMovementInputVector();
			}
			// 最后使用角色朝向
			else
			{
				TargetVector_Move = MyCharacter->GetActorForwardVector();
			}
			break;

		case ECharacterActionDirectionType::None:
		default:
			// 不改变移动向量（保持为角色朝向）
			break;
		}
	}	
}

void UGA_CharacterAction::DoSomethingWithTargetVector()
{
	// 再次检查激活条件（防止在计算目标向量后条件发生变化）
	if (GetIsAbleToActivateCondition() == false)
	{
		SimpleCancelAbility();
		return;
	}

	// ========================================================================================
	// 第一步：获取角色和组件引用
	// ========================================================================================
	ACharacter* MyCharacter = nullptr;
	UCharacterMovementComponent* MyCharacterMovementComponent = nullptr;
	IUSAAttackableInterface* AttackableInterface = nullptr;

	if (CurrentActorInfo != nullptr)
	{
		MyCharacter = Cast <ACharacter>(CurrentActorInfo->AvatarActor);
	}

	if (MyCharacter != nullptr)
	{
		MyCharacterMovementComponent = MyCharacter->GetCharacterMovement();
	}

	// ========================================================================================
	// 第二步：设置角色朝向和移动
	// ========================================================================================
	if (MyCharacter != nullptr
		&& MyCharacterMovementComponent != nullptr)
	{
		// 计算前方向和右方向（用于相对坐标转换）
		FVector	ForwardDirection = TargetVector_Move;
		ForwardDirection.Z = 0.0f; // 只使用水平方向
		ForwardDirection.Normalize();

		FVector	RightDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection);
		RightDirection.Normalize();

		// 用于存储移动相关数据
		FVector EndLocation = FVector::ZeroVector;
		FVector AfterVelocity = FVector::ZeroVector;
		FVector FinalLaunchVector = FVector::ZeroVector;

		// 保存相机弹簧臂的变换（防止角色旋转时相机也跟着旋转）
		USpringArmComponent* SpringArmComponent = MyCharacter->GetComponentByClass<USpringArmComponent>();
		FTransform SprintArmComponentTransform;
		if (IsValid(SpringArmComponent))
		{
			SprintArmComponentTransform = SpringArmComponent->GetComponentTransform();
		}

		// 设置角色朝向为移动方向
		MyCharacter->SetActorRotation(ForwardDirection.Rotation());
		MyCharacter->UpdateComponentTransforms();

		// 恢复相机弹簧臂的变换（保持相机不变）
		if (IsValid(SpringArmComponent))
		{
			SpringArmComponent->SetWorldTransform(SprintArmComponentTransform);
		}

		// 声明移动任务变量
		UAT_MoveToLocationByVelocity* AbilityTask_MoveToLocation;
		UAT_LaunchCharacterForPeriod* AbilityTask_LaunchCharacter;
		UAT_ChangeCharacterMovementInfo* AbilityTask_ChangeMovementInfo;

		// ========================================================================================
		// 第三步：根据移动类型执行移动
		// ========================================================================================

		// 情况1：移动到目标（Magnet移动）- 当有目标距离时
		if (TargetDistance > SMALL_NUMBER
			&& MoveType != ECharacterActionMoveType::Custom)
		{
			// 计算目标位置
			EndLocation = MyCharacter->GetActorLocation() + TargetVector_Move * TargetDistance;

			// 计算移动后的速度（相对于角色方向）
			AfterVelocity = (ForwardDirection * MoveToTargetAfterVelocity.X)
				+ (RightDirection * MoveToTargetAfterVelocity.Y)
				+ (FVector::UpVector * MoveToTargetAfterVelocity.Z);

			// 创建并激活移动到目标的任务
			AbilityTask_MoveToLocation = UAT_MoveToLocationByVelocity::GetNewAbilityTask_MoveToLocationByVelocity
			(this, TEXT("MoveToTarget"), EndLocation, AfterVelocity, MoveToTargetDuration, MoveToTargetCurveFloat, MoveToTargetCurveVector);

			AbilityTask_MoveToLocation->ReadyForActivation();
		}
		// 情况2：根据MoveType执行不同的移动
		else
		{
			switch (MoveType)
			{
			case ECharacterActionMoveType::Move:
				// 基于偏移量的移动
				// 计算目标位置（当前位置 + 相对偏移）
				EndLocation = MyCharacter->GetActorLocation()
					+ (ForwardDirection * MoveOffsetLocation.X)
					+ (RightDirection * MoveOffsetLocation.Y)
					+ (FVector::UpVector * MoveOffsetLocation.Z);

				// 计算移动后的速度
				AfterVelocity = (ForwardDirection * MoveAfterVelocity.X)
					+ (RightDirection * MoveAfterVelocity.Y)
					+ (FVector::UpVector * MoveAfterVelocity.Z);

				// 创建并激活移动任务
				AbilityTask_MoveToLocation = UAT_MoveToLocationByVelocity::GetNewAbilityTask_MoveToLocationByVelocity
				(this, TEXT("Move"), EndLocation, AfterVelocity, MoveDuration, MoveCurveFloat, nullptr);

				AbilityTask_MoveToLocation->ReadyForActivation();
				break;

			case ECharacterActionMoveType::Walk:
				// 临时改变角色移动参数（速度、摩擦力等）
				AbilityTask_ChangeMovementInfo = UAT_ChangeCharacterMovementInfo::GetNewAbilityTask_ChangeCharacterMovementInfo
				(this, MyCharacter, WalkMovementInfo);
				// 绑定结束和取消事件，确保任务结束时恢复移动参数
				OnEndAbility.AddDynamic(AbilityTask_ChangeMovementInfo, &UAT_ChangeCharacterMovementInfo::SimpleEndAbilityTask);
				OnCancelAbility.AddDynamic(AbilityTask_ChangeMovementInfo, &UAT_ChangeCharacterMovementInfo::SimpleCancelAbilityTask);
				AbilityTask_ChangeMovementInfo->ReadyForActivation();
				break;

			case ECharacterActionMoveType::Launch:
				// 发射角色（给角色一个速度）
				// 计算发射向量（相对于角色方向）
				FinalLaunchVector = (ForwardDirection * MoveLaunchVector.X)
					+ (RightDirection * MoveLaunchVector.Y)
					+ (FVector::UpVector * MoveLaunchVector.Z);

				// 创建并激活发射任务
				AbilityTask_LaunchCharacter = UAT_LaunchCharacterForPeriod::GetNewAbilityTask_LaunchCharacterForPeriod
				(this, FinalLaunchVector, bMoveLaunchXYOverride, bMoveLaunchZOverride, MoveLaunchPeriod);

				AbilityTask_LaunchCharacter->ReadyForActivation();
				break;

			case ECharacterActionMoveType::Custom:
				// 自定义移动（移动到角色指定的位置）
				EndLocation = MyCharacter->GetActorLocation() + TargetVector_Move * TargetDistance;

				AfterVelocity = (ForwardDirection * CustomMoveAfterVelocity.X)
					+ (RightDirection * CustomMoveAfterVelocity.Y)
					+ (FVector::UpVector * CustomMoveAfterVelocity.Z);

				AbilityTask_MoveToLocation = UAT_MoveToLocationByVelocity::GetNewAbilityTask_MoveToLocationByVelocity
				(this, TEXT("CustomMove"), EndLocation, AfterVelocity, CustomMoveDuration, CustomMoveCurveFloat, CustomMoveCurveVector);

				AbilityTask_MoveToLocation->ReadyForActivation();
				break;

			case ECharacterActionMoveType::None:
			default:
				// 不执行移动
				break;
			}
		}
	}

	// ========================================================================================
	// 第四步：设置能力结束条件
	// ========================================================================================
	UAT_WaitGameplayTagAdded* WaitGameplayTagAdded = nullptr;
	UAT_WaitGameplayTagRemoved* WaitGameplayTagRemoved = nullptr;
	UAT_WaitDelay* AbilityTaskDelay = nullptr;

	switch (EndType)
	{
	case ECharacterActionEndType::WaitTagAdded:
		// 等待标签被添加时结束
		WaitGameplayTagAdded = UAT_WaitGameplayTagAdded::GetNewAbilityTask_WaitGameplayTagAdded(this, EndGameplayTag);
		WaitGameplayTagAdded->Added.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		WaitGameplayTagAdded->ReadyForActivation();
		break;

	case ECharacterActionEndType::WaitTagRemoved:
		// 等待标签被移除时结束
		WaitGameplayTagRemoved = UAT_WaitGameplayTagRemoved::GetNewAbilityTask_WaitGameplayTagRemoved(this, EndGameplayTag);
		WaitGameplayTagRemoved->Removed.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		WaitGameplayTagRemoved->ReadyForActivation();
		break;

	case ECharacterActionEndType::WaitTime:
	default:
		// 等待指定时间后结束
		AbilityTaskDelay = UAT_WaitDelay::GetNewAbilityTask_WaitDelay(this, Period);
		AbilityTaskDelay->OnFinish.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		AbilityTaskDelay->ReadyForActivation();
		break;
	}

	// ========================================================================================
	// 第五步：设置能力中断条件
	// ========================================================================================
	UAT_WaitGameplayTagAdded* WaitGameplayTagAdded_Interrupt = nullptr;
	UAT_WaitGameplayTagRemoved* WaitGameplayTagRemoved_Interrupt = nullptr;

	switch (InterruptType)
	{
	case ECharacterActionEndType::WaitTagAdded:
		// 当标签被添加时中断能力
		WaitGameplayTagAdded_Interrupt = UAT_WaitGameplayTagAdded::GetNewAbilityTask_WaitGameplayTagAdded(this, InterruptGameplayTag);
		WaitGameplayTagAdded_Interrupt->Added.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		WaitGameplayTagAdded_Interrupt->ReadyForActivation();
		break;

	case ECharacterActionEndType::WaitTagRemoved:
		// 当标签被移除时中断能力
		WaitGameplayTagRemoved_Interrupt = UAT_WaitGameplayTagRemoved::GetNewAbilityTask_WaitGameplayTagRemoved(this, InterruptGameplayTag);
		WaitGameplayTagRemoved_Interrupt->Removed.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		WaitGameplayTagRemoved_Interrupt->ReadyForActivation();
		break;

	case ECharacterActionEndType::WaitTime:
	default:
		// 基于时间的中断在激活时已检查
		break;
	}

	// ========================================================================================
	// 第六步：在服务器上执行攻击和生成（仅在服务器或单机模式下）
	// ========================================================================================
	if (UKismetSystemLibrary::IsServer(GetWorld())
		|| UKismetSystemLibrary::IsStandalone(GetWorld()))
	{
		// 生成Actor（特效、碰撞体等）
		UAT_SpawnActors* AbiltiyTaskSpawn = UAT_SpawnActors::GetNewAbilityTask_SpawnActors(this, SpawnActorData, TargetVector_Attack);
		AbiltiyTaskSpawn->ReadyForActivation();

		// 执行攻击追踪（碰撞检测）
		UAT_TraceAttack* AbiltiyTaskAttack = UAT_TraceAttack::GetNewAbilityTask_TraceAttack(this, AttackTraceData, TargetVector_Attack);
		AbiltiyTaskAttack->ReadyForActivation();
	}

	// ========================================================================================
	// 第七步：播放动画蒙太奇
	// ========================================================================================
	UAT_PlayAnimMontages* AbilityTaskMontage = UAT_PlayAnimMontages::GetNewAbilityTask_PlayAnimMontages(this, ActionAnimMontageData);
	// 绑定结束和取消事件，确保能力结束时停止动画
	OnEndAbility.AddDynamic(AbilityTaskMontage, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
	OnCancelAbility.AddDynamic(AbilityTaskMontage, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
	AbilityTaskMontage->ReadyForActivation();
}

bool UGA_CharacterAction::GetIsAbleToActivateCondition()
{
	// 컴포넌트 검사
	ACharacter* MyCharacter = nullptr;
	UCharacterMovementComponent* MyCharacterMovementComponent = nullptr;

	if (CurrentActorInfo != nullptr)
	{
		MyCharacter = Cast <ACharacter>(CurrentActorInfo->AvatarActor);
	}

	if (MyCharacter != nullptr)
	{
		MyCharacterMovementComponent = MyCharacter->GetCharacterMovement();
	}

	if (MyCharacter == nullptr
		|| MyCharacterMovementComponent == nullptr)
	{
		return false;
	}
	
	return true;
}

void UGA_CharacterAction::AddArmorAttributeFromBase(float InAddArmor)
{
	if (bIsAppliedArmorAttribute == false)
	{
		bIsAppliedArmorAttribute = true;

		UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
		if (OwnerASC != nullptr
			&& OwnerASC->GetSet <UUSAAttributeSet>() != nullptr)
		{
			float BaseArmor = 0.0f;
			bool CheckIsAttributeFound = false;
			BaseArmor = OwnerASC->GetGameplayAttributeValue(UUSAAttributeSet::GetBaseArmorAttribute(), CheckIsAttributeFound);

			if (CheckIsAttributeFound == true)
			{
				OwnerASC->SetNumericAttributeBase(UUSAAttributeSet::GetCurrentArmorAttribute(), BaseArmor + InAddArmor);
			}
		}
	}
}

void UGA_CharacterAction::ResetArmorAttributeToBase()
{
	if (bIsAppliedArmorAttribute == true)
	{
		bIsAppliedArmorAttribute = false;

		UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
		if (OwnerASC != nullptr
			&& OwnerASC->GetSet <UUSAAttributeSet>() != nullptr)
		{
			float BaseArmor = 0.0f;
			bool CheckIsAttributeFound = false;
			BaseArmor = OwnerASC->GetGameplayAttributeValue(UUSAAttributeSet::GetBaseArmorAttribute(), CheckIsAttributeFound);

			if (CheckIsAttributeFound == true)
			{
				OwnerASC->SetNumericAttributeBase(UUSAAttributeSet::GetCurrentArmorAttribute(), BaseArmor);
			}
		}
	}
}