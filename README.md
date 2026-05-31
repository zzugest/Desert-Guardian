# Desert Guardian — 3인칭 액션 RPG

> 아이온2를 레퍼런스로 한 MMORPG 기반의 3인칭 액션 어드벤처 싱글플레이 ARPG

---

## 개요

| 항목 | 내용 |
|------|------|
| 엔진 | Unreal Engine 5.6 |
| 언어 | C++ 70% / Blueprint 30% |
| 개발 기간 | 2026.01 ~ 2026.03 (약 2개월 반) |
| 개발 형태 | 1인 개발 |

---

## 인게임 스크린샷

![gameplay](Screenshots/gameplay.png)

---

## 시연 영상

[![시연 영상](https://img.youtube.com/vi/7G13lTbAaio/0.jpg)](https://youtu.be/7G13lTbAaio)

---

## 주요 구현 기능

### 전투 시스템
- 3단 근접 콤보 공격 및 3단 마법 콤보
- 점프 공격, 구르기 등 액션 어드벤처 기반 전투
- HP / MP / SP 스탯 관리 및 마나 자동 회복
- 피격 시 화면 가장자리 빨간 오버레이 이펙트 (Post Process Material + Timeline)

### 버프 시스템 (이중 구조)
- **스킬 버프**: 스킬 사용 시 적용, `ActiveBuffs[]` 배열 복제, 자동 제거 타이머
- **아이템 버프**: 소비 아이템 사용 시 적용, 스킬 버프와 독립적으로 중첩 가능
- 두 버프 모두 `BaseAttackPower`에 반영, `BuffListWidget` / `BuffIconWidget`으로 UI 표시

### 자동 타겟팅 시스템
- 카메라 전방 벡터 기준 구체 스윕(Sphere Sweep)으로 화면 중심에 가장 가까운 적 자동 선정
- 탐색 거리 3000 유닛, 반경 500 유닛

### 자동이동 시스템 (AutoMoveComponent)
- NavMesh 기반 경로 계산 및 1초 주기 재탐색
- 전방 적 탐지 후 좌/우 경유지 계산으로 자동 우회
- 제자리 감지 타임아웃 (4초) 및 데칼 경로 시각화
- **레벨 간 이동**: `DT_LevelGraph` 테이블을 BFS로 탐색 → 최단 경로 포탈로 자동 이동 → 레벨 진입 후 목적지까지 자동 재개

### 스킬 시스템
- 3개 퀵슬롯(Q, E, R) 기반 스킬
- 투사체 스킬과 버프 스킬
- 각 스킬별 고유 쿨타임과 버프 지속 시간 UI 표시

### 적 AI 시스템
- `AEnemy → NormalMonster + EliteMonster + BossMonster` 계층 구조
- AI Perception(시야 감지) + Behavior Tree 기반 행동
- 리쉬(Leash) 시스템: 일정 거리 이탈 시 스폰 위치로 복귀
- 오브젝트 풀링(`ObjectPoolSubsystem`)으로 스폰/사망 비용 최소화

### 보스 시스템
- 1페이즈 → 2페이즈 전환 (변신 연출 + 공격 패턴 변화 + HP 전체 회복)
- 점프 공격: 타겟 방향으로 발사 → 경고 데칼 표시 → 착지 강타
- 멀티플레이어 타겟 전환 (`BTTask_SwitchTarget`): 범위 내 다른 플레이어로 랜덤 교체
- 보스 등장 컷신 (레벨 시퀀서 기반)

### 미니맵 시스템
- `MinimapComponent`: `SceneCaptureComponent2D`로 지형을 렌더 타겟에 매 프레임 캡처
- `MinimapSubsystem`: Enemy / NPC / Portal 마커를 색상별로 등록·해제
- `MinimapWidget`: 렌더 타겟 배경 위에 마커 점 오버레이 렌더링

### 포탈 / 레벨 이동 시스템
- `DataTable` 기반 포탈 설정 (목적지, 회전, 조건)
- 선행 퀘스트 완료 또는 보스 처치 시 포탈 활성화
- 같은 존 내 텔레포트 / 다른 서브레벨 전환 / 그룹 전체 이동 지원
- 서브레벨 스트리밍 방식으로 이전 레벨 자동 언로드

### NPC 시스템
- `TalkNPC`: DataTable 기반 대화 시퀀스
- `QuestNPC`: 퀘스트 수락 / 보고 / 완료 처리
- `ShopNPC`: 판매 아이템 목록 UI, 서버 권위 구매 처리

### 인벤토리 / 퀵슬롯 / 상점
- 아이템 획득, 장착, 사용 시스템
- 소비 아이템 퀵슬롯 (슬롯 교환, 쿨다운 관리)
- NPC 상점 구매 UI

### 퀘스트 시스템
- 수락 / 진행 / 완료 상태 관리
- 선행 퀘스트 / 보스 처치 등 조건 설정 기능 포함
- 진행도 레벨 이동 후에도 유지 (`QuestSubsystem`)
- 퀘스트 자동이동 연동: 사냥 목표 → NPC 보고 위치 자동 이동

### 네트워크 / 멀티플레이어
- 서버 권위(Server-Authoritative) 모델
- **Server RPC**: 공격, 스킬, 아이템 사용, 포탈 이동 등 모든 주요 입력
- **Multicast RPC**: 애니메이션 동기화, 버프 시작·제거, 데미지 텍스트
- **Client RPC**: 존 스트리밍(로드/언로드), 개인 데미지 텍스트
- 복제 최적화: `IsNetRelevantFor` 오버라이드로 구역 밖 클라이언트에 몬스터 복제 억제

### 데이터 기반 설계
- 적 스탯, 아이템, 스킬, 퀘스트, 대화, 포털, 레벨 그래프 정보를 DataTable로 관리
- 코드 수정 없이 에디터에서 게임 데이터 수정 가능

---

## 아키텍처

### 플레이어 — 컴포넌트 패턴
`MyCharacter`가 기능별 컴포넌트를 조합하는 구조:

```
MyCharacter
├── CombatComponent       — 전투/스탯/버프
├── SkillComponent        — 스킬/퀵슬롯
├── TargetingComponent    — 자동 타겟팅
├── InventoryComponent    — 인벤토리
├── QuickSlotComponent    — 소비 아이템 슬롯
├── MoneyComponent        — 재화
├── AutoMoveComponent     — NavMesh 자동이동 + BFS 레벨 간 이동
├── MinimapComponent      — SceneCaptureComponent2D 미니맵 캡처
├── QuestComponent        — 퀘스트 수락/진행/완료 관리
└── CursorOptionComponent — Ctrl 키 기반 마우스 커서 전환
```

### 전역 상태 — 서브시스템 패턴
`UGameInstanceSubsystem` 상속으로 게임 인스턴스 전체에서 접근 가능한 싱글톤:

```
InventorySubsystem  — 아이템 저장·로직
QuickSlotSubsystem  — 슬롯 데이터 영속
SkillSubsystem      — 스킬 학습·가용성
MoneySubsystem      — 재화 관리
StatSubsystem       — 레벨 이동 후 HP/MP/SP 복원
UISubsystem         — 활성 UI 스택, 마우스/입력 모드 자동 전환
ObjectPoolSubsystem — 몬스터·투사체 풀링
QuestSubsystem      — 퀘스트 상태 영속 (레벨 이동 후 유지)
WarningSubsystem    — 게임 전역 토스트 알림
MinimapSubsystem    — Enemy/NPC/Portal 마커 등록·색상 관리
```

---
