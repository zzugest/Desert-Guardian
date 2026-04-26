# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.6 Action RPG built entirely in C++. Uses Enhanced Input System, Gameplay Tags, Niagara VFX, and UE's subsystem/component architecture patterns throughout.

## Build Commands

**Generate project files** (run from project root):
```
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="ThirdGame.uproject" -game -engine
```

**Build game (Development):**
```
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ThirdGame Win64 Development -Project="ThirdGame.uproject"
```

**Build editor:**
```
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ThirdGameEditor Win64 Development -Project="ThirdGame.uproject"
```

**Clean build:**
```
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ThirdGame Win64 Development -Project="ThirdGame.uproject" -clean
```

Tests are done through Unreal Editor PIE (Play-in-Editor) mode — there is no separate test framework.

## Source Module Structure

All C++ source lives under `Source/ThirdGame/`. The build file (`ThirdGame.Build.cs`) adds include paths for the major subsystem folders: `Character`, `Item`, `Inventory`, `QuickSlot`, `Enemy`.

## Architecture

### Component Model (Player)
`MyCharacter` aggregates behavior via UActorComponents:
- `CombatComponent` — HP/MP/SP stats, 3-hit melee combos, 3-stage magic combos, jump attacks, buff/debuff system, mana regen
- `SkillComponent` — learnable skills, 6 quick slots (Q, E, R + 1-5), cooldown tracking, projectile spawning
- `TargetingComponent` — screen-center trace to find/lock enemies, visual target marker
- `InventoryComponent` — item storage, connected to `InventorySubsystem`
- `QuickSlotComponent` — 5 consumable/ability slots bound to keys 1-5
- `MoneyComponent` — player currency

### Subsystem Pattern (Global State)
UGameInstanceSubsystem subclasses own all global state:
- `InventorySubsystem` — item storage and logic
- `QuickSlotSubsystem` — slot management
- `SkillSubsystem` — skill learning/availability
- `MoneySubsystem` — economy
- `StatSubsystem` — persistent character stats
- `UISubsystem` — central widget manager
- `ObjectPoolSubsystem` — generic pooling for enemies and projectiles
- `QuestSubsystem` — quest state tracking
- `WarningSubsystem` — toast notification system

### Enemy Hierarchy
`AEnemy` (base) → `NormalMonster` → `EliteMonster` → `BossMonster`. Enemies use AI Perception (sight), a leash system returning them to spawn if too far, Behavior Trees for AI, and `MonsterSpawner` for pooled respawning. Enemy configuration is data-driven via `FEnemyData` structs in DataTables.

### Animation Notify System (`/AnimNotify`)
Damage and effects are triggered from animations via AnimNotify/AnimNotifyState subclasses:
- `AnimNotify_AttackHit` — triggers damage at animation frame
- `ANS_WeaponTrace` — enables weapon collision during attack window
- `AnimNotify_FireProjectile` — spawns skill projectiles mid-animation
- `ANS_TrackTarget` / `ANS_LockRotation` — targeting helpers during combos

### Data-Driven Design
Most game data lives in DataTables using these row structs:
- `FEnemyData` — enemy stats, BT reference, drop rewards, mesh config
- `ItemData` — item name, icon, type, effects, price, stackability
- `SkillData` — skill cooldowns, effects, animation refs
- `QuestData` — tasks (Hunt/UseItem/CollectGold), prerequisites, rewards, dialogue per state
- `TalkData` — NPC dialogue sequences
- `PortalData` — level transition configuration

### NPC Systems (`/NPC`)
- `TalkNPC` — simple dialogue with `TalkDialogueWidget`
- `QuestNPC` + `QuestComponent` (on player) — quest lifecycle management
- `ShopNPC` + `ShopComponent` — merchant with purchase UI

### Key Gameplay Flow
1. `MyGameModeBase::BeginPlay` preloads Niagara VFX pool via `ObjectPoolSubsystem`
2. `MyGameInstance` holds reference to `GlobalUIData` asset for shared UI data
3. `MyGameTypes.h` defines `FAttackData` used across combat and damage systems
4. `MyCharacter` binds Enhanced Input actions via `InputMappingContext` and delegates to components
