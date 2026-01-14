# Migration Workflow: Sekiro Remake logic to ProjectUSA (GAS)

## Migration Status

**Overall Progress**: 100% Complete ✅

**Completed Systems**:
- ✅ Core Character & Attributes (Posture System)
- ✅ Advanced Combat (Deflect, Attack, Execution, Damage Calculation)
- ✅ Movement System (Dash, Sprint)
- ✅ Input Buffering & GAS Integration
- ✅ Hit Reaction System (Ground/Air, Knockdown, Get-Up)
- ✅ Animation Notifies (Attack Windows, Parry Timing)
- ✅ Jump System (8-directional with state progression)

**Migration Complete!** All Sekiro Remake features have been successfully ported to ProjectUSA's GAS framework.

---

This document outlines the workflow for migrating gameplay logic from the `Sekiro_Remake` repository to the `ProjectUSA` GAS framework.

## 1. Environment Setup

To continue work on a new machine:

1.  **Clone Repositories**:
    *   Ensure both `ProjectUSA` and `Sekiro_Remake` are checked out.
    *   **Sekiro_Remake**: This is the reference repository. Ensure you have the latest code:
        ```bash
        cd Actions/Sekiro_Remake
        git pull origin main
        ```
    *   **ProjectUSA**: This is the target working repository.
    *   Recommended Structure:
        ```text
        MyProject/
        ├── Actions/
        │   ├── ProjectUSA/   (Main GAS Framework - Work Here)
        │   └── Sekiro_Remake/(Reference Repo - Read Only)
        ```

2.  **Open Project**:
    *   Open `ProjectUSA.uproject` to implement changes.
    *   Open `Sekiro_Remake` code in a separate editor window for reference.

3.  **Track Progress**:
    *   Task tracking: `Docs/SekiroMigrationTasks.md`
    *   Implementation plan: `Docs/SekiroImplementationPlan.md`
    *   These files are version controlled - commit changes to sync progress across machines.

## 2. Reference Guide

Key locations in `Sekiro_Remake` containing logic to be ported:

*   **FSM (Finite State Machine)**: `Source/Sekiro_Remake/FSM/`
    *   `DeflectState.cpp`: Logic for parrying/blocking.
    *   `AttackState.cpp`: Attack combos and input handling during attacks.
    *   `SekiroCharacter.cm`: Core character variables (Posture, Health).
*   **Animation**: `Source/Sekiro_Remake/AnimNotify/`
    *   Look for notifies that trigger gameplay events (e.g., `AnimNotify_DeflectWindow`).

## 3. Migration Tasks & Architecture Mapping

We are mapping a custom FSM system to Unreal's Gameplay Ability System (GAS).

### A. Core Attributes (Posture System)
**Goal**: Replicate Sekiro's Posture bar.
*   **Sekiro**: `float m_Posture` in `SekiroCharacter`.
*   **ProjectUSA**:
    1.  Modify `USAAttributeSet.h`.
    2.  Add `CurrentPosture` and `MaxPosture`.
    3.  Implement `PostGameplayEffectExecute` to handle posture break logic.

### B. Character Class
**Goal**: A specialized character for Sekiro gameplay.
*   **Sekiro**: `ASekiroCharacter`.
*   **ProjectUSA**:
    1.  Create `ASekiroHeroCharacter` inheriting from `AUSACharacterPlayer`.
    2.  Override `SetupGAS` to grant Sekiro-specific abilities on possess.

### C. Combat Logic (Abilities)

| Mechanic | Sekiro Implementation | ProjectUSA (GAS) Implementation |
| :--- | :--- | :--- |
| **Deflect/Parry** | `DeflectState` | **GA_SekiroDeflect**: A GameplayAbility activated by Input. Applies a "Blocking" Tag. Uses `WaitGameplayEvent` to detect hits for Perfect Parry. |
| **Attack** | `AttackState` | **GA_SekiroAttack**: Uses `PlayMontageAndWait`. Logic for combo branching moves to the Ability graph or next-combo-check logic. |
| **Input Buffering** | `PreInputComponent` | **GAS Input Queuing**: Standard GAS input handling or a custom AbilityTask for input windows. |

## 4. Work Process

1.  **Analyze**: Read the specific State class in `Sekiro_Remake`.
2.  **Implement**: Create the corresponding GameplayAbility in `ProjectUSA`.
3.  **Verify**:
    *   Does the ability play the animation?
    *   Does it apply the correct Tags (e.g., `State.Defending`)?
    *   Does it modify Attributes (Stamina/Posture) correctly?

## 5. Directory Structure in ProjectUSA

New files should be placed in:
*   `Source/ProjectUSA/Character/SekiroHeroCharacter.h`
*   `Source/ProjectUSA/GAS/GA/Sekiro/` (For all Sekiro-related abilities)
*   `Source/ProjectUSA/GAS/Sekiro/` (For execution calculations)

## 6. Blueprint Configuration

After C++ implementation, configure in Blueprint:

### A. Create SekiroHeroCharacter Blueprint
1.  Create Blueprint `BP_SekiroHero` based on `ASekiroHeroCharacter`.
2.  In **GameplayAbilities_Start**, add:
    *   `GA_SekiroPostureRecovery` (passive, auto-activates)
3.  In **GameplayAbilities_Active**, add with input bindings:
    *   `GA_SekiroDeflect` (bind to RMB or Block button)
    *   `GA_SekiroAttack` (bind to LMB or Attack button)
    *   `GA_SekiroExecution` (bind to E or Execution button)

### B. Create Damage GameplayEffect
1.  Create Blueprint `GE_SekiroDamage` based on `GameplayEffect`.
2.  Set **Execution Class** to `SekiroDamageExecution`.
3.  Configure **Modifiers** to capture `Damage` from Source.

### C. Configure Gameplay Tags
Add these tags to your project's GameplayTags:
*   `State.Blocking` - Applied during deflect
*   `State.PerfectParry` - Applied during perfect parry window
*   `State.PostureBroken` - Applied when posture >= max
*   `GameplayEvent.Combat.Hit` - Sent when character is hit

### D. Set Tag Values in Abilities
In each ability Blueprint (or C++ default properties):
*   `GA_SekiroDeflect`: Set `BlockingTag`, `PerfectParryTag`, `HitEventTag`
*   Configure montages and timing values

