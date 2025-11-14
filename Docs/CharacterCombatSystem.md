## 角色战斗系统 (C++)

本文档总结了角色战斗在输入、角色操作、动画和战斗逻辑方面的工作原理，重点关注主要的 C++ 源代码。

### 输入 → 能力激活
- 玩家输入通过 `AUSACharacterBase::SetupPlayerInputComponent` 中的增强输入系统绑定，包括带有 `InputID` 的能力输入。按下/释放时，它会查找对应的 `FGameplayAbilitySpec`，并尝试激活它或转发输入事件。

```692:735:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	...
	for (const auto& GameplayActiveAbility : GameplayAbilities_Active)
	{
		...
		EnhancedInputComponent->BindAction(GameplayActiveAbility.InputAction, ETriggerEvent::Triggered,
			this, &AUSACharacterBase::InputPressGameplayAbilityByInputID, GameplayActiveAbility.InputID);
		EnhancedInputComponent->BindAction(GameplayActiveAbility.InputAction, ETriggerEvent::Completed,
			this, &AUSACharacterBase::InputReleaseGameplayAbilityByInputID, GameplayActiveAbility.InputID);
	}
}
```

- 玩家移动/视角输入从摄像机偏航角计算世界方向，以实现响应式控制。

```901:920:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	...
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

	USACharacterInputMovementDirection = ForwardDirection * MovementVector.Y;
	USACharacterInputMovementDirection += RightDirection * MovementVector.X;
}
```

### 角色操作 (GAS 设置、游戏标签、目标锁定)
- GAS 在 `PossessedBy`（服务器端）中初始化并复制到客户端；能力在 `PostSetupGAS` 中授予。游戏标签事件处理器切换移动标志和约束。

```737:755:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::PossessedBy(AController* NewController)
{
	...
	if (bIsASCInitialized == false)
	{
		SetupGAS();
		PostSetupGAS();
		SetupAttributeSet();
		ResetAttributeSet();
		BeginStartAbilities();
	}
}
```

- 标签回调调整移动、输入和状态（例如，忽略输入、速度归零、蹲伏/坠落/死亡）。

```1228:1243:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::OnGameplayTagCallback_IgnoreMoveInput(const FGameplayTag CallbackTag, int32 NewCount)
{
	...
	if (NewCount > 0) { GetController()->SetIgnoreMoveInput (true); }
	else { GetController()->ResetIgnoreMoveInput (); }
}
```

- 目标锁定：玩家通过摄像机前方候选者的重叠评分来维持目标；通过 `LookTarget()` 切换，带有锁定摄像机管理。

```367:472:Source/ProjectUSA/Character/USACharacterPlayer.cpp
void AUSACharacterPlayer::SetCurrentTargetableActorUsingForwardVector(const FVector& InDirection, TObjectPtr<class AActor>& InOutTargetActorPointer)
{
	// Overlap in range, score by distance and dot with forward/camera vector
	// Choose best result as the current target
	...
}
```

### 能力编排 (核心)
- `UUSAGameplayAbility` 提供通用模式：在激活/取消/结束时应用效果，计算目标向量，并处理基于目标向量的能力的客户端-服务器流程。

```106:165:Source/ProjectUSA/GAS/GA/USAGameplayAbility.cpp
void UUSAGameplayAbility::ActivateAbility(...)
{
	...
	ApplyEffectsViaArray(ActivateAbilityEffects, Handle, ActorInfo, ActivationInfo);
	OnActivateAbility.Broadcast();
}

void UUSAGameplayAbility::ActivateAbilityUsingTargetVector(...)
{
	if (GetIsAbleToActivateCondition() == false) { SimpleCancelAbility(); return; }
	...
	CalculateTargetVector();
	...
	if (PlayerController && PlayerController->IsLocalController())
	{
		if (UKismetSystemLibrary::IsServer(GetWorld())) { DoSomethingWithTargetVector(); }
		else {
			ServerRPC_SetTargetVectorAndDoSomething(GetTargetVector_Move(), GetTargetVector_Attack(), GetTargetDistance());
			DoSomethingWithTargetVector();
		}
	}
	else { DoSomethingWithTargetVector(); }
}
```

### 角色动作能力 (战斗逻辑核心)
主要的近战动作能力按顺序执行移动、动画、特效生成和攻击追踪，以及结束/中断逻辑。它还会临时调整护甲，并支持磁吸/吸附到目标的移动。

