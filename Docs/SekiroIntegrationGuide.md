# Sekiro 系统集成与配置指南 (ProjectUSA)

本文档旨在指导开发者如何将已实现的 Sekiro C++ 核心功能接入到 `ProjectUSA` 的游戏流程中，使其能够完整运行并达到预期的战斗体验。

---

## 1. 角色 Blueprint 配置 (Character Setup)

### 1.1 创建 BP 类
- 基于 `ASekiroHeroCharacter` 创建一个新的 Blueprint（例如 `BP_SekiroHero`）。

### 1.2 组件初始化
- 检查组件列表，确保 `InputBufferComponent` 和 `TargetLockComponent` 已成功挂载。
- 在 `CapsuleComponent` 中配置碰撞通道，建议设置专门的 `Pawn` 或 `Character` 通道。

### 1.3 属性初始化 (Gameplay Effects)
- 创建目录：`/Game/ProjectUSA/GAS/Attributes/`。
- 创建 `GameplayEffect` 命名为 `GE_Sekiro_InitAttributes`：
  - **Duration Policy**: Instant
  - **Modifiers**:
    - `USAAttributeSet.MaxHealth`: 100
    - `USAAttributeSet.CurrentHealth`: 100
    - `USAAttributeSet.MaxPosture`: 100
    - `USAAttributeSet.CurrentPosture`: 0
    - `USAAttributeSet.PostureRecoverRate`: 10
    - `USAAttributeSet.MaxSpiritEmblems`: 15
    - `USAAttributeSet.MaxResurrectionPower`: 3.0
- 在 `BP_SekiroHero` 的 `BeginPlay` 中应用此 GE，或配置在初始属性集里。

---

## 2. 技能赋予 (Ability Granting)

在 `BP_SekiroHero` 的 **GameplayAbilities_Start** 数组中，手动添加以下已实现的 C++ 技能类：

| 技能名称 | 类名 (C++) | 作用 |
| :--- | :--- | :--- |
| **基础攻击** | `GA_SekiroAttack` | 处理普通攻击连招 |
| **防御与招架** | `GA_SekiroDeflect` | 点击防御，长按格挡，判定完美弹刀 |
| **垫步/闪避** | `GA_SekiroDash` | 快速位移，提供短暂无敌帧 |
| **冲刺** | `GA_SekiroSprint` | 提升移动速度 |
| **跳跃** | `GA_SekiroJump` | 8 方向跳跃逻辑 |
| **击破处决** | `GA_SekiroExecution` | 针对架势崩坏敌人的忍杀 |
| **被动架势恢复** | `GA_SekiroPostureRecovery` | 实现架势值随时间自动衰减 |
| **被击反应** | `GA_SekiroHitReaction` | 自动根据受击事件播放对应级别的受击动画 |
| **起死回生** | `GA_SekiroResurrection` | 死亡后的复活逻辑 |

---

## 3. 动画与 Montage 集成

### 3.1 攻击判定 (GA_SekiroAttack)
- 在攻击 Montage 的活跃帧添加 `AnimNotify_SekiroAttackWindow`。
- 配置参数：`EventTag` = `GameplayEvent.Combat.Hit`。

### 3.2 招架判定 (GA_SekiroDeflect)
- 在招架动画（通常是非常短的前摇）中添加 `AnimNotify_SekiroParryWindow`。
- 配置：`ParryTag` = `State.PerfectParry`，`bIsStarting` 控制在特定时间段内开启判定。

### 3.3 受击动画 (GA_SekiroHitReaction)
- 需要为 `SekiroHeroCharacter` 准备一个 DataAsset 或在 BP 中配置受击 Montage 列表。
- 逻辑会自动根据 `SekiroDamageExecution` 传递的伤害数值（1=轻, 2=中, 3=重, 4=击飞）选择对应的 Montage。

---

## 4. 核心 GameplayTags 配置

确保项目设置中定义了以下标签，否则检测逻辑将失败：

- `State.Blocking`: 角色正处于按住防御的状态。
- `State.PerfectParry`: 角色处于完美弹刀的判定窗口。
- `State.PostureBroken`: 架势条已满，处于可被处决状态。
- `State.Crouching`: 角色正在下蹲。
- `State.NoResurrect`: 角色背负黑印，无法连续复活。
- `Attack.Perilous.Thrust`: 当前攻击是危险的“突刺”。
- `Attack.Perilous.Sweep`: 当前攻击是危险的“下段横扫”。
- `GameplayEvent.UI.DangerNotice`: 触发“危”字显示的 UI 事件。

---

## 5. 输入映射

- 将鼠标左键绑定到 `AttackAction`，并触发 `InputPressGameplayAbilityByInputID`（通过 ID 分发）。
- 将鼠标右键绑定到 `DeflectAction`。
- **注意**: 利用 `SekiroInputBufferComponent`。如果在一串攻击中连续点击，输入会被缓存，角色会在上一个动画结束后自动执行下一个缓存的动作。

---

## 6. UI 与反馈

### 6.1 架势条渲染
- 建议制作一个 `WBP_PostureBar`。
- 使用 `GetAttributeValue(UUSAAttributeSet::GetCurrentPostureAttribute())` 获取实时数据。

### 6.2 “危”字提示
- 在 UI 层级创建一个能接收 GameplayEvent 的逻辑。
- 当接收到 `GameplayEvent.UI.DangerNotice` 时，在玩家头顶显示“危”字图标，并配合音效。

---

## 7. 多段攻击 (Combo) 扩展说明

目前 C++ 实现了基础交互，推荐以下两种方式扩展为真正的多段连招：
1. **Montage Section**: 在一个 Montage 中配置 Attack1 -> Attack2 -> Attack3 的 Section，并在每个 Section 末尾通过 Notify 检查缓存输入决定是否跳转。
2. **Combo GA Extension**: 如果需要更复杂的连招分叉，可以继承 `GA_SekiroAttack` 创建 `GA_SekiroComboAttack`，使用一个 `int32 ComboIndex` 来记录并播放不同的 Montage。
