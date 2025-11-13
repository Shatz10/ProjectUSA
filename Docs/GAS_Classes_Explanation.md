# GAS 系统类说明文档

本文档详细解释 `Source/ProjectUSA/GAS/` 文件夹下每个类的作用。

---

## 📁 GA (Gameplay Ability) - 游戏能力类

### 基础类

#### `UUSAGameplayAbility`
**作用**: 所有游戏能力的基础类，继承自 `UGameplayAbility`

**核心功能**:
- 提供统一的能力生命周期管理（激活、取消、结束）
- 管理游戏效果（GameplayEffect）的自动应用：
  - `ActivateAbilityEffects`: 激活时应用的效果
  - `CancelAbilityEffects`: 取消时应用的效果
  - `EndAbilityEffects`: 结束时应用的效果
- 提供目标向量计算系统（`CalculateTargetVector`），用于计算移动和攻击方向
- 客户端-服务器同步：`ServerRPC_SetTargetVectorAndDoSomething` 用于同步目标向量
- 提供简单的取消/结束方法：`SimpleCancelAbility()` 和 `SimpleEndAbility()`

---

### 角色动作类

#### `UGA_CharacterAction`
**作用**: 角色动作的核心能力类，用于实现攻击、技能等动作

**核心功能**:
- **方向系统**: 支持多种方向类型（输入方向、目标方向、自定义方向）
- **移动系统**: 支持多种移动类型：
  - `Move`: 基于偏移量的移动
  - `Walk`: 临时改变角色移动参数
  - `Launch`: 发射角色
  - `Custom`: 自定义移动曲线
  - `Magnet`: 自动移动到目标位置
- **结束/中断系统**: 支持基于时间、游戏标签的结束和中断条件
- **动画系统**: 播放动画蒙太奇，支持游戏标签驱动的章节切换（用于连击系统）
- **生成系统**: 在服务器上生成特效或碰撞体
- **攻击追踪**: 执行攻击碰撞检测
- **护甲系统**: 临时增加/恢复护甲属性

**使用场景**: 所有需要移动、动画、攻击的动作（如普通攻击、技能攻击等）

---

#### `UGA_CharacterDash`
**作用**: 角色冲刺/闪避能力

**核心功能**:
- 支持输入按下/释放事件
- 基于曲线（CurveFloat/CurveVector）的冲刺移动
- 可配置的冲刺距离、持续时间、后续速度
- 播放冲刺动画蒙太奇

**使用场景**: 闪避、快速移动

---

#### `UGA_CharacterJump`
**作用**: 角色跳跃能力

**核心功能**:
- 检查是否可以跳跃（`CanActivateAbility`）
- 支持按住跳跃（输入释放时结束）
- 可配置自定义跳跃力度和最大按住时间
- 播放跳跃动画

**使用场景**: 角色跳跃动作

---

#### `UGA_CharacterSlide`
**作用**: 角色滑行能力（如滑铲）

**核心功能**:
- 检测斜坡和天花板，根据环境应用不同的游戏效果
- 支持输入按下/释放来控制滑行
- 检测地面离开事件
- 可配置滑行起始力度、持续时间、角度
- 临时改变角色移动参数（降低摩擦力等）

**使用场景**: 滑铲、滑行动作

---

#### `UGA_CharacterStomp`
**作用**: 角色踩踏/下砸能力

**核心功能**:
- 支持输入按下/释放
- 分阶段应用游戏效果：
  - `PreGameplayEffects`: 准备阶段
  - `ActiveGameplayEffects`: 激活阶段
  - `EndGameplayEffects`: 结束阶段
  - `PostGameplayEffects`: 后置阶段
- 可配置移动速度、前后延迟
- 播放踩踏动画

**使用场景**: 空中下砸攻击、重击地面

---

#### `UGA_CharacterLaunch`
**作用**: 角色发射能力（已注释，可能未使用）

**状态**: 代码已被注释，可能已被其他系统替代

---

### 物品系统类