```34:89:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::ActivateAbility(...)
{
	if (GetIsAbleToActivateCondition() == false) { SimpleEndAbility(); return; }
	ActivateAbilityUsingTargetVector(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AddArmorAttributeFromBase(ArmorAttributeAddNumber);
	// Early interrupt gate
	if (InterruptGameplayTag.IsValid() && IsValid(GetAbilitySystemComponentFromActorInfo())) {
		switch (InterruptType) {
		case ECharacterActionEndType::WaitTagAdded:
			if (GetAbilitySystemComponentFromActorInfo()->GetGameplayTagCount(InterruptGameplayTag) > 0) { SimpleEndAbility(); }
			break;
		case ECharacterActionEndType::WaitTagRemoved:
			if (GetAbilitySystemComponentFromActorInfo()->GetGameplayTagCount(InterruptGameplayTag) <= 0) { SimpleEndAbility(); }
			break;
		case ECharacterActionEndType::WaitTime:
		default: break;
		}
	}
	K2_DoSomething_Activate();
}
```

目标向量的计算优先考虑面向目标和输入，磁吸距离考虑两个胶囊体半径加上可配置的间隙：

```113:196:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::CalculateTargetVector()
{
	// Updates instant target; determines TargetVector_Attack (towards target)
	// Picks TargetVector_Move via: Move-to-target, Custom, Input, or Target direction
	// Magnet TargetDistance = max(distance - targetRadius - selfRadius - MoveToTargetGap, 0)
}
```

执行流程：
- 旋转到 `TargetVector_Move`；保留弹簧臂变换以避免摄像机弹出。
- 移动：
  - 使用 `UAT_MoveToLocationByVelocity` 进行磁吸移动到计算出的 `EndLocation` 和可选的后续速度。
  - 或使用相应的移动/发射/改变信息任务之一（Move/Walk/Launch/Custom 分支）。
- 结束/中断：
  - 通过 `UAT_WaitGameplayTagAdded/Removed` 或 `UAT_WaitDelay`（基于时间）结束。
  - 根据 `InterruptType` 和 `InterruptGameplayTag` 通过并行 WaitTag 任务中断。
- 服务器端：
  - `UAT_SpawnActors` 用于特效或碰撞盒。
  - `UAT_TraceAttack` 使用 `AttackTraceData` 和 `TargetVector_Attack` 进行定时追踪。
- 动画：
  - `UAT_PlayAnimMontages` 播放蒙太奇，并支持通过游戏标签动态切换片段。

```264:495:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::DoSomethingWithTargetVector()
{
	// Rotate → (Magnet/Move/Walk/Launch/Custom) → End/Interrupt → Server spawn/trace → Play montage
}
```

临时护甲在开始时应用，在结束/取消时恢复：

```522:564:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::AddArmorAttributeFromBase(float InAddArmor) { ... }
void UGA_CharacterAction::ResetArmorAttributeToBase() { ... }
```

#### 移动类型和方向选择
- **移动类型**在动作执行中处理：
  - **磁吸/吸附到目标**：在范围内且不使用自定义移动时。从 `TargetVector_Move * TargetDistance` 计算 `EndLocation`，并应用可选的后续速度。
  - **移动**：从当前位置偏移，使用 `MoveOffsetLocation` 和 `MoveAfterVelocity`。
  - **行走**：通过 `UAT_ChangeCharacterMovementInfo` 在动作持续时间内临时覆盖移动设置。
  - **发射**：使用配置的向量和 XY/Z 覆盖发射一段时间。
  - **自定义**：使用预计算的 `TargetVector_Move` 和 `TargetDistance`，带有自定义曲线和后续速度。

```350:409:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
// Magnet move if TargetDistance > 0 and not Custom, else switch on MoveType:
// Move → UAT_MoveToLocationByVelocity
// Walk → UAT_ChangeCharacterMovementInfo
// Launch → UAT_LaunchCharacterForPeriod
// Custom → UAT_MoveToLocationByVelocity with custom data
```

- **方向选择**：根据 `DirectionType` 和运行时输入/锁定，从输入、目标、自定义或前方向中选择 `TargetVector_Move`。攻击向量在可用时面向当前目标。

```211:261:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (DirectionType) {
  case Input: prefer input movement direction or pending input; fallback to forward
  case Target: prefer target direction; fallback to input or pending input; else forward
  case None/default: leave as-is
}
```

