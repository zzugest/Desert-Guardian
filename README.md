# Desert Guardian — 3인칭 액션 RPG

> 아이온2를 레퍼런스로 한 MMORPG 기반의 3인칭 액션 어드벤처 싱글플레이 ARPG

[![YouTube](https://img.shields.io/badge/YouTube-시연영상-red?logo=youtube)](https://youtu.be/7G13lTbAaio)

---

## 개요

| 항목 | 내용 |
|------|------|
| 엔진 | Unreal Engine 5.6 |
| 언어 | C++ (블루프린트 미사용) |
| 개발 기간 | 2026.01 ~ 2026.03 (약 2개월 반) |
| 개발 형태 | 1인 개발 |

---

## 시연 영상

[![시연 영상](https://img.youtube.com/vi/7G13lTbAaio/0.jpg)](https://youtu.be/7G13lTbAaio)

---

## 주요 구현 기능

### 전투 시스템
- 3단 근접 콤보 공격 및 3단 마법 콤보
- 점프 공격, 구르기 등 액션 어드벤처 기반 전투
- HP / MP / SP 스탯 관리 및 마나 자동 회복
- 버프 / 디버프 시스템

### 자동 타겟팅 시스템
- 카메라 전방 벡터 기준 내적(Dot Product) 연산으로 화면 중심에 가장 가까운 적 자동 선정
- LineTrace로 벽 뒤 적 타겟 제외
- `GetAllActorsOfClass()` 대신 적이 스폰/사망 시 자체 관리하는 정적 목록(`AllActiveEnemies`) 순회로 성능 최적화

### 스킬 시스템
- 6개 퀵슬롯(Q, E, R, 1~3) 기반 스킬
- 쿨다운 추적 및 프로젝타일 발사
- 애니메이션 중간 프레임에 이펙트/데미지 발동 (AnimNotify 시스템)

### 적 AI 시스템
- `AEnemy → NormalMonster → EliteMonster → BossMonster` 계층 구조
- AI Perception(시야 감지) + Behavior Tree 기반 행동
- 리쉬(Leash) 시스템: 일정 거리 이탈 시 스폰 위치로 복귀
- 오브젝트 풀링(`ObjectPoolSubsystem`)으로 스폰/사망 비용 최소화

### 보스 시스템
- HP 50% 도달 시 2페이즈 전환 (변신 연출 + 공격 패턴 변화)
- 점프 공격 시 데칼로 경고 범위 표시
- 보스 감지 시 HP바 UI 표시

### 인벤토리 / 퀵슬롯 / 상점
- 아이템 획득, 장착, 사용 시스템
- 소비 아이템 퀵슬롯 (슬롯 교환, 쿨다운 관리)
- NPC 상점 구매 UI (장바구니 → 일괄 구매)

### 퀘스트 시스템
- 수락 / 진행 / 완료 상태 관리
- Hunt / UseItem / CollectGold 퀘스트 타입
- 선행 퀘스트 / 보스 처치 조건 포함

### 데이터 기반 설계
- 적 스탯, 아이템, 스킬, 퀘스트, 대화, 포털 정보를 DataTable로 관리
- 코드 수정 없이 에디터에서 게임 데이터 수정 가능

---

## 아키텍처

### 플레이어 — 컴포넌트 패턴
`MyCharacter`가 기능별 컴포넌트를 조합하는 구조:

```
MyCharacter
├── CombatComponent     — 전투/스탯
├── SkillComponent      — 스킬/퀵슬롯
├── TargetingComponent  — 자동 타겟팅
├── InventoryComponent  — 인벤토리
├── QuickSlotComponent  — 소비 아이템 슬롯
└── MoneyComponent      — 재화
```

### 전역 상태 — 서브시스템 패턴
`UGameInstanceSubsystem` 상속으로 게임 인스턴스 전체에서 접근 가능한 싱글톤:

```
InventorySubsystem / QuickSlotSubsystem / SkillSubsystem
MoneySubsystem / StatSubsystem / UISubsystem
ObjectPoolSubsystem / QuestSubsystem / WarningSubsystem
```

---

## 기술적으로 고민한 점

### 타겟팅 성능 최적화
매 프레임 호출되는 타겟 스캔에서 `GetAllActorsOfClass()`를 제거하고, 적이 스폰/사망 시 스스로 목록에 등록·해제하는 방식(`AllActiveEnemies`)으로 변경했습니다. 월드 전체 Actor 탐색 비용 없이 현재 살아있는 적만 순회합니다.

### 오브젝트 풀링
몬스터 스폰/사망마다 `SpawnActor/Destroy`를 반복하는 대신 `ObjectPoolSubsystem`으로 재사용해 오버헤드를 줄였습니다.

### AnimNotify 기반 타이밍 제어
데미지 판정, VFX, 프로젝타일 발사를 애니메이션 특정 프레임과 정확히 동기화하기 위해 AnimNotify / AnimNotifyState 서브클래스로 분리했습니다.