#### `UGA_CharacterUseItem`
**作用**: 角色使用物品能力

**核心功能**:
- 在指定时机调用角色的使用物品方法
- 临时改变角色移动参数
- 提供蓝图可实现的激活/取消/结束事件

**使用场景**: 使用消耗品、道具等

---

#### `UGA_CharacterChangeItem`
**作用**: 角色切换物品能力

**核心功能**:
- 切换到下一个或上一个物品
- 调用角色的切换物品方法
- 提供蓝图可实现的激活/取消/结束事件

**使用场景**: 切换武器、道具栏切换

---

#### `UGA_CharacterDropWeapon`
**作用**: 角色丢弃武器能力

**使用场景**: 丢弃当前武器

---

### 检查类

#### `UGA_CheckCharacterCeiling`
**作用**: 检查角色是否碰到天花板的能力

**使用场景**: 用于需要检测天花板碰撞的能力

---

#### `UGA_CheckCharacterIsFalling`
**作用**: 检查角色是否在掉落的能力

**使用场景**: 用于需要检测掉落状态的能力

---

### 其他能力类

#### `UGA_AttackTrace`
**作用**: 攻击追踪能力（已注释，可能未使用）

**状态**: 代码已被注释，攻击追踪功能已集成到 `UGA_CharacterAction` 中

---

#### `UGA_ApplyEffectByInput`
**作用**: 通过输入应用游戏效果的能力

**核心功能**:
- 监听输入按下/释放事件
- 根据输入状态应用游戏效果
- 跟踪输入是否被按下一次

**使用场景**: 需要根据输入状态触发效果的能力

---

#### `UGA_PlayAndWaitCharacterAction`
**作用**: 播放并等待角色动作完成的能力

**使用场景**: 用于序列化执行多个动作

---

## 📁 AT (Ability Task) - 能力任务类

### 基础类

#### `UUSAAbilityTask`
**作用**: 所有能力任务的基础类，继承自 `UAbilityTask`

**核心功能**:
- 提供统一的取消/结束方法
- 提供委托系统（`OnAbilityTaskCancel`, `OnAbilityTaskEnd`）
- 可配置是否可取消（`bIsCancelable`）

---

### 动画任务

#### `UAT_PlayAnimMontages`
**作用**: 播放动画蒙太奇任务

**核心功能**:
- 播放指定的动画蒙太奇
- **游戏标签驱动的章节切换**：
  - `AnimMontageSectionMapByGameplayTagAdded`: 标签添加时切换到对应章节（用于连击系统）
  - `AnimMontageSectionMapByGameplayTagRemoved`: 标签移除时切换到对应章节（用于恢复动作）
- 监听蒙太奇事件（完成、混合、中断、取消）
- 支持动态章节切换，实现灵活的连击系统

**使用场景**: 所有需要播放动画的能力

---

### 移动任务

#### `UAT_MoveToLocationByVelocity`
**作用**: 通过速度移动到指定位置的任务

**核心功能**:
- 基于曲线（CurveFloat/CurveVector）的平滑移动
- 移动完成后应用后续速度（`AfterVelocity`）
- 继承自 `UAbilityTask_MoveToLocation`，提供网络同步支持

**使用场景**: 冲刺、技能位移、移动到目标位置

---

#### `UAT_LaunchCharacterForPeriod`
**作用**: 在指定时间内发射角色的任务

**核心功能**:
- 可配置 XY 和 Z 轴的速度覆盖
- 在指定时间内持续应用发射速度
- 支持 Tick 更新，精确控制发射时间

**使用场景**: 击飞、跳跃、技能位移

---

#### `UAT_InputCharacterMoveForPeriod`
**作用**: 在指定时间内根据输入移动角色的任务（已注释）

**状态**: 代码已被注释，可能已被其他系统替代

---

#### `UAT_MaintainCharacterVelocity`
**作用**: 维持角色速度的任务

**使用场景**: 保持角色在特定方向上的速度

---

#### `UAT_MoveToGround`
**作用**: 移动到地面的任务