#### 结束和中断类型
- **结束条件**（其中之一）：
  - 等待游戏标签被添加或移除。
  - 等待固定时间延迟。

```417:445:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (EndType) {
  case WaitTagAdded: UAT_WaitGameplayTagAdded → SimpleEndAbility
  case WaitTagRemoved: UAT_WaitGameplayTagRemoved → SimpleEndAbility
  case WaitTime/default: UAT_WaitDelay → SimpleEndAbility
}
```

- **中断条件**与上述类似，并与结束逻辑并行运行：

```448:475:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (InterruptType) {
  case WaitTagAdded: UAT_WaitGameplayTagAdded → SimpleEndAbility
  case WaitTagRemoved: UAT_WaitGameplayTagRemoved → SimpleEndAbility
  case WaitTime/default: // none
}
```

#### 旋转期间的摄像机保持
- 能力将角色旋转到面向 `TargetVector_Move`，但保留弹簧臂变换以防止帧期间的摄像机弹出：

```306:326:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
USpringArmComponent* SpringArm = MyCharacter->GetComponentByClass<USpringArmComponent>();
FTransform Saved = SpringArm ? SpringArm->GetComponentTransform() : FTransform::Identity;
MyCharacter->SetActorRotation(ForwardDirection.Rotation());
MyCharacter->UpdateComponentTransforms();
if (SpringArm) { SpringArm->SetWorldTransform(Saved); }
```

#### 仅服务器端执行（生成和追踪）
- 生成和攻击追踪是权威的；在服务器（或独立）上，能力生成配置的 Actor 并使用计算出的 `TargetVector_Attack` 运行定时追踪攻击：

```477:488:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
if (IsServerOrStandalone) {
  UAT_SpawnActors::GetNewAbilityTask_SpawnActors(..., TargetVector_Attack)->ReadyForActivation();
  UAT_TraceAttack::GetNewAbilityTask_TraceAttack(..., TargetVector_Attack)->ReadyForActivation();
}
```

#### 蒙太奇播放
- 动画通过 `UAT_PlayAnimMontages` 编排；它订阅结束/取消以正确清理：

```490:495:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
UAT_PlayAnimMontages* Task = UAT_PlayAnimMontages::GetNewAbilityTask_PlayAnimMontages(this, ActionAnimMontageData);
OnEndAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
OnCancelAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
Task->ReadyForActivation();
```

### 战斗逻辑 (追踪、伤害、击退、特效)
- 攻击追踪由定时片段驱动。每个片段从沿前/右/上（或朝向 `TargetVector`）的偏移计算开始/结束，然后进行球体追踪以命中可伤害的 Actor，应用伤害并生成命中效果。

```55:253:Source/ProjectUSA/GAS/AT/AT_TraceAttack.cpp
void UAT_TraceAttack::AttackTraceAndSetNextTimer()
{
	...
	// Compute segment space
	FVector FinalAttackDirection = MyCharacter->GetActorForwardVector();
	...
	if (bIsDirectToTarget && TargetVector.Length() > SMALL_NUMBER) { FinalAttackDirection = TargetVector; ... }
	...
	// Single or multi trace against Pawn channel
	if (bIsUsingSigleTrace) { SphereTraceSingle(...); if (Hit) { ApplyDamageHitNiagaraEffect(...); TakeDamage(...); } }
	else { SphereTraceMulti(... each hit -> ApplyDamageHitNiagaraEffect + TakeDamage) }
	...
	// Schedule next segment
	GetWorld()->GetTimerManager().SetTimer(..., &UAT_TraceAttack::AttackTraceAndSetNextTimer, TimeDelay, false);
}
```

- 伤害应用在服务器端是权威的；执行团队击杀保护和无敌检查；成功时，通知客户端进行摄像机震动和特效。击退根据上下文（死亡/格挡/伤害、空中/地面）选择，并通过 GAS 激活。

