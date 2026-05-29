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
- `TargetingComponent` — screen-center sphere sweep (radius 500, distance 3000) to find/lock enemies, visual target marker
- `InventoryComponent` — server-authoritative item storage, connected to `InventorySubsystem`
- `QuickSlotComponent` — 5 consumable/ability slots bound to keys 1-5, drag-and-drop registration
- `MoneyComponent` — player currency
- `AutoMoveComponent` — NavMesh-based auto-movement with 1s path recalculation, forward enemy detection and left/right waypoint avoidance, stuck timeout (4s), decal path visualization; uses `DT_AutoMoveTarget` (quest destinations) and `DT_LevelGraph` (BFS inter-level routing)
- `MinimapComponent` — SceneCaptureComponent2D that writes terrain to RT_Minimap render target
- `QuestComponent` — active quest list management, receives events from `QuestSubsystem`
- `CursorOptionComponent` — toggles mouse cursor visibility and UI/game input mode via Ctrl key (Enhanced Input)

### Subsystem Pattern (Global State)
UGameInstanceSubsystem subclasses own all global state:
- `InventorySubsystem` — item storage and logic
- `QuickSlotSubsystem` — slot management
- `SkillSubsystem` — skill learning/availability
- `MoneySubsystem` — economy
- `StatSubsystem` — persistent character stats across level transitions (stores HP/MP/SP, restored on new level load; -1 means not yet saved)
- `UISubsystem` — central widget manager, active UI stack, automatic mouse/input mode switching
- `ObjectPoolSubsystem` — generic pooling for enemies and projectiles
- `QuestSubsystem` — quest state tracking, broadcasts `OnQuestObjectiveUpdated`; progress persists across level transitions
- `WarningSubsystem` — toast notification system
- `MinimapSubsystem` — registers/unregisters actor markers (Enemy/NPC/Portal) with color coding; consumed by `MinimapWidget`

### Enemy Hierarchy
`AEnemy` (base) → `NormalMonster` → `EliteMonster` → `BossMonster`. Enemies use AI Perception (sight), a leash system returning them to spawn if too far, Behavior Trees for AI, and `MonsterSpawner` for pooled respawning. Enemy configuration is data-driven via `FEnemyData` structs in DataTables.

### Animation Notify System (`/AnimNotify`)
Damage and effects are triggered from animations via AnimNotify/AnimNotifyState subclasses:
- `AnimNotify_AttackHit` — triggers damage at animation frame
- `ANS_WeaponTrace` — enables weapon collision during attack window
- `AnimNotify_FireProjectile` — spawns skill projectiles mid-animation
- `ANS_TrackTarget` / `ANS_LockRotation` — targeting helpers during combos

### Buff System (Dual-Layer)
Two independent buff stacks coexist on `CombatComponent`:
- **Skill buffs** — applied by skill use; stored in `ActiveBuffs[]` (replicated); each entry has BuffID/Amount/Duration with an auto-remove timer; removal broadcasts to `BuffListWidget`
- **Item buffs** — applied by consumable use; tracked separately so they stack independently from skill buffs; client-side countdown displayed on `BuffIconWidget`

Both layers feed into `BaseAttackPower` (replicated) for damage calculation and are displayed via `BuffListWidget` / `BuffIconWidget` (icon + countdown).

### Minimap System
Three-layer architecture:
1. `MinimapComponent` (on player) — SceneCaptureComponent2D captures world geometry to `RT_Minimap` render target each frame
2. `MinimapSubsystem` — global registry; actors call `RegisterMarker()` / `UnregisterMarker()` with a type (Enemy/NPC/Portal) and color
3. `MinimapWidget` — renders RT_Minimap as background, overlays marker dots at screen-space positions

### Data-Driven Design
Most game data lives in DataTables using these row structs:
- `FEnemyData` — enemy stats, BT reference, drop rewards, mesh config
- `ItemData` — item name, icon, type, effects, price, stackability
- `SkillData` — skill cooldowns, effects, animation refs
- `QuestData` — tasks (Hunt/UseItem/CollectGold), prerequisites, rewards, dialogue per state
- `TalkData` — NPC dialogue sequences
- `PortalData` — level transition configuration
- `AutoMoveTargetData` — per-quest destination locations and completion NPC tags
- `LevelGraphData` — inter-level adjacency for BFS routing in `AutoMoveComponent`

### NPC Systems (`/NPC`)
- `TalkNPC` — simple dialogue with `TalkDialogueWidget`
- `QuestNPC` + `QuestComponent` (on player) — quest lifecycle management
- `ShopNPC` + `ShopComponent` — merchant with purchase UI

### Network & Replication
The game is built for multiplayer with a server-authoritative model:

**Replicated properties (with OnRep callbacks):**
- `CombatComponent`: `CurrentHP/MP/SP`, `BaseAttackPower`, `ActiveBuffs[]`
- `Enemy`: `CurrentHP`, `bIsDead`, `EnemyDataTable/RowName` (triggers mesh/anim setup on clients)
- `BossMonster`: `bIsRagePhase`, `bIsJumpAttacking`, `CachedImpactLocation`
- `InventoryComponent`: `Content[]` (owner-only)
- `MapPortal`: `bBossKilled`
- `MyCharacter`: `bIsInCombatStance`, `CurrentZoneName`

**RPC patterns:**
- **Server RPCs** (client → server, validated): attack/magic/roll/skill input, item pickup/purchase, quest reward, portal travel, hit reports from client-side traces
- **Multicast RPCs** (server → all clients): montage playback (attack, magic, death, skill), buff start/remove, damage text, portal UI
- **Client RPCs** (server → owning client): zone streaming (load/unload sublevel), personal damage text

**Level streaming:**
- `Server_RequestPortalTravel()` — loads target sublevel, teleports player, unloads previous sublevel
- `Server_RequestGroupPortalTravel()` — moves all players in the same zone simultaneously
- `Client_UpdateZoneStreaming()` — executes local sublevel load/unload on owning client

**Network optimizations:**
- `IsNetRelevantFor` overridden on `AEnemy` and `AMonsterSpawner` to suppress replication for players outside the zone
- Sphere sweep targeting runs only on server; result replicated via target marker
- Damage text spawned locally on each client (no replication)

### Key Gameplay Flow
1. `MyGameModeBase::BeginPlay` preloads Niagara VFX pool via `ObjectPoolSubsystem`
2. `MyGameInstance` holds reference to `GlobalUIData` asset for shared UI data
3. `MyGameTypes.h` defines `FAttackData` used across combat and damage systems
4. `MyCharacter` binds Enhanced Input actions via `InputMappingContext` and delegates to components
5. `StatSubsystem` saves HP/MP/SP before level transition; `MyCharacter::BeginPlay` restores them on the new level
6. `MinimapSubsystem` markers are registered in `BeginPlay` of each enemy/NPC/portal and unregistered on `EndPlay` or death