**使用场景**: 确保角色贴地、落地检测

---

### 环境检测任务

#### `UAT_CheckCharacterIsFalling`
**作用**: 检查角色是否在掉落的任务

**核心功能**:
- 持续检测角色移动组件的掉落状态
- 触发事件：
  - `OnPositiveFalling`: 正向掉落（向下）
  - `OnNegativeFalling`: 负向掉落（向上，如被击飞）
  - `OnGrounded`: 落地
  - `OnFinished`: 任务完成

**使用场景**: 需要根据掉落状态改变行为的能力（如空中技能、落地检测）

---

#### `UAT_CheckCharacterSlope`
**作用**: 检查角色是否在斜坡上的任务

**核心功能**:
- 检测角色前方是否有斜坡（基于角度）
- 触发事件：
  - `OnSlopeTrue`: 检测到斜坡
  - `OnSlopeFalse`: 不在斜坡上
  - `OnGroundOut`: 离开地面

**使用场景**: 滑行能力、斜坡相关动作

---

#### `UAT_CheckCharacterCeiling`
**作用**: 检查角色是否碰到天花板的任务

**核心功能**:
- 使用球体检测角色上方是否有天花板
- 触发事件：
  - `OnCeilingTrue`: 检测到天花板
  - `OnCeilingFalse`: 没有天花板

**使用场景**: 滑行能力、防止角色卡在天花板

---

### 战斗任务

#### `UAT_TraceAttack`
**作用**: 攻击追踪任务（执行攻击碰撞检测）

**核心功能**:
- 基于时间段的多次攻击追踪
- 支持球体追踪（Sphere Trace）
- 每个时间段可以配置：
  - 追踪位置偏移（相对于角色）
  - 追踪半径
  - 伤害值
  - 伤害类型
- 自动应用伤害和生成命中特效
- 服务器端执行（权威性）

**使用场景**: 所有近战攻击、技能攻击的碰撞检测

---

#### `UAT_SpawnActors`
**作用**: 生成 Actor 的任务（用于特效、碰撞体等）

**核心功能**:
- 基于时间段生成多个 Actor
- 每个时间段可以配置：
  - Actor 类
  - 生成位置偏移
  - 旋转
  - 缩放
- 支持目标向量方向计算生成位置
- 服务器端执行（权威性）

**使用场景**: 生成特效、生成碰撞体、生成投射物

---

### 等待任务

#### `UAT_WaitDelay`
**作用**: 延迟等待任务

**使用场景**: 简单的延迟功能

---

#### `UAT_WaitGameplayTag`
**作用**: 等待游戏标签的基础类

**核心功能**:
- 监听游戏标签的变化
- 支持只触发一次或持续监听

---

#### `UAT_WaitGameplayTagAdded`
**作用**: 等待游戏标签被添加的任务

**核心功能**:
- 当指定标签被添加时触发 `Added` 事件
- 如果标签已存在，立即触发

**使用场景**: 等待特定状态、等待连击窗口打开

---

#### `UAT_WaitGameplayTagRemoved`
**作用**: 等待游戏标签被移除的任务

**核心功能**:
- 当指定标签被移除时触发 `Removed` 事件

**使用场景**: 等待状态结束、等待连击窗口关闭

---

### 其他任务

#### `UAT_ChangeCharacterMovementInfo`
**作用**: 改变角色移动参数的任务

**核心功能**:
- 临时改变角色的移动参数（速度、摩擦力、重力等）
- 任务结束时自动恢复原始参数
- 支持取消时恢复

**使用场景**: 滑行、冲刺、特殊移动状态

---

#### `UAT_ApplyGameplayEffect`
**作用**: 应用游戏效果的任务

**使用场景**: 在能力执行过程中应用游戏效果

---

## 📁 AttributeSet - 属性集类

### `UUSAAttributeSet`
**作用**: 角色属性集，管理角色的所有游戏属性