```1835:1902:Source/ProjectUSA/Character/USACharacterBase.cpp
float AUSACharacterBase::TakeDamage(...)
{
	if (GetWorld()->GetAuthGameMode() == nullptr) { return 0; } // server only
	if (ASC && ASC->GetGameplayTagCount(USA_CHARACTER_STATE_INVINCIBLE) > 0) { return 0; }
	if (ASC && ASC->GetGameplayTagCount(USA_CHARACTER_STATE_DEAD) > 0) { return 0; }
	...
	// Team protection (same controller)
	if (MyAIController && EventAIController && (MyAIController != EventAIController)) { return 0; }
	if (MyPlayerController && EventPlayerController && (MyPlayerController != EventPlayerController)) { return 0; }
	...
	// Apply attribute damage (skip during parry)
	if (ASC && !ASC->HasMatchingGameplayTag(USA_CHARACTER_ACTION_PARRY)) {
		ASC->GetSet<UUSAAttributeSet>()->SetDamage(DamageAmount);
		MulticastRPC_TakeDamage(...); // camera shakes for causer
	}
	// Knockback ability selection + activation
	ApplyDamageMomentum(ResultDamageAmount, DamageEvent, EventInstigatorPawn, DamageCauser);
	ApplyDamageUSAJellyEffect(DamageEvent.DamageTypeClass);
	return ResultDamageAmount;
}
```

```1926:2078:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::ApplyDamageMomentum(...)
{
	// Skip if dead or shielded by armor; derive push direction from DamageType
	// Choose ability by context (death/parry/damage) and air/ground variants
	...
	MulticastRPC_ApplyDamageMomentum(NewDirection, DamageAbilityClass); // rotates actor and tries ability on server
}
```

### 动画处理
- 能力通过能力任务播放蒙太奇。起始片段可以根据游戏标签动态切换；标签添加/移除可以跳转到其他片段。在结束/取消时，配置的行为要么停止，要么播放结束蒙太奇。

```29:121:Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp
void UAT_PlayAnimMontages::Activate()
{
	...
	// Register section-switch delegates tied to gameplay tags
	if (ASC) {
		... ASC->RegisterGameplayTagEvent(TagAdded).AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagAdded);
		... ASC->RegisterGameplayTagEvent(TagRemoved).AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagRemoved);
	}
	// Play montage at selected section
	if (MyCharacter && MyCharacter->PlayAnimMontage(AnimMontage, Rate, StartSectionName)) { ... }
}
```

```123:180:Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp
void UAT_PlayAnimMontages::SimpleEndAbilityTask()
{
	// If configured, stop montage; otherwise optionally play an end montage
	if (MyCharacter && PlayAnimMontageData->AnimMontage && MyCharacter->GetCurrentMontage() == PlayAnimMontageData->AnimMontage)
	{
		if (bIsStopWhenFinished) { MyCharacter->StopAnimMontage(); }
		else if (IsValid(EndAnimMontage)) { MyCharacter->PlayAnimMontage(EndAnimMontage, AnimMontageRate); }
	}
	Super::SimpleEndAbilityTask();
}
```

### 攻击连击系统

连击系统是**游戏标签驱动**的，而不是僵化的状态机。它使用输入事件、蒙太奇片段切换和游戏标签来链接攻击。

#### 连击的输入处理

当玩家按下绑定到能力的输入时：

```938:974:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::InputPressGameplayAbilityByInputID(int32 InputID)
{
	FGameplayAbilitySpec* GameplayAbilitySpec = ASC->FindAbilitySpecFromInputID(InputID);
	
	if (GameplayAbilitySpec == nullptr) { return; }
	
	if (GameplayAbilitySpec->IsActive())
	{
		// Ability is already running - forward input press event
		ASC->AbilitySpecInputPressed(*GameplayAbilitySpec);
	}
	else
	{
		// Ability not active - try to activate it
		ASC->TryActivateAbility(GameplayAbilitySpec->Handle);
	}
}
```

**关键行为**：如果能力已经激活，则调用 `AbilitySpecInputPressed` 而不是激活新实例。这允许正在运行的能力响应输入按下以继续连击。

#### 连击窗口的游戏标签

连击窗口由 `Config/DefaultGameplayTags.ini` 中定义的游戏标签控制：

- `Character.Wait.Attack.ComboA00`、`ComboA01`、`ComboA02`、`ComboA03`（连击 A 链）
- `Character.Wait.Attack.ComboB00`、`ComboB01`、`ComboB02`、`ComboB03`（连击 B 链）

这些标签通常由蒙太奇中特定帧的**动画通知**添加/移除，以打开和关闭连击窗口。

#### 蒙太奇片段切换

`UAT_PlayAnimMontages` 注册游戏标签事件的监听器，并动态切换蒙太奇片段：

