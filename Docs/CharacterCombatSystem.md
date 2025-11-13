## Character Combat System (C++)

This document summarizes how character combat works across inputs, character operation, animation, and combat logic, focusing on the primary C++ sources.

### Input → Ability Activation
- Player inputs are bound via Enhanced Input in `AUSACharacterBase::SetupPlayerInputComponent`, including ability inputs with an `InputID`. On press/release, it finds the corresponding `FGameplayAbilitySpec` and either tries to activate it or forwards input events.

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

- Player movement/look inputs compute world directions from camera yaw for responsive control.

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

### Character Operation (GAS setup, gameplay tags, targeting)
- GAS is initialized in `PossessedBy` (server) and replicated to clients; abilities are granted in `PostSetupGAS`. Gameplay tag event handlers toggle locomotion flags and constraints.

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

- Tag callbacks adjust movement, input, and state (e.g., ignore input, velocity zero, crouch/fall/dead).

```1228:1243:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::OnGameplayTagCallback_IgnoreMoveInput(const FGameplayTag CallbackTag, int32 NewCount)
{
	...
	if (NewCount > 0) { GetController()->SetIgnoreMoveInput (true); }
	else { GetController()->ResetIgnoreMoveInput (); }
}
```

- Targeting: player maintains a target via overlap scoring of candidates in front of the camera; toggled by `LookTarget()`, with lock-on camera management.

```367:472:Source/ProjectUSA/Character/USACharacterPlayer.cpp
void AUSACharacterPlayer::SetCurrentTargetableActorUsingForwardVector(const FVector& InDirection, TObjectPtr<class AActor>& InOutTargetActorPointer)
{
	// Overlap in range, score by distance and dot with forward/camera vector
	// Choose best result as the current target
	...
}
```

### Ability Orchestration (Core)
- `UUSAGameplayAbility` provides a common pattern: apply effects on activate/cancel/end, calculate target vectors, and handle client-server flow for target-vector-based abilities.

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

### Character Action Ability (Battle Logic Core)
The main melee action ability sequences movement, animation, VFX spawns, and attack traces, plus end/interrupt logic. It also temporarily adjusts armor and supports magnet/snap-to-target movement.

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

Target vectors are computed with priority for target-facing and input, and magnet distance considers both capsule radii plus a configurable gap:

```113:196:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::CalculateTargetVector()
{
	// Updates instant target; determines TargetVector_Attack (towards target)
	// Picks TargetVector_Move via: Move-to-target, Custom, Input, or Target direction
	// Magnet TargetDistance = max(distance - targetRadius - selfRadius - MoveToTargetGap, 0)
}
```

Execution outline:
- Rotate to `TargetVector_Move`; preserve spring arm transform to avoid camera pops.
- Movement:
  - Magnet move with `UAT_MoveToLocationByVelocity` to computed `EndLocation` and optional after-velocity.
  - Or one of Move/Walk/Launch/Custom branches using the corresponding movement/launch/change-info tasks.
- End/Interrupt:
  - End via `UAT_WaitGameplayTagAdded/Removed` or `UAT_WaitDelay` (time-based).
  - Interrupt via a parallel WaitTag task depending on `InterruptType` and `InterruptGameplayTag`.
- Server-side:
  - `UAT_SpawnActors` for FX or hitboxes.
  - `UAT_TraceAttack` for timed traces using `AttackTraceData` and `TargetVector_Attack`.
- Animation:
  - `UAT_PlayAnimMontages` plays montage and supports dynamic section switching by gameplay tags.

```264:495:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::DoSomethingWithTargetVector()
{
	// Rotate → (Magnet/Move/Walk/Launch/Custom) → End/Interrupt → Server spawn/trace → Play montage
}
```

Temporary armor is applied on start and restored on end/cancel:

```522:564:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
void UGA_CharacterAction::AddArmorAttributeFromBase(float InAddArmor) { ... }
void UGA_CharacterAction::ResetArmorAttributeToBase() { ... }
```

#### Movement Types and Direction Selection
- **Movement types** handled in action execution:
  - **Magnet/Snap to Target** when within range and not using Custom move. Computes `EndLocation` from `TargetVector_Move * TargetDistance` and applies optional after velocity.
  - **Move**: offset from current location with `MoveOffsetLocation` and `MoveAfterVelocity`.
  - **Walk**: temporarily overrides movement settings via `UAT_ChangeCharacterMovementInfo` for the action duration.
  - **Launch**: launches with configured vector and XY/Z overrides for a period.
  - **Custom**: uses precomputed `TargetVector_Move` and `TargetDistance` with custom curves and after velocity.