**核心属性**:
- `CurrentHealth`: 当前生命值（可复制）
- `MaxHealth`: 最大生命值（可复制）
- `CurrentArmor`: 当前护甲值（可复制）
- `BaseArmor`: 基础护甲值（可复制）
- `Damage`: 伤害值（用于应用伤害）

**核心功能**:
- **属性变化回调**:
  - `PreAttributeChange`: 属性改变前的处理
  - `PostAttributeChange`: 属性改变后的处理
  - `PreGameplayEffectExecute`: 游戏效果执行前的处理
  - `PostGameplayEffectExecute`: 游戏效果执行后的处理
- **生命值管理**:
  - `OnOutOfHealth`: 生命值耗尽时触发
  - `OnRevive`: 复活时触发
  - `OnCurrentHealthChanged`: 当前生命值改变时触发
  - `OnMaxHealthChanged`: 最大生命值改变时触发
- **网络复制**: 关键属性支持网络复制，确保多人游戏同步

**使用场景**: 所有需要管理角色属性的系统（生命值、护甲、伤害等）

---

## 📊 类关系图

```
UUSAGameplayAbility (基础能力类)
├── UGA_CharacterAction (核心动作能力)
├── UGA_CharacterDash (冲刺)
├── UGA_CharacterJump (跳跃)
├── UGA_CharacterSlide (滑行)
├── UGA_CharacterStomp (踩踏)
├── UGA_CharacterUseItem (使用物品)
├── UGA_CharacterChangeItem (切换物品)
└── ... (其他能力)

UUSAAbilityTask (基础任务类)
├── UAT_PlayAnimMontages (动画播放)
├── UAT_MoveToLocationByVelocity (移动)
├── UAT_LaunchCharacterForPeriod (发射)
├── UAT_TraceAttack (攻击追踪)
├── UAT_SpawnActors (生成Actor)
├── UAT_WaitGameplayTagAdded/Removed (等待标签)
├── UAT_CheckCharacterIsFalling (掉落检测)
├── UAT_CheckCharacterSlope (斜坡检测)
├── UAT_CheckCharacterCeiling (天花板检测)
└── ... (其他任务)

UUSAAttributeSet (属性集)
└── 管理所有角色属性
```

---

## 🎯 典型使用流程

### 攻击能力示例（使用 UGA_CharacterAction）

1. **激活**: `ActivateAbility` 被调用
2. **计算方向**: `CalculateTargetVector` 计算移动和攻击方向
3. **执行动作**: `DoSomethingWithTargetVector` 执行：
   - 旋转角色面向目标
   - 使用 `UAT_MoveToLocationByVelocity` 移动到目标
   - 使用 `UAT_PlayAnimMontages` 播放攻击动画
   - 使用 `UAT_TraceAttack` 执行攻击追踪（服务器端）
   - 使用 `UAT_SpawnActors` 生成特效（服务器端）
   - 使用 `UAT_WaitGameplayTagAdded` 等待结束条件
4. **结束**: 满足结束条件后，`EndAbility` 被调用，恢复护甲等状态

### 连击系统示例

1. **第一次攻击**: 播放 "Attack1" 章节
2. **动画通知**: 在连击窗口打开时添加 `ComboA00` 标签
3. **玩家输入**: 在窗口内按下攻击键
4. **章节切换**: `UAT_PlayAnimMontages` 检测到 `ComboA00` 标签，切换到 "Attack2" 章节
5. **重复**: 继续连击步骤

---

## 📝 总结

这个 GAS 系统设计非常完善，提供了：

1. **模块化设计**: 能力、任务、属性分离，易于扩展
2. **灵活的连击系统**: 基于游戏标签的章节切换，无需硬编码状态机
3. **完善的移动系统**: 支持多种移动类型和曲线控制
4. **环境感知**: 检测掉落、斜坡、天花板等环境状态
5. **网络支持**: 关键功能在服务器端执行，确保多人游戏同步
6. **蓝图友好**: 提供大量蓝图可实现的函数和事件

整个系统非常适合实现复杂的动作游戏战斗系统。