```70:91:Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp
UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();
FName StartSectionName = PlayAnimMontageData->StartAnimMontageSectionName;

if (ASC)
{
	// Register listeners for tag additions (combo confirm windows)
	TArray<FGameplayTag> KeyArray;
	PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded.GenerateKeyArray(KeyArray);
	for (const FGameplayTag TagAdded : KeyArray)
	{
		FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(TagAdded)
			.AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagAdded);
		DelegateHandles.Add({ TagAdded, DelegateHandle });
		
		// If tag already exists, use its mapped section as start
		if (ASC->HasMatchingGameplayTag(TagAdded))
		{
			StartSectionName = PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded[TagAdded];
		}
	}
	
	// Register listeners for tag removals (recovery/finisher sections)
	PlayAnimMontageData->AnimMontageSectionMapByGameplayTagRemoved.GenerateKeyArray(KeyArray);
	for (const FGameplayTag TagRemoved : KeyArray)
	{
		FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(TagRemoved)
			.AddUObject(this, &UAT_PlayAnimMontages::OnAnimSectionGameplayTagRemoved);
		DelegateHandles.Add({ TagRemoved, DelegateHandle });
	}
}
```

当在连击窗口期间添加连击标签时，蒙太奇切换到映射的片段：

```189:217:Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp
void UAT_PlayAnimMontages::OnAnimSectionGameplayTagAdded(FGameplayTag InTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		ACharacter* MyCharacter = Cast<ACharacter>(Ability->GetAvatarActorFromActorInfo());
		if (MyCharacter == nullptr) { return; }
		
		FName SectionName = NAME_None;
		if (PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded.Contains(InTag))
		{
			SectionName = PlayAnimMontageData->AnimMontageSectionMapByGameplayTagAdded[InTag];
		}
		
		// Stop current montage and play new section
		MyCharacter->StopAnimMontage();
		MyCharacter->PlayAnimMontage(
			PlayAnimMontageData->AnimMontage,
			PlayAnimMontageData->AnimMontageRate,
			SectionName);
	}
}
```

#### 连击数据结构

蒙太奇片段映射存储在 `FPlayAnimMontageData` 中：

```330:372:Source/ProjectUSA/Struct/USAStructs.h
struct FPlayAnimMontageData
{
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AnimMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AnimMontageRate = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartAnimMontageSectionName = NAME_None;
	
	// Maps gameplay tags to montage sections for combo continuation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FName> AnimMontageSectionMapByGameplayTagAdded;
	
	// Maps gameplay tag removals to recovery/finisher sections
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FName> AnimMontageSectionMapByGameplayTagRemoved;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsStopWhenFinished = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> EndAnimMontage;
};
```

#### 连击流程示例

1. **第一次攻击**：玩家按下攻击输入 → `GA_CharacterAction` 激活 → 从 `StartAnimMontageSectionName`（例如 "Attack1"）开始播放蒙太奇。
2. **连击窗口打开**：蒙太奇中的动画通知添加 `Character.Wait.Attack.ComboA00` 标签。
3. **玩家再次按下攻击**：由于能力已激活，调用 `AbilitySpecInputPressed`（但 `GA_CharacterAction` 不覆盖 `InputPressed`，因此通过标签处理）。
4. **标签驱动的片段切换**：当连击标签存在且玩家输入发生时（或另一个触发器添加下一个标签），`OnAnimSectionGameplayTagAdded` 触发 → 蒙太奇切换到映射的片段（例如 "Attack2"）。
5. **连击继续**：为后续连击步骤重复该过程（ComboA01 → Attack3 等）。
6. **连击窗口关闭**：如果在窗口期间没有输入，动画通知移除连击标签 → 如果已映射，通过 `OnAnimSectionGameplayTagRemoved` 切换到恢复片段。

#### 结束/取消窗口

连击能力可以使用 `UAT_WaitGameplayTagAdded/Removed` 任务在特定标签上结束或中断，从而在连击中途启用取消/闪避窗口：

```417:445:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (EndType)
{
	case ECharacterActionEndType::WaitTagAdded:
		WaitGameplayTagAdded = UAT_WaitGameplayTagAdded::GetNewAbilityTask_WaitGameplayTagAdded(this, EndGameplayTag);
		WaitGameplayTagAdded->Added.AddDynamic(this, &UGA_CharacterAction::SimpleEndAbility);
		WaitGameplayTagAdded->ReadyForActivation();
		break;
	// ... similar for WaitTagRemoved and WaitTime
}
```