```350:409:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
// Magnet move if TargetDistance > 0 and not Custom, else switch on MoveType:
// Move → UAT_MoveToLocationByVelocity
// Walk → UAT_ChangeCharacterMovementInfo
// Launch → UAT_LaunchCharacterForPeriod
// Custom → UAT_MoveToLocationByVelocity with custom data
```

- **Direction selection**: `TargetVector_Move` is chosen from Input, Target, Custom, or Forward depending on `DirectionType` and runtime inputs/lock-on. The attack vector faces the current target when available.

```211:261:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (DirectionType) {
  case Input: prefer input movement direction or pending input; fallback to forward
  case Target: prefer target direction; fallback to input or pending input; else forward
  case None/default: leave as-is
}
```

#### End and Interrupt Types
- **End conditions** (one of):
  - Wait for a gameplay tag to be added, or removed.
  - Wait a fixed time delay.

```417:445:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (EndType) {
  case WaitTagAdded: UAT_WaitGameplayTagAdded → SimpleEndAbility
  case WaitTagRemoved: UAT_WaitGameplayTagRemoved → SimpleEndAbility
  case WaitTime/default: UAT_WaitDelay → SimpleEndAbility
}
```

- **Interrupt conditions** mirror the above and run in parallel to end logic:

```448:475:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
switch (InterruptType) {
  case WaitTagAdded: UAT_WaitGameplayTagAdded → SimpleEndAbility
  case WaitTagRemoved: UAT_WaitGameplayTagRemoved → SimpleEndAbility
  case WaitTime/default: // none
}
```

#### Camera Preservation During Rotation
- The ability rotates the character to face `TargetVector_Move` but preserves the spring arm transform to prevent camera pops during the frame:

```306:326:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
USpringArmComponent* SpringArm = MyCharacter->GetComponentByClass<USpringArmComponent>();
FTransform Saved = SpringArm ? SpringArm->GetComponentTransform() : FTransform::Identity;
MyCharacter->SetActorRotation(ForwardDirection.Rotation());
MyCharacter->UpdateComponentTransforms();
if (SpringArm) { SpringArm->SetWorldTransform(Saved); }
```

#### Server-only Execution (Spawns and Traces)
- Spawning and attack traces are authoritative; on server (or standalone), the ability spawns configured actors and runs timed trace attacks using the computed `TargetVector_Attack`:

```477:488:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
if (IsServerOrStandalone) {
  UAT_SpawnActors::GetNewAbilityTask_SpawnActors(..., TargetVector_Attack)->ReadyForActivation();
  UAT_TraceAttack::GetNewAbilityTask_TraceAttack(..., TargetVector_Attack)->ReadyForActivation();
}
```

#### Montage Playback
- Animation is orchestrated through `UAT_PlayAnimMontages`; it is subscribed to end/cancel to clean up properly:

```490:495:Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp
UAT_PlayAnimMontages* Task = UAT_PlayAnimMontages::GetNewAbilityTask_PlayAnimMontages(this, ActionAnimMontageData);
OnEndAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
OnCancelAbility.AddDynamic(Task, &UAT_PlayAnimMontages::SimpleEndAbilityTask);
Task->ReadyForActivation();
```

### Combat Logic (Traces, damage, knockback, VFX)
- Attack traces are driven by timed segments. Each segment computes a start/end from offsets along forward/right/up (or towards a `TargetVector`), then sphere-traces to hit damageable actors, applying damage and spawning hit effects.

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

- Damage application is authoritative on the server; team-kill protection and invincibility checks are performed; on success, clients are notified for camera shakes and VFX. Knockback is selected by context (death/parry/damage, air/ground) and activated via GAS.

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

### Animation Handling
- Abilities play montages via an ability task. Start section can switch dynamically based on gameplay tags; tag add/remove can jump to other sections. On end/cancel, configured behavior either stops or plays an end montage.

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

### Attack Combo System

The combo system is **gameplay tag-driven** rather than a rigid state machine. It uses input events, montage section switching, and gameplay tags to chain attacks together.

#### Input Handling for Combos

When a player presses an input bound to an ability:

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

**Key behavior**: If the ability is already active, `AbilitySpecInputPressed` is called instead of activating a new instance. This allows the running ability to react to input presses for combo continuation.

#### Gameplay Tags for Combo Windows

Combo windows are controlled by gameplay tags defined in `Config/DefaultGameplayTags.ini`:

