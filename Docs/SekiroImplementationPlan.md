# Migration Plan: Sekiro Remake Logic to ProjectUSA (GAS)

## Goal Description
Migrate the core gameplay logic from `Sekiro_Remake` (custom FSM-based) to `ProjectUSA` (GAS-based). This involves identifying the "Sekiro" specific mechanics (Posture system, Deflect/Parry combat loop) and reimplementing them using Gameplay Attributes, Gameplay Abilities, and Gameplay Effects within the existing `ProjectUSA` framework.

## User Review Required
> [!IMPORTANT]
> **Attribute Set Changes**: I will be modifying `USAAttributeSet.h` to include `Posture` and `MaxPosture`. This is a core change to the base attribute set.
> **Input Mapping**: I will assume functionality needs to be mapped to existing inputs or new ones.

## Proposed Changes

### GAS Core
#### [MODIFY] [USAAttributeSet](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/AttributeSet/USAAttributeSet.h)
- Add `CurrentPosture` and `MaxPosture` attributes.
- Add `PostureRecoverRate` attribute (optional, but good for Sekiro logic).

### Character
#### [NEW] [SekiroHeroCharacter](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/Character/SekiroHeroCharacter.h)
- Create a new character class `ASekiroHeroCharacter` inheriting from `AUSACharacterPlayer`.
- This class will override `SetupGAS` to grant Sekiro-specific abilities.
- It will likely handle the `AnimInstance` linkage if needed (though GAS usually drives this via Tags/Montages).

### Gameplay Abilities (Combat)
#### [NEW] [GA_SekiroDeflect](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroDeflect.h)
- Implements the "Block" and "Parry" logic.
- Activates on input.
- Applies `Status.State.Blocking` tag.
- Listens for `GameplayEvent.Combat.Hit` (or similar) to check for "Perfect Parry" timing window.
- Replaces `DeflectState` and `PerfectParryTiming` logic.

#### [NEW] [GA_SekiroAttack](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroAttack.h)
- Implements the attack chain.
- Uses `AT_PlayAnimMontages` (existing capability).
- Replaces `AttackState`.

### Documentation
#### [NEW] [MigrationWorkflow](file:///e:/MyProject/Actions/ProjectUSA/Docs/MigrationWorkflow.md)
- detailed guide on how to reference `Sekiro_Remake` code.
- Steps to migrate logic to `ProjectUSA`.
- Instructions for setting up the environment on a new machine.

### Combat Calculations
#### [NEW] [SekiroDamageExecution](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/Sekiro/SekiroDamageExecution.h)
- Custom `UGameplayEffectExecutionCalculation` to handle Sekiro's damage logic.
- Logic: If target is "Defending", redirect damage to Posture. If "Perfect Parry", redirect damage back to attacker or nullify.
- Posture damage scales based on target's remaining Health (lower health = slower posture recovery, more posture damage).

### Posture System
#### [MODIFY] [USAAttributeSet](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/AttributeSet/USAAttributeSet.cpp)
- Implement ticking posture recovery in `PostGameplayEffectExecute` or via a passive `GameplayAbility`.

### Combat Abilities
#### [NEW] [GA_SekiroExecution](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroExecution.h)
- Ability to perform a "Deathblow" when target's Posture is broken.

---

### Movement System
#### [NEW] [GA_SekiroDash](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroDash.h)
- Quick dash ability with directional input.
- Uses `AT_LaunchCharacterForPeriod` or custom movement task.
- Applies invincibility frames via `State.Dashing` tag.

#### [NEW] [GA_SekiroSprint](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroSprint.h)
- Toggle sprint mode (increases movement speed).
- Modifies `CharacterMovement->MaxWalkSpeed` while active.

---

### Jump System
#### [NEW] [GA_SekiroJump](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroJump.h)
- Replaces default jump with Sekiro-style directional jump.
- Handles Jump Ready -> Start -> Loop -> End state progression.
- Supports 8-directional jump based on input.

---

### Hit Reaction System
#### [NEW] [GA_SekiroHitReaction](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/GAS/GA/Sekiro/GA_SekiroHitReaction.h)
- Triggered by damage events.
- Selects appropriate reaction based on:
  - Damage type (light/heavy/knockdown)
  - Character state (grounded/airborne)
  - Hit direction
- Plays corresponding montage and applies stun duration.

---

### Input Buffering
#### [NEW] [USekiroInputBufferComponent](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/Component/SekiroInputBufferComponent.h)
- Component that queues input during ability execution.
- Integrates with GAS by checking buffer before ability activation.
- Configurable buffer window (default 0.3s).

---

### Animation Notifies
#### [NEW] [AnimNotify_SekiroAttackWindow](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/AnimNotify/AnimNotify_SekiroAttackWindow.h)
- Marks frames where attack collision is active.
- Sends GameplayEvent to trigger damage application.

#### [NEW] [AnimNotify_SekiroParryWindow](file:///e:/MyProject/Actions/ProjectUSA/Source/ProjectUSA/AnimNotify/AnimNotify_SekiroParryWindow.h)
- Marks perfect parry timing windows.
- Adds/removes `State.PerfectParry` tag.

## Verification Plan

### Automated Tests
- None available currently in the project.

### Manual Verification
1.  **Compile**: Ensure the project compiles with the new AttributeSet and Character class.
2.  **Blueprint Setup**:
    - Create a Blueprint based on `ASekiroHeroCharacter`.
    - Create a GE (GameplayEffect) to initialize Attributes (Health, Posture).
    - Create GAs (GameplayAbilities) for Deflect and Attack.
3.  **Play Test**:
    - Spawn as the new Sekiro Character.
    - Check if `Posture` attribute exists on the debug view (`showdebug abilitysystem`).
    - Verify "Deflect" ability plays the animation and applies the Blocking tag.