#### 方向影响

每个连击步骤在激活时重新计算方向：

```113:261:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::CalculateTargetVector()
{
	// Recomputes TargetVector_Move based on DirectionType:
	// - Input: uses current input movement direction
	// - Target: uses direction toward locked target
	// - Custom: uses custom location
	// This allows direction-adjusted follow-ups in combos
}
```

#### 添加新的连击步骤

1. **添加蒙太奇片段**：在攻击蒙太奇中创建新片段（例如 "Attack4"）。
2. **定义游戏标签**：使用现有连击标签或在 `DefaultGameplayTags.ini` 中添加新标签。
3. **将标签映射到片段**：在能力的 `ActionAnimMontageData` 中，向 `AnimMontageSectionMapByGameplayTagAdded` 添加条目：`ComboA03` → `"Attack4"`。
4. **添加动画通知**：在前一个攻击片段中，添加通知以：
   - 在连击窗口开始时添加连击标签。
   - 在窗口结束时移除连击标签（或将移除映射到恢复片段）。
5. **可选**：配置结束/中断标签以用于取消窗口。

该系统是灵活的：连击可以根据不同的标签、时机或输入组合进行分支，所有这些都由游戏标签事件驱动，而不是硬编码的状态转换。

### 能力取消和恢复中断

能力取消系统允许新能力自动中断并取消当前激活的能力，从而实现恢复中断（后摇打断）机制。这对于响应式战斗至关重要，玩家可以通过输入新攻击来取消恢复动画。

#### 输入处理层

当玩家按下绑定到能力的输入时：

```938:974:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::InputPressGameplayAbilityByInputID(int32 InputID)
{
	FGameplayAbilitySpec* GameplayAbilitySpec = ASC->FindAbilitySpecFromInputID(InputID);
	
	if (GameplayAbilitySpec == nullptr) { return; }
	
	if (GameplayAbilitySpec->IsActive())
	{
		// Ability is already running - forward input press event
		ASC->AbilitySpecInputPressed(*GameplayAbilitySpec);
	}
	else
	{
		// Ability not active - try to activate it
		ASC->TryActivateAbility(GameplayAbilitySpec->Handle);
	}
}
```

**关键行为**：如果能力已经激活，则调用 `AbilitySpecInputPressed`（用于连击继续）。否则，调用 `TryActivateAbility`，这会触发 GAS 自动取消系统。

#### GAS 自动取消系统

虚幻引擎的 GAS 系统在调用 `TryActivateAbility` 时自动处理能力取消。取消基于**游戏标签**：

1. **CancelAbilitiesWithTag**：新能力的 `CancelAbilitiesWithTag` 属性中指定的标签
2. **AbilityTags**：分配给当前激活能力的标签

当新能力（例如 A01）尝试激活时：
- GAS 检查 A01 的 `CancelAbilitiesWithTag` 是否包含与 A00 的 `AbilityTags` 匹配的任何标签
- 如果匹配，GAS 在激活 A01 之前自动调用 A00 的 `CancelAbility`
- 这是**自动**发生的 - 不需要额外的代码

#### 取消流程

当 A01 激活并取消 A00 时：

1. **接收输入**：玩家按下 X 键 → 调用 `InputPressGameplayAbilityByInputID`
2. **TryActivateAbility**：调用 `ASC->TryActivateAbility(A01->Handle)`
3. **GAS 检查取消**：GAS 检测到 A01 的 `CancelAbilitiesWithTag` 与 A00 的 `AbilityTags` 匹配
4. **取消 A00**：GAS 自动调用 `A00->CancelAbility(...)`
5. **A00 清理**：
   - 调用 `UGA_CharacterAction::CancelAbility`
   - `ResetArmorAttributeToBase()` 将护甲恢复到基础值
   - `OnCancelAbility` 委托触发（清理任务如停止蒙太奇）
   - 应用 `CancelAbilityEffects`
6. **激活 A01**：A00 取消后，A01 正常激活

#### CancelAbility 实现

取消清理在 `UGA_CharacterAction::CancelAbility` 中处理：

```87:96:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	// Restore armor attribute to base value
	ResetArmorAttributeToBase();

	// Execute blueprint-defined cancel behavior
	K2_DoSomething_Cancel();
}
```