- `Character.Wait.Attack.ComboA00`, `ComboA01`, `ComboA02`, `ComboA03` (Combo A chain)
- `Character.Wait.Attack.ComboB00`, `ComboB01`, `ComboB02`, `ComboB03` (Combo B chain)

These tags are typically added/removed by **anim notifies** in the montage at specific frames to open and close combo windows.

#### Montage Section Switching

`UAT_PlayAnimMontages` registers listeners for gameplay tag events and switches montage sections dynamically:

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

When a combo tag is added during a combo window, the montage switches to the mapped section:

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

#### Combo Data Structure

The montage section mappings are stored in `FPlayAnimMontageData`:

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

#### Combo Flow Example

1. **First attack**: Player presses attack input → `GA_CharacterAction` activates → plays montage starting at `StartAnimMontageSectionName` (e.g., "Attack1").
2. **Combo window opens**: An anim notify in the montage adds `Character.Wait.Attack.ComboA00` tag.
3. **Player presses attack again**: Since ability is active, `AbilitySpecInputPressed` is called (but `GA_CharacterAction` doesn't override `InputPressed`, so this is handled via tags).
4. **Tag-driven section switch**: When the combo tag is present and the player input occurs (or another trigger adds the next tag), `OnAnimSectionGameplayTagAdded` fires → montage switches to the mapped section (e.g., "Attack2").
5. **Combo continues**: Process repeats for subsequent combo steps (ComboA01 → Attack3, etc.).
6. **Combo window closes**: If no input during window, an anim notify removes the combo tag → if mapped, switches to recovery section via `OnAnimSectionGameplayTagRemoved`.

#### End/Cancel Windows

Combo abilities can use `UAT_WaitGameplayTagAdded/Removed` tasks to end or interrupt on specific tags, enabling cancel/dodge windows mid-combo:

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

#### Directional Influence

Each combo step recalculates direction on activation:

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

#### Adding a New Combo Step

1. **Add montage section**: Create a new section in the attack montage (e.g., "Attack4").
2. **Define gameplay tag**: Use existing combo tags or add new ones in `DefaultGameplayTags.ini`.
3. **Map tag to section**: In the ability's `ActionAnimMontageData`, add entry to `AnimMontageSectionMapByGameplayTagAdded`: `ComboA03` → `"Attack4"`.
4. **Add anim notifies**: In the previous attack section, add notifies to:
   - Add the combo tag at the start of the combo window.
   - Remove the combo tag at the end of the window (or map removal to a recovery section).
5. **Optional**: Configure end/interrupt tags for cancel windows.

The system is flexible: combos can branch based on different tags, timing, or input combinations, all driven by gameplay tag events rather than hardcoded state transitions.

### Networking Model
- Client computes target vector and sends it to server for authoritative replication (`ServerRPC_SetTargetVectorAndDoSomething`); the client also runs its local version for responsiveness.
- Damage, knockback choice, and item/weapon replication are server-driven with multicast updates for visual effects and camera behavior.
- Character animation RPCs in `AUSACharacterBase` ensure consistent montage state in networked games.

```372:385:Source/ProjectUSA/Character/USACharacterBase.cpp
void AUSACharacterBase::ServerRPC_PlayAnimMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	MulticastRPC_PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}
```

### Putting It Together
1. Player presses an input bound to an ability InputID.
2. `ASC->TryActivateAbility` runs a `UUSAGameplayAbility` (e.g., character action, dash, attack).
3. Ability computes target/move vectors, rotates character, plays montage, and spawns attack/trace tasks.
4. `UAT_TraceAttack` applies timed traces → damage/VFX. Server validates and triggers knockback abilities if needed.
5. Gameplay tags drive interrupt/end windows and camera/locomotion adjustments.

### Key Files
- `Source/ProjectUSA/Character/USACharacterBase.cpp`: Input bindings, GAS initialization, tag-driven character adjustments, damage and knockback, animation RPCs.
- `Source/ProjectUSA/Character/USACharacterPlayer.cpp`: Player-specific input scaling, targeting system, camera management.
- `Source/ProjectUSA/GAS/GA/USAGameplayAbility.cpp`: Ability lifecycle, target-vector client/server flow, effect application helpers.
- `Source/ProjectUSA/GAS/AT/AT_TraceAttack.cpp`: Timed attack traces, hit processing, damage and VFX dispatch.
- `Source/ProjectUSA/GAS/AT/AT_PlayAnimMontages.cpp`: Montage playback with gameplay tag–driven section switching and cleanup.

For deeper ability choreography (movement, interrupts, attribute shields), also see `Source/ProjectUSA/GAS/GA/GA_CharacterAction.cpp`.