基础 `UUSAGameplayAbility::CancelAbility` 处理：
- 应用 `CancelAbilityEffects`（取消前）
- 广播 `OnCancelAbility` 委托（清理任务订阅此委托）
- 调用父级 `CancelAbility`（GAS 清理）
- 应用 `PostCancelAbilityEffects`（取消后）

```74:87:Source/ProjectUSA/GAS/GA/USAGameplayAbility.cpp
void UUSAGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	ApplyEffectsViaArray(CancelAbilityEffects, Handle, ActorInfo, ActivationInfo);

	OnCancelAbility.Broadcast();
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	ApplyEffectsViaArray(PostCancelAbilityEffects, Handle, ActorInfo, ActivationInfo);
}
```

#### 取消时的任务清理

当广播 `OnCancelAbility` 时，能力任务自动清理。例如，动画任务停止蒙太奇：

```490:495:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
UAT_PlayAnimMontages* Task = UAT_PlayAnimMontages::GetNewAbilityTask_PlayAnimMontages(this, ActionAnimMontageData);
OnEndAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
OnCancelAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
Task->ReadyForActivation();
```

当能力被取消时，`OnCancelAbility` 触发 → 调用 `SimpleEndAbilityTask` → 停止蒙太奇。

#### 配置（蓝图设置）

要在能力之间启用恢复中断：

1. **在 A00 上设置 AbilityTags**：在 A00 的能力蓝图中，分配唯一标签（例如 `Character.Action.Attack.A00`）
2. **在 A01 上设置 CancelAbilitiesWithTag**：在 A01 的能力蓝图中，将 A00 的标签添加到 `CancelAbilitiesWithTag`
3. **结果**：当 A01 激活时，它自动取消 A00

**配置示例**：
- A00: `AbilityTags` = `[Character.Action.Attack.A00]`
- A01: `CancelAbilitiesWithTag` = `[Character.Action.Attack.A00]`
- 当玩家在 A00 的恢复期间按下 X 时，A01 激活并取消 A00

#### 总结

恢复中断系统通过以下方式工作：

1. **代码层**：`USACharacterBase::InputPressGameplayAbilityByInputID` 调用 `TryActivateAbility`
2. **系统层**：虚幻引擎的 GAS 基于标签自动取消冲突的能力
3. **配置层**：`AbilityTags` 和 `CancelAbilitiesWithTag` 的蓝图设置

这种设计允许灵活的、标签驱动的取消，而无需硬编码能力关系。任何能力都可以通过匹配标签来取消任何其他能力，从而实现具有多个取消路径的复杂战斗系统。

### 网络模型
- 客户端计算目标向量并将其发送到服务器以进行权威复制（`ServerRPC_SetTargetVectorAndDoSomething`）；客户端还运行其本地版本以实现响应性。
- 伤害、击退选择和物品/武器复制是服务器驱动的，具有用于视觉效果和摄像机行为的多播更新。
- `AUSACharacterBase` 中的角色动画 RPC 确保网络游戏中蒙太奇状态的一致性。

```372:385:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::ServerRPC_PlayAnimMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	MulticastRPC_PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}
```

### 整体流程
1. 玩家按下绑定到能力 InputID 的输入。
2. `ASC->TryActivateAbility` 运行 `UUSAGameplayAbility`（例如，角色动作、冲刺、攻击）。
3. 能力计算目标/移动向量，旋转角色，播放蒙太奇，并生成攻击/追踪任务。
4. `UAT_TraceAttack` 应用定时追踪 → 伤害/特效。服务器验证并在需要时触发击退能力。
5. 游戏标签驱动中断/结束窗口和摄像机/移动调整。

### 关键文件
- `Source/ProjectUSA/Character/USACharacterBase.cpp`：输入绑定、GAS 初始化、标签驱动的角色调整、伤害和击退、动画 RPC。
- `Source/ProjectUSA/Character/USACharacterPlayer.cpp`：玩家特定的输入缩放、目标锁定系统、摄像机管理。
- `Source/ProjectUSA/GAS/GA/USAGameplayAbility.cpp`：能力生命周期、目标向量客户端/服务器流程、效果应用辅助函数。
- `Source/ProjectUSA/GAS/AT/AT_TraceAttack.cpp`：定时攻击追踪、命中处理、伤害和特效分发。
- `Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp`：蒙太奇播放，具有游戏标签驱动的片段切换和清理。

有关更深层的能力编排（移动、中断、属性护盾），另请参阅 `Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp`。
