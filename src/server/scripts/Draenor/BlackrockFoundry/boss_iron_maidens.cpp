////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2016 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

# include "boss_iron_maidens.hpp"

/// Admiral Gar'an - 77557
class boss_admiral_garan : public CreatureScript
{
    public:
        boss_admiral_garan() : CreatureScript("boss_admiral_garan") { }

        enum eSpells
        {
            IronMaidenIntroConversation = 172658,
            AfterTrashesConversation    = 172686,
            /// Combat spells
            /// Iron Shot
            SpellIronShotSearcher       = 156666,
            SpellIronShotDamage         = 156669,
            /// Rapid Fire
            SpellRapidFireRedArrow      = 156631,
            SpellRapideFirePeriodic     = 156626,
            SpellRapidFireTargetVisual  = 156649,
            /// Penetrating Shot
            SpellPenetratingShotAura    = 164271,
            /// Deploy Turret
            SpellDeployTurretJump       = 159585,
            SpellDeployTurretSummon     = 158599,
            SpellDominatorBlastDoT      = 158601
        };

        enum eEvents
        {
            EventRapidFire = 1,
            EventRegenIronFury,
            EventPenetratingShot,
            EventDeployTurret
        };

        enum eTimers
        {
            TimerRapidFire          = 19 * TimeConstants::IN_MILLISECONDS,
            TimerRapidFireCD        = 30 * TimeConstants::IN_MILLISECONDS,
            TimerEnergyRegen        = 6 * TimeConstants::IN_MILLISECONDS + 333,
            TimerPenetratingShotCD  = 30 * TimeConstants::IN_MILLISECONDS,
            TimerDeployTurretCD     = 21 * TimeConstants::IN_MILLISECONDS + 300
        };

        enum eTalks
        {
        };

        enum eMoves
        {
            MoveJump = 1,
            MoveDown,
            MoveLast
        };

        enum eVisual
        {
            IntroVisual = 6636
        };

        struct boss_admiral_garanAI : public BossAI
        {
            boss_admiral_garanAI(Creature* p_Creature) : BossAI(p_Creature, eFoundryDatas::DataIronMaidens)
            {
                m_Instance  = p_Creature->GetInstanceScript();
                m_IntroDone = false;

                p_Creature->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_DISABLE_COLLISION);
            }

            InstanceScript* m_Instance;

            EventMap m_Events;

            bool m_IntroDone;
            std::set<uint64> m_TrashesGuids;

            bool m_DeployTurret;

            void Reset() override
            {
                me->setPowerType(Powers::POWER_ENERGY);
                me->SetMaxPower(Powers::POWER_ENERGY, 100);
                me->SetPower(Powers::POWER_ENERGY, 0);

                me->CastSpell(me, eIronMaidensSpells::ZeroPowerZeroRegen, true);

                m_Events.Reset();

                RespawnMaidens(m_Instance, me);

                _Reset();

                if (!m_IntroDone)
                {
                    AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                    {
                        std::list<Creature*> l_TrashesList;

                        me->GetCreatureListWithEntryInGrid(l_TrashesList, eIronMaidensCreatures::AquaticTechnician, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_TrashesList, eIronMaidensCreatures::IronDockworker, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_TrashesList, eIronMaidensCreatures::IronEarthbinder, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_TrashesList, eIronMaidensCreatures::IronMauler, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_TrashesList, eIronMaidensCreatures::IronCleaver, 150.0f);

                        for (Creature* l_Trash : l_TrashesList)
                        {
                            if (l_Trash->isAlive())
                                m_TrashesGuids.insert(l_Trash->GetGUID());
                        }

                        if (m_TrashesGuids.empty())
                        {
                            m_IntroDone = true;

                            AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                            {
                                if (Creature* l_Sorka = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossEnforcerSorka)))
                                {
                                    if (l_Sorka->IsAIEnabled)
                                        l_Sorka->AI()->DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                                }

                                if (Creature* l_Marak = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossMarakTheBlooded)))
                                {
                                    if (l_Marak->IsAIEnabled)
                                        l_Marak->AI()->DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                                }

                                DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                            });
                        }
                    });
                }

                m_DeployTurret = false;
            }

            void SetGUID(uint64 p_Guid, int32 p_ID /*= 0*/) override
            {
                m_TrashesGuids.erase(p_Guid);

                if (!m_IntroDone && m_TrashesGuids.empty() && m_Instance != nullptr)
                {
                    m_IntroDone = true;

                    AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                    {
                        if (Creature* l_Sorka = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossEnforcerSorka)))
                        {
                            if (l_Sorka->IsAIEnabled)
                                l_Sorka->AI()->DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                        }

                        if (Creature* l_Marak = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossMarakTheBlooded)))
                        {
                            if (l_Marak->IsAIEnabled)
                                l_Marak->AI()->DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                        }

                        DoAction(eIronMaidensActions::ActionAfterTrashesIntro);
                    });
                }
            }

            void KilledUnit(Unit* p_Killed) override
            {
                /*if (p_Killed->IsPlayer())
                    Talk(eThogarTalks::TalkSlay);*/
            }

            void EnterCombat(Unit* p_Attacker) override
            {
                StartMaidens(m_Instance, me, p_Attacker);

                _EnterCombat();

                ///m_Events.ScheduleEvent(eEvents::EventRapidFire, eTimers::TimerRapidFire);
                ///m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
            }

            void JustDied(Unit* /*p_Killer*/) override
            {
                me->RemoveAllAreasTrigger();

                summons.DespawnAll();

                _JustDied();

                if (m_Instance != nullptr)
                    m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);

                RemoveCombatAuras();
            }

            void EnterEvadeMode() override
            {
                WipeMaidens(m_Instance);

                CreatureAI::EnterEvadeMode();

                me->RemoveAllAreasTrigger();

                summons.DespawnAll();

                RemoveCombatAuras();
            }

            void DoAction(int32 const p_Action) override
            {
                switch (p_Action)
                {
                    case eIronMaidensActions::ActionIntro:
                    {
                        if (m_IntroDone)
                            break;

                        me->CastSpell(me, eSpells::IronMaidenIntroConversation, false);
                        break;
                    }
                    case eIronMaidensActions::ActionAfterTrashesIntro:
                    {
                        me->CastSpell(me, eSpells::AfterTrashesConversation, false);

                        std::list<Creature*> l_CosmeticMobs;
                        me->GetCreatureListWithEntryInGrid(l_CosmeticMobs, eIronMaidensCreatures::Ukurogg, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_CosmeticMobs, eIronMaidensCreatures::Uktar, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_CosmeticMobs, eIronMaidensCreatures::BattleMedicRogg, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_CosmeticMobs, eIronMaidensCreatures::Gorak, 150.0f);
                        me->GetCreatureListWithEntryInGridAppend(l_CosmeticMobs, eIronMaidensCreatures::IronEviscerator, 150.0f);

                        for (Creature* l_Mob : l_CosmeticMobs)
                            l_Mob->DespawnOrUnsummon();

                        AddTimedDelayedOperation(9 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->GetMotionMaster()->MoveJump(g_BoatBossFirstJumpPos, 30.0f, 20.0f, eMoves::MoveJump);
                        });

                        break;
                    }
                    default:
                        break;
                }
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                if (p_Type != MovementGeneratorType::EFFECT_MOTION_TYPE &&
                    p_Type != MovementGeneratorType::POINT_MOTION_TYPE)
                    return;

                switch (p_ID)
                {
                    case eMoves::MoveJump:
                    {
                        me->SetAIAnimKitId(eVisual::IntroVisual);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_FLYING);
                            me->SetSpeed(UnitMoveType::MOVE_FLIGHT, 4.0f);
                            me->GetMotionMaster()->MoveSmoothFlyPath(eMoves::MoveDown, g_BoatBossFlyingMoves);
                        });

                        break;
                    }
                    case eMoves::MoveDown:
                    {
                        me->SetAIAnimKitId(0);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->RemoveUnitMovementFlag(MovementFlags::MOVEMENTFLAG_FLYING);
                            me->SetSpeed(UnitMoveType::MOVE_FLIGHT, me->GetCreatureTemplate()->speed_fly);
                            me->GetMotionMaster()->MoveJump(g_GaranHomePos, 30.0f, 20.0f, eMoves::MoveLast);
                        });

                        break;
                    }
                    case eMoves::MoveLast:
                    {
                        for (uint8 l_I = 0; l_I < MAX_EQUIPMENT_ITEMS * 2; ++l_I)
                            me->SetUInt32Value(EUnitFields::UNIT_FIELD_VIRTUAL_ITEMS + l_I, 0);

                        me->SetHomePosition(g_GaranHomePos);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->SetFacingTo(g_GaranFinalFacing);
                        });

                        AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->LoadEquipment();

                            me->RemoveFlag(EUnitFields::UNIT_FIELD_FLAGS, eUnitFlags::UNIT_FLAG_IMMUNE_TO_PC);

                            if (Creature* l_Sorka = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossEnforcerSorka)))
                                l_Sorka->RemoveFlag(EUnitFields::UNIT_FIELD_FLAGS, eUnitFlags::UNIT_FLAG_IMMUNE_TO_PC);

                            if (Creature* l_Marak = Creature::GetCreature(*me, m_Instance->GetData64(eFoundryCreatures::BossMarakTheBlooded)))
                                l_Marak->RemoveFlag(EUnitFields::UNIT_FIELD_FLAGS, eUnitFlags::UNIT_FLAG_IMMUNE_TO_PC);
                        });

                        break;
                    }
                    case eSpells::SpellDeployTurretJump:
                    {
                        me->CastSpell(*me, eSpells::SpellDeployTurretSummon, false);
                        break;
                    }
                    default:
                        break;
                }
            }

            void RegeneratePower(Powers /*p_Power*/, int32& p_Value) override
            {
                /// Iron Maidens only regens by script
                p_Value = 0;
            }

            void SetPower(Powers p_Power, int32 p_Value) override
            {
                switch (p_Value)
                {
                    case eIronMaidensDatas::FirstIronFuryAbility:
                    {
                        if (m_Events.HasEvent(eEvents::EventPenetratingShot))
                            break;

                        m_Events.ScheduleEvent(eEvents::EventPenetratingShot, 1);
                        break;
                    }
                    case eIronMaidensDatas::SecondIronFuryAbility:
                    {
                        if (m_Events.HasEvent(eEvents::EventDeployTurret))
                            break;

                        m_Events.ScheduleEvent(eEvents::EventDeployTurret, 1);
                        break;
                    }
                    default:
                        break;
                }
            }

            void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
            {
                if (p_Target == nullptr)
                    return;

                switch (p_SpellInfo->Id)
                {
                    case eSpells::SpellIronShotSearcher:
                    {
                        me->CastSpell(p_Target, eSpells::SpellIronShotDamage, false);
                        break;
                    }
                    default:
                        break;
                }
            }

            void OnSpellCasted(SpellInfo const* p_SpellInfo) override
            {
                switch (p_SpellInfo->Id)
                {
                    case eSpells::SpellDeployTurretSummon:
                    {
                        if (Player* l_Victim = SelectMainTank())
                            me->GetMotionMaster()->MoveChase(l_Victim);

                        m_DeployTurret = false;
                        break;
                    }
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                UpdateOperations(p_Diff);

                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) >= 80.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                m_Events.Update(p_Diff);

                if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
                    return;

                switch (m_Events.ExecuteEvent())
                {
                    case eEvents::EventRapidFire:
                    {
                        if (Creature* l_Stalker = me->SummonCreature(eIronMaidensCreatures::RapidFireStalker, *me))
                        {
                            l_Stalker->DespawnOrUnsummon(11 * TimeConstants::IN_MILLISECONDS);

                            me->CastSpell(l_Stalker, eSpells::SpellRapideFirePeriodic, false);

                            if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM))
                            {
                                me->CastSpell(l_Target, eSpells::SpellRapidFireRedArrow, true);

                                uint64 l_TargetGUID = l_Target->GetGUID();
                                uint64 l_Guid       = l_Stalker->GetGUID();
                                AddTimedDelayedOperation(3 * TimeConstants::IN_MILLISECONDS, [this, l_Guid, l_TargetGUID]() -> void
                                {
                                    if (Creature* l_Stalker = Creature::GetCreature(*me, l_Guid))
                                    {
                                        if (l_Stalker->IsAIEnabled)
                                            l_Stalker->AI()->SetGUID(l_TargetGUID);

                                        l_Stalker->CastSpell(l_Stalker, eSpells::SpellRapidFireTargetVisual, true);
                                    }
                                });
                            }
                        }

                        m_Events.ScheduleEvent(eEvents::EventRapidFire, eTimers::TimerRapidFireCD);
                        break;
                    }
                    case eEvents::EventRegenIronFury:
                    {
                        me->ModifyPower(Powers::POWER_ENERGY, 1);
                        m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
                        break;
                    }
                    case eEvents::EventPenetratingShot:
                    {
                        if (Player* l_Target = SelectRangedTarget())
                            me->CastSpell(l_Target, eSpells::SpellPenetratingShotAura, false);

                        m_Events.ScheduleEvent(eEvents::EventPenetratingShot, eTimers::TimerPenetratingShotCD);
                        break;
                    }
                    case eEvents::EventDeployTurret:
                    {
                        m_DeployTurret = true;

                        float l_Range = frand(10.0f, 25.0f);
                        float l_Angle = frand(0.0f, 2 * M_PI);

                        Position l_JumpPos;

                        l_JumpPos.m_positionX = g_RoomCenterPos.m_positionX + l_Range * cos(l_Angle);
                        l_JumpPos.m_positionY = g_RoomCenterPos.m_positionY + l_Range * sin(l_Angle);
                        l_JumpPos.m_positionZ = g_RoomCenterPos.m_positionZ;

                        me->CastSpell(l_JumpPos, eSpells::SpellDeployTurretJump, true);

                        m_Events.ScheduleEvent(eEvents::EventDeployTurret, eTimers::TimerDeployTurretCD);
                        break;
                    }
                    default:
                        break;
                }

                /// Must be checked again, before using Iron Shot
                if (me->HasUnitState(UnitState::UNIT_STATE_CASTING) || m_DeployTurret)
                    return;

                me->CastSpell(me, eSpells::SpellIronShotSearcher, true);
            }

            void RemoveCombatAuras()
            {
                if (m_Instance == nullptr)
                    return;

                m_Instance->DoRemoveAurasDueToSpellOnPlayers(eSpells::SpellRapidFireRedArrow);
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(eSpells::SpellDominatorBlastDoT);
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new boss_admiral_garanAI(p_Creature);
        }
};

/// Enforcer Sorka - 77231
class boss_enforcer_sorka : public CreatureScript
{
    public:
        boss_enforcer_sorka() : CreatureScript("boss_enforcer_sorka") { }

        enum eSpells
        {
            /// Combat Spells
            /// Blade Dash
            SpellBladeDashCast          = 155794,
            SpellBladeDashDamage        = 155841,
            /// Convulsive Shadows
            SpellConvulsiveShadowsCast  = 156109,
            SpellConvulsiveShadowsAura  = 156214,
            /// Dark Hunt
            SpellDarkHuntAura           = 158315
        };

        enum eEvents
        {
            EventBladeDash = 1,
            EventRegenIronFury,
            EventConvulsiveShadows,
            EventDarkHunt
        };

        enum eTimers
        {
            TimerBladeDash              = 11 * TimeConstants::IN_MILLISECONDS,
            TimerBladeDashCD            = 20 * TimeConstants::IN_MILLISECONDS,
            TimerEnergyRegen            = /*6 * TimeConstants::IN_MILLISECONDS + 333*/1000,
            TimerConvulsiveShadowsCD    = 56 * TimeConstants::IN_MILLISECONDS,
            TimerDarkHuntCD             = 13 * TimeConstants::IN_MILLISECONDS + 500
        };

        enum eTalks
        {
        };

        enum eMove
        {
            MoveJump = 1
        };

        struct boss_enforcer_sorkaAI : public BossAI
        {
            boss_enforcer_sorkaAI(Creature* p_Creature) : BossAI(p_Creature, eFoundryDatas::DataIronMaidens)
            {
                m_Instance = p_Creature->GetInstanceScript();

                p_Creature->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_DISABLE_COLLISION);
            }

            InstanceScript* m_Instance;

            EventMap m_Events;

            bool m_IsInBladeDash;
            std::set<uint64> m_BladeDashTargets;

            void Reset() override
            {
                me->setPowerType(Powers::POWER_ENERGY);
                me->SetMaxPower(Powers::POWER_ENERGY, 100);
                me->SetPower(Powers::POWER_ENERGY, /*0*/25);

                me->CastSpell(me, eIronMaidensSpells::ZeroPowerZeroRegen, true);

                m_Events.Reset();

                RespawnMaidens(m_Instance, me);

                _Reset();

                m_IsInBladeDash = false;
                m_BladeDashTargets.clear();
            }

            void KilledUnit(Unit* p_Killed) override
            {
                /*if (p_Killed->IsPlayer())
                    Talk(eThogarTalks::TalkSlay);*/
            }

            void EnterCombat(Unit* p_Attacker) override
            {
                StartMaidens(m_Instance, me, p_Attacker);

                _EnterCombat();

                ///m_Events.ScheduleEvent(eEvents::EventBladeDash, eTimers::TimerBladeDash);
                m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
            }

            void JustDied(Unit* /*p_Killer*/) override
            {
                me->RemoveAllAreasTrigger();

                summons.DespawnAll();

                _JustDied();

                if (m_Instance != nullptr)
                    m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);

                RemoveCombatAuras();
            }

            void EnterEvadeMode() override
            {
                WipeMaidens(m_Instance);

                CreatureAI::EnterEvadeMode();

                me->RemoveAllAreasTrigger();

                summons.DespawnAll();

                RemoveCombatAuras();
            }

            void DoAction(int32 const p_Action) override
            {
                switch (p_Action)
                {
                    case eIronMaidensActions::ActionAfterTrashesIntro:
                    {
                        AddTimedDelayedOperation(13 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->GetMotionMaster()->MoveJump(g_SorkaHomePos, 30.0f, 20.0f, eMove::MoveJump);
                        });

                        break;
                    }
                    default:
                        break;
                }
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                if (p_Type != MovementGeneratorType::EFFECT_MOTION_TYPE)
                    return;

                if (p_ID == eMove::MoveJump)
                    me->SetHomePosition(g_SorkaHomePos);
            }

            void RegeneratePower(Powers /*p_Power*/, int32& p_Value) override
            {
                /// Iron Maidens only regens by script
                p_Value = 0;
            }

            void SetPower(Powers p_Power, int32 p_Value) override
            {
                switch (p_Value)
                {
                    case eIronMaidensDatas::FirstIronFuryAbility:
                    {
                        if (m_Events.HasEvent(eEvents::EventConvulsiveShadows))
                            break;

                        m_Events.ScheduleEvent(eEvents::EventConvulsiveShadows, 1);
                        break;
                    }
                    case eIronMaidensDatas::SecondIronFuryAbility:
                    {
                        if (m_Events.HasEvent(eEvents::EventDarkHunt))
                            break;

                        m_Events.ScheduleEvent(eEvents::EventDarkHunt, 1);
                        break;
                    }
                    default:
                        break;
                }
            }

            void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
            {
                if (p_Target == nullptr)
                    return;

                switch (p_SpellInfo->Id)
                {
                    case eSpells::SpellBladeDashCast:
                    {
                        if (!p_Target->IsPlayer())
                            break;

                        DashToTarget(p_Target);
                        break;
                    }
                    case eSpells::SpellBladeDashDamage:
                    {
                        if (!m_IsInBladeDash)
                        {
                            if (Player* l_Target = SelectMainTank())
                                me->GetMotionMaster()->MoveChase(l_Target);

                            me->RemoveAura(eSpells::SpellBladeDashCast);
                            break;
                        }

                        /// Cannot select a player twice
                        m_BladeDashTargets.insert(p_Target->GetGUID());

                        std::list<Player*> l_PossibleTargets;
                        me->GetPlayerListInGrid(l_PossibleTargets, 8.0f);

                        if (!l_PossibleTargets.empty())
                        {
                            l_PossibleTargets.remove_if([this](Player* p_Player) -> bool
                            {
                                if (p_Player == nullptr || m_BladeDashTargets.find(p_Player->GetGUID()) != m_BladeDashTargets.end())
                                    return true;

                                return false;
                            });
                        }

                        if (l_PossibleTargets.empty())
                        {
                            m_IsInBladeDash = false;

                            if (Player* l_Target = SelectMainTank())
                                DashToTarget(l_Target);

                            break;
                        }

                        auto l_Iter = l_PossibleTargets.begin();
                        std::advance(l_Iter, urand(0, l_PossibleTargets.size() - 1));

                        DashToTarget(*l_Iter);
                        break;
                    }
                    case eSpells::SpellConvulsiveShadowsCast:
                    {
                        p_Target->CastSpell(p_Target, eSpells::SpellConvulsiveShadowsAura, true, nullptr, nullptr, me->GetGUID());

                        if (Aura* l_Aura = p_Target->GetAura(eSpells::SpellConvulsiveShadowsAura))
                            l_Aura->SetStackAmount(4);

                        break;
                    }
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                UpdateOperations(p_Diff);

                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) >= 80.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                m_Events.Update(p_Diff);

                if (me->HasUnitState(UnitState::UNIT_STATE_CASTING) || m_IsInBladeDash)
                    return;

                switch (m_Events.ExecuteEvent())
                {
                    case eEvents::EventBladeDash:
                    {
                        m_IsInBladeDash = true;
                        m_BladeDashTargets.clear();

                        if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 45.0f, true))
                        {
                            me->SetFacingTo(me->GetAngle(l_Target));

                            uint64 l_Guid = l_Target->GetGUID();
                            AddTimedDelayedOperation(10, [this, l_Guid]() -> void
                            {
                                if (Unit* l_Target = Unit::GetUnit(*me, l_Guid))
                                    me->CastSpell(l_Target, eSpells::SpellBladeDashCast, false);
                            });
                        }

                        m_Events.ScheduleEvent(eEvents::EventBladeDash, eTimers::TimerBladeDashCD);
                        break;
                    }
                    case eEvents::EventRegenIronFury:
                    {
                        me->ModifyPower(Powers::POWER_ENERGY, 1);
                        m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
                        break;
                    }
                    case eEvents::EventConvulsiveShadows:
                    {
                        if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            me->CastSpell(l_Target, eSpells::SpellConvulsiveShadowsCast, false);

                        m_Events.ScheduleEvent(eEvents::EventConvulsiveShadows, eTimers::TimerConvulsiveShadowsCD);
                        break;
                    }
                    case eEvents::EventDarkHunt:
                    {
                        if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            me->CastSpell(l_Target, eSpells::SpellDarkHuntAura, true);

                        m_Events.ScheduleEvent(eEvents::EventDarkHunt, eTimers::TimerDarkHuntCD);
                        break;
                    }
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }

            void DashToTarget(Unit* p_Target)
            {
                me->NearTeleportTo(*p_Target);

                uint64 l_Guid = p_Target->GetGUID();
                AddTimedDelayedOperation(10, [this, l_Guid]() -> void
                {
                    if (Unit* l_Target = Unit::GetUnit(*me, l_Guid))
                        me->CastSpell(l_Target, eSpells::SpellBladeDashDamage, true);
                });
            }

            void RemoveCombatAuras()
            {
                if (m_Instance == nullptr)
                    return;

                m_Instance->DoRemoveAurasDueToSpellOnPlayers(eSpells::SpellConvulsiveShadowsAura);
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(eSpells::SpellDarkHuntAura);
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new boss_enforcer_sorkaAI(p_Creature);
        }
};

/// Marak the Blooded - 77477
class boss_marak_the_blooded : public CreatureScript
{
    public:
        boss_marak_the_blooded() : CreatureScript("boss_marak_the_blooded") { }

        enum eSpells
        {
            /// Combat Spells
            /// Blood Ritual
            SpellBloodRitualAura    = 159724,
            SpellBloodRitualCast    = 158078
        };

        enum eEvents
        {
            EventBloodRitual = 1,
            EventRegenIronFury,
            EventBloodsoakedHeartseeker,
            EventSanguineStrikes
        };

        enum eTimers
        {
            TimerBloodRitual    = 5 * TimeConstants::IN_MILLISECONDS,
            TimerBloodRitualCD  = 20 * TimeConstants::IN_MILLISECONDS,
            TimerEnergyRegen    = 6 * TimeConstants::IN_MILLISECONDS + 333
        };

        enum eTalks
        {
        };

        enum eVisual
        {
            IntroVisual = 6636
        };

        enum eMoves
        {
            MoveJump = 1,
            MoveDown,
            MoveLast
        };

        struct boss_marak_the_bloodedAI : public BossAI
        {
            boss_marak_the_bloodedAI(Creature* p_Creature) : BossAI(p_Creature, eFoundryDatas::DataIronMaidens)
            {
                m_Instance = p_Creature->GetInstanceScript();

                p_Creature->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_DISABLE_COLLISION);
            }

            InstanceScript* m_Instance;

            EventMap m_Events;

            void Reset() override
            {
                me->setPowerType(Powers::POWER_ENERGY);
                me->SetMaxPower(Powers::POWER_ENERGY, 100);
                me->SetPower(Powers::POWER_ENERGY, 0);

                me->CastSpell(me, eIronMaidensSpells::ZeroPowerZeroRegen, true);

                m_Events.Reset();

                RespawnMaidens(m_Instance, me);

                _Reset();
            }

            void KilledUnit(Unit* p_Killed) override
            {
                /*if (p_Killed->IsPlayer())
                    Talk(eThogarTalks::TalkSlay);*/
            }

            void EnterCombat(Unit* p_Attacker) override
            {
                StartMaidens(m_Instance, me, p_Attacker);

                _EnterCombat();

                ///m_Events.ScheduleEvent(eEvents::EventBloodRitual, eTimers::TimerBloodRitual);
                ///m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
            }

            void JustDied(Unit* /*p_Killer*/) override
            {
                me->RemoveAllAreasTrigger();

                summons.DespawnAll();

                _JustDied();

                if (m_Instance != nullptr)
                {
                    m_Instance->SendEncounterUnit(EncounterFrameType::ENCOUNTER_FRAME_DISENGAGE, me);
                }
            }

            void EnterEvadeMode() override
            {
                WipeMaidens(m_Instance);

                CreatureAI::EnterEvadeMode();

                me->RemoveAllAreasTrigger();

                summons.DespawnAll();
            }

            void DoAction(int32 const p_Action) override
            {
                switch (p_Action)
                {
                    case eIronMaidensActions::ActionAfterTrashesIntro:
                    {
                        me->GetMotionMaster()->MoveJump(g_BoatBossFirstJumpPos, 30.0f, 20.0f, eMoves::MoveJump);
                        break;
                    }
                    default:
                        break;
                }
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                if (p_Type != MovementGeneratorType::EFFECT_MOTION_TYPE &&
                    p_Type != MovementGeneratorType::POINT_MOTION_TYPE)
                    return;

                switch (p_ID)
                {
                    case eMoves::MoveJump:
                    {
                        me->SetAIAnimKitId(eVisual::IntroVisual);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_FLYING);
                            me->SetSpeed(UnitMoveType::MOVE_FLIGHT, 4.0f);
                            me->GetMotionMaster()->MoveSmoothFlyPath(eMoves::MoveDown, g_BoatBossFlyingMoves);
                        });

                        break;
                    }
                    case eMoves::MoveDown:
                    {
                        me->SetAIAnimKitId(0);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->RemoveUnitMovementFlag(MovementFlags::MOVEMENTFLAG_FLYING);
                            me->SetSpeed(UnitMoveType::MOVE_FLIGHT, me->GetCreatureTemplate()->speed_fly);
                            me->GetMotionMaster()->MoveJump(g_MarakHomePos, 30.0f, 20.0f, eMoves::MoveLast);
                        });

                        break;
                    }
                    case eMoves::MoveLast:
                    {
                        me->SetHomePosition(g_MarakHomePos);

                        AddTimedDelayedOperation(10, [this]() -> void
                        {
                            me->SetFacingTo(g_MarakFinalFacing);
                        });

                        break;
                    }
                    default:
                        break;
                }
            }

            void RegeneratePower(Powers /*p_Power*/, int32& p_Value) override
            {
                /// Iron Maidens only regens by script
                p_Value = 0;
            }

            void SetPower(Powers p_Power, int32 p_Value) override
            {
                switch (p_Value)
                {
                    case eIronMaidensDatas::FirstIronFuryAbility:
                    {
                        m_Events.ScheduleEvent(eEvents::EventBloodsoakedHeartseeker, 1);
                        break;
                    }
                    case eIronMaidensDatas::SecondIronFuryAbility:
                    {
                        m_Events.ScheduleEvent(eEvents::EventSanguineStrikes, 1);
                        break;
                    }
                    default:
                        break;
                }
            }

            void OnSpellCasted(SpellInfo const* p_SpellInfo) override
            {
                switch (p_SpellInfo->Id)
                {
                    case eSpells::SpellBloodRitualCast:
                    {
                        std::list<Player*> l_PlayerList;
                        me->GetPlayerListInGrid(l_PlayerList, 45.0f);

                        if (!l_PlayerList.empty())
                        {
                            l_PlayerList.remove_if([this](Player* p_Player) -> bool
                            {
                                if (p_Player == nullptr || !p_Player->isInFront(me))
                                    return true;

                                return false;
                            });
                        }

                        break;
                    }
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                UpdateOperations(p_Diff);

                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) >= 80.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                m_Events.Update(p_Diff);

                if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
                    return;

                switch (m_Events.ExecuteEvent())
                {
                    case eEvents::EventBloodRitual:
                    {
                        if (Unit* l_Target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 45.0f, true))
                        {
                            me->CastSpell(l_Target, eSpells::SpellBloodRitualAura, true);
                            me->CastSpell(l_Target, eSpells::SpellBloodRitualCast, false);
                        }

                        m_Events.ScheduleEvent(eEvents::EventBloodRitual, eTimers::TimerBloodRitualCD);
                        break;
                    }
                    case eEvents::EventRegenIronFury:
                    {
                        me->ModifyPower(Powers::POWER_ENERGY, 1);
                        m_Events.ScheduleEvent(eEvents::EventRegenIronFury, eTimers::TimerEnergyRegen);
                        break;
                    }
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new boss_marak_the_bloodedAI(p_Creature);
        }
};

/// Loading Chain - 78767
class npc_foundry_loading_chain : public CreatureScript
{
    public:
        npc_foundry_loading_chain() : CreatureScript("npc_foundry_loading_chain") { }

        enum eSpells
        {
            RideLoadingChain    = 158646,
            LoadingChainVisual  = 159086,
            LoadCrate           = 171209
        };

        enum eMoves
        {
            MoveBoss = 1,
            MoveBoat
        };

        struct npc_foundry_loading_chainAI : public ScriptedAI
        {
            npc_foundry_loading_chainAI(Creature* p_Creature) : ScriptedAI(p_Creature), m_ChainID(0) { }

            uint8 m_ChainID;
            bool m_IsAvailable;

            void Reset() override
            {
                m_IsAvailable = true;

                me->CastSpell(me, eSpells::LoadingChainVisual, true);

                me->SetReactState(ReactStates::REACT_PASSIVE);

                me->SetUInt32Value(EUnitFields::UNIT_FIELD_INTERACT_SPELL_ID, eSpells::RideLoadingChain);

                /// Init chain ID
                for (Position const l_Pos : g_LoadingChainsSpawnPos)
                {
                    if (me->IsNearPosition(&l_Pos, 0.5f))
                        break;

                    ++m_ChainID;
                }
            }

            void SpellHit(Unit* p_Attacker, SpellInfo const* p_SpellInfo) override
            {
                if (p_SpellInfo->Id == eSpells::LoadCrate && m_ChainID < eIronMaidensDatas::MaxLoadingChains)
                    me->GetMotionMaster()->MovePoint(eMoves::MoveBoat, g_LoadingChainsMoveBoatPos[m_ChainID]);
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                if (p_Type != MovementGeneratorType::POINT_MOTION_TYPE)
                    return;

                switch (p_ID)
                {
                    case eMoves::MoveBoss:
                    {
                        m_IsAvailable = true;
                        break;
                    }
                    case eMoves::MoveBoat:
                    {
                        AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->RemoveAura(eSpells::LoadCrate);

                            me->GetMotionMaster()->MovePoint(eMoves::MoveBoss, g_LoadingChainsSpawnPos[m_ChainID]);
                        });

                        break;
                    }
                    default:
                        break;
                }
            }

            void SetData(uint32 p_ID, uint32 p_Value) override
            {
                switch (p_ID)
                {
                    case eIronMaidensDatas::LoadingChainAvailable:
                        m_IsAvailable = p_Value != 0;
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 p_ID = 0) override
            {
                switch (p_ID)
                {
                    case eIronMaidensDatas::LoadingChainID:
                        return uint32(m_ChainID);
                    case eIronMaidensDatas::LoadingChainAvailable:
                        return uint32(m_IsAvailable);
                    default:
                        return 0;
                }
            }

            void UpdateAI(uint32 const p_Diff)
            {
                UpdateOperations(p_Diff);
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new npc_foundry_loading_chainAI(p_Creature);
        }
};

/// Uk'urogg <Deckhand of Marak the Blooded> - 78341
class npc_foundry_ukurogg : public CreatureScript
{
    public:
        npc_foundry_ukurogg() : CreatureScript("npc_foundry_ukurogg") { }

        enum eSpells
        {
            CarryingCrate = 171198
        };

        enum eEvents
        {
        };

        enum eMoves
        {
            MoveCarryCrate = 1,
            MoveThrowCrate
        };

        struct npc_foundry_ukuroggAI : public ScriptedAI
        {
            npc_foundry_ukuroggAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

            Position const m_UkuroggThrowCratePos = { 495.576f, 3273.3f, 141.388f, 0.0f };
            Position const m_UkuroggCarryCratePos = { 478.2083f, 3280.669f, 141.388f, 0.0f };

            float const m_UkuroggFinalFacing = 2.859696f;

            EventMap m_Events;

            void Reset() override
            {
                m_Events.Reset();
            }

            void EnterCombat(Unit* /*p_Attacker*/) override
            {
                me->SetWalk(false);

                ClearDelayedOperations();
            }

            void SpellHit(Unit* p_Attacker, SpellInfo const* p_SpellInfo)
            {
                if (p_SpellInfo->Id == eSpells::CarryingCrate)
                {
                    AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                    {
                        me->SetWalk(true);

                        me->GetMotionMaster()->MovePoint(eMoves::MoveThrowCrate, m_UkuroggThrowCratePos);
                    });
                }
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                if (p_Type != MovementGeneratorType::POINT_MOTION_TYPE)
                    return;

                switch (p_ID)
                {
                    case eMoves::MoveCarryCrate:
                    {
                        AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->SetFacingTo(m_UkuroggFinalFacing);
                        });

                        break;
                    }
                    case eMoves::MoveThrowCrate:
                    {
                        AddTimedDelayedOperation(1 * TimeConstants::IN_MILLISECONDS, [this]() -> void
                        {
                            me->RemoveAura(eSpells::CarryingCrate);

                            me->GetMotionMaster()->MovePoint(eMoves::MoveCarryCrate, m_UkuroggCarryCratePos);
                        });

                        break;
                    }
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                UpdateOperations(p_Diff);

                if (!UpdateVictim())
                    return;

                m_Events.Update(p_Diff);

                if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
                    return;

                switch (m_Events.ExecuteEvent())
                {
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new npc_foundry_ukuroggAI(p_Creature);
        }
};

/// Zipline Stalker - 82538
class npc_foundry_zipline_stalker : public CreatureScript
{
    public:
        npc_foundry_zipline_stalker() : CreatureScript("npc_foundry_zipline_stalker") { }

        enum eSpell
        {
            ZiplineStalkerVisual = 166239
        };

        enum eMoves
        {
            MoveNone,
            MoveSecond
        };

        struct npc_foundry_zipline_stalkerAI : public ScriptedAI
        {
            npc_foundry_zipline_stalkerAI(Creature* p_Creature) : ScriptedAI(p_Creature), m_Vehicle(p_Creature->GetVehicleKit()) { }

            Vehicle* m_Vehicle;

            void Reset() override
            {
                me->SetFlag(EUnitFields::UNIT_FIELD_FLAGS_2, eUnitFlags2::UNIT_FLAG2_DISABLE_TURN);

                me->SetReactState(ReactStates::REACT_PASSIVE);

                me->CastSpell(me, eSpell::ZiplineStalkerVisual, true);
            }

            void MovementInform(uint32 p_Type, uint32 p_ID) override
            {
                /*if (p_Type != MovementGeneratorType::POINT_MOTION_TYPE || m_Vehicle == nullptr)
                    return;

                switch (p_ID)
                {
                    case eMoves::MoveSecond:
                    {
                        for (int8 l_SeatID = 0; l_SeatID < MAX_VEHICLE_SEATS; ++l_SeatID)
                        {
                            if (m_Vehicle->GetPassenger(l_SeatID) == nullptr)
                                continue;

                            if (Creature* l_Passenger = m_Vehicle->GetPassenger(l_SeatID)->ToCreature())
                            {
                                if (l_Passenger->IsAIEnabled)
                                    l_Passenger->AI()->DoAction(eIronMaidensActions::ActionZiplineArrived);
                            }
                        }

                        break;
                    }
                    default:
                        break;
                }*/
            }

            void PassengerBoarded(Unit* p_Passenger, int8 p_SeatID, bool p_Apply) override
            {
                /*if (p_Apply)
                {
                    if (p_Passenger->GetEntry() == eFoundryCreatures::BossAdmiralGaran)
                    {
                        if (Creature* l_Garan = p_Passenger->ToCreature())
                        {
                            if (l_Garan->IsAIEnabled)
                                l_Garan->AI()->DoAction(eIronMaidensActions::ActionEnteredZipline);
                        }
                    }
                }*/
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                UpdateOperations(p_Diff);
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new npc_foundry_zipline_stalkerAI(p_Creature);
        }
};

/// Rapid Fire Stalker - 77636
class npc_foundry_rapid_fire_stalker : public CreatureScript
{
    public:
        npc_foundry_rapid_fire_stalker() : CreatureScript("npc_foundry_rapid_fire_stalker") { }

        struct npc_foundry_rapid_fire_stalkerAI : public ScriptedAI
        {
            npc_foundry_rapid_fire_stalkerAI(Creature* p_Creature) : ScriptedAI(p_Creature), m_TargetGUID(0) { }

            uint64 m_TargetGUID;

            void SetGUID(uint64 p_Guid, int32 p_ID /*= 0*/) override
            {
                m_TargetGUID = p_Guid;
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                if (Unit* l_Target = Unit::GetUnit(*me, m_TargetGUID))
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0, *l_Target, false);
                }
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new npc_foundry_rapid_fire_stalkerAI(p_Creature);
        }
};

/// Dominator Turret - 78583
class npc_foundry_dominator_turret : public CreatureScript
{
    public:
        npc_foundry_dominator_turret() : CreatureScript("npc_foundry_dominator_turret") { }

        enum eSpells
        {
            DominatorBlastTimingAura    = 158598,
            DominatorTurretDeathVisual  = 158640
        };

        struct npc_foundry_dominator_turretAI : public ScriptedAI
        {
            npc_foundry_dominator_turretAI(Creature* p_Creature) : ScriptedAI(p_Creature) { }

            void Reset() override
            {
                me->SetReactState(ReactStates::REACT_PASSIVE);

                me->CastSpell(me, eSpells::DominatorBlastTimingAura, true);
            }

            void JustDied(Unit* /*p_Killer*/) override
            {
                me->CastSpell(me, eSpells::DominatorTurretDeathVisual, true);
            }

            void UpdateAI(uint32 const p_Diff) override
            {
                me->SetFacingTo(me->m_orientation + M_PI / 32.0f);
            }
        };

        CreatureAI* GetAI(Creature* p_Creature) const override
        {
            return new npc_foundry_dominator_turretAI(p_Creature);
        }
};

/// Blood Ritual - 158078
class spell_foundry_blood_ritual : public SpellScriptLoader
{
    public:
        spell_foundry_blood_ritual() : SpellScriptLoader("spell_foundry_blood_ritual") { }

        enum eSpell
        {
            CrystallizedBlood = 158080
        };

        class spell_foundry_blood_ritual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_foundry_blood_ritual_SpellScript)

            void CorrectTargets(std::list<WorldObject*>& p_Targets)
            {
                if (p_Targets.empty())
                    return;

                Unit* l_Caster = GetCaster();
                p_Targets.sort(JadeCore::ObjectDistanceOrderPred(l_Caster));

                if (Unit* l_Target = (*p_Targets.begin())->ToUnit())
                    l_Caster->CastSpell(l_Target, eSpell::CrystallizedBlood, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_foundry_blood_ritual_SpellScript::CorrectTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_104);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_foundry_blood_ritual_SpellScript();
        }
};

/// Penetrating Shot (aura) - 164271
class spell_foundry_penetrating_shot : public SpellScriptLoader
{
    public:
        spell_foundry_penetrating_shot() : SpellScriptLoader("spell_foundry_penetrating_shot") { }

        class spell_foundry_penetrating_shot_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_foundry_penetrating_shot_AuraScript)

            enum eSpell
            {
                PenetratingShotDamage = 164279
            };

            void AfterRemove(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
            {
                Unit* l_Caster = GetCaster();
                Unit* l_Target = GetTarget();
                if (l_Caster == nullptr || l_Target == nullptr)
                    return;

                l_Caster->SetFacingTo(l_Caster->GetAngle(l_Target));
                l_Caster->CastSpell(*l_Target, eSpell::PenetratingShotDamage, true);
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_foundry_penetrating_shot_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_foundry_penetrating_shot_AuraScript();
        }
};

/// Penetrating Shot (damage) - 164279
class spell_foundry_penetrating_shot_damage : public SpellScriptLoader
{
    public:
        spell_foundry_penetrating_shot_damage() : SpellScriptLoader("spell_foundry_penetrating_shot_damage") { }

        class spell_foundry_penetrating_shot_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_foundry_penetrating_shot_damage_SpellScript)

            uint32 m_TargetCount;

            bool Load() override
            {
                m_TargetCount = 0;
                return true;
            }

            void CorrectTargets(std::list<WorldObject*>& p_Targets)
            {
                m_TargetCount = uint32(p_Targets.size());
            }

            void HandleDamage(SpellEffIndex p_EffIndex)
            {
                if (!m_TargetCount)
                    return;

                SetHitDamage(GetHitDamage() / m_TargetCount);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_foundry_penetrating_shot_damage_SpellScript::CorrectTargets, EFFECT_0, TARGET_ENNEMIES_IN_CYLINDER);
                OnEffectHitTarget += SpellEffectFn(spell_foundry_penetrating_shot_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_foundry_penetrating_shot_damage_SpellScript();
        }
};

/// Convulsive Shadows - 156214
class spell_foundry_convulsive_shadows : public SpellScriptLoader
{
    public:
        spell_foundry_convulsive_shadows() : SpellScriptLoader("spell_foundry_convulsive_shadows") { }

        enum eSpell
        {
            ShadowExplosion = 156280
        };

        class spell_foundry_convulsive_shadows_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_foundry_convulsive_shadows_SpellScript)

            void HandleLingeringShadow(SpellEffIndex p_EffIndex)
            {
                Unit* l_Caster = GetCaster();
                if (l_Caster == nullptr)
                    return;

                if (!l_Caster->GetMap()->IsMythic())
                    PreventHitEffect(p_EffIndex);
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_foundry_convulsive_shadows_SpellScript::HandleLingeringShadow, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_foundry_convulsive_shadows_SpellScript();
        }

        class spell_foundry_convulsive_shadows_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_foundry_convulsive_shadows_AuraScript)

            void AfterTick(AuraEffect const* p_AurEff)
            {
                p_AurEff->GetBase()->DropStack();
            }

            void HandleDispel(DispelInfo* p_DispelInfo)
            {
                Unit* l_Target = GetUnitOwner();
                if (l_Target == nullptr)
                    return;

                if (InstanceScript* l_InstanceScript = l_Target->GetInstanceScript())
                {
                    if (Creature* l_Sorka = Creature::GetCreature(*l_Target, l_InstanceScript->GetData64(eFoundryCreatures::BossEnforcerSorka)))
                    {
                        int32 l_Damage = 40000 * p_DispelInfo->GetRemovedCharges();

                        l_Sorka->CastCustomSpell(l_Target, eSpell::ShadowExplosion, &l_Damage, nullptr, nullptr, true);
                    }
                }
            }

            void Register() override
            {
                AfterEffectPeriodic += AuraEffectPeriodicFn(spell_foundry_convulsive_shadows_AuraScript::AfterTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
                OnDispel += AuraDispelFn(spell_foundry_convulsive_shadows_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_foundry_convulsive_shadows_AuraScript();
        }
};

/// Dark Hunt - 158315
class spell_foundry_dark_hunt : public SpellScriptLoader
{
    public:
        spell_foundry_dark_hunt() : SpellScriptLoader("spell_foundry_dark_hunt") { }

        class spell_foundry_dark_hunt_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_foundry_dark_hunt_AuraScript)

            enum eSpell
            {
                DarkHuntExecution = 158321
            };

            void AfterRemove(AuraEffect const* /*p_AurEff*/, AuraEffectHandleModes /*p_Mode*/)
            {
                Unit* l_Caster = GetCaster();
                Unit* l_Target = GetTarget();
                if (l_Caster == nullptr || l_Target == nullptr)
                    return;

                l_Caster->CastSpell(l_Target, eSpell::DarkHuntExecution, true);
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_foundry_dark_hunt_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_foundry_dark_hunt_AuraScript();
        }
};

/// Dominator Blast - 158602
class areatrigger_foundry_dominator_blast : public AreaTriggerEntityScript
{
    public:
        areatrigger_foundry_dominator_blast() : AreaTriggerEntityScript("areatrigger_foundry_dominator_blast"), m_DamageCooldown(0) { }

        enum eSpell
        {
            DominatorBlastDoT = 158601
        };

        int32 m_DamageCooldown;

        void OnSetCreatePosition(AreaTrigger* p_AreaTrigger, Unit* p_Caster, Position& /*p_SourcePosition*/, Position& p_DestinationPosition, std::list<Position>& /*p_PathToLinearDestination*/) override
        {
            p_AreaTrigger->SetTimeToTarget(15 * TimeConstants::IN_MILLISECONDS);

            p_DestinationPosition.m_positionX = p_Caster->m_positionX + 80.0f * cos(p_Caster->m_orientation);
            p_DestinationPosition.m_positionY = p_Caster->m_positionY + 80.0f * sin(p_Caster->m_orientation);
        }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time) override
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                if (m_DamageCooldown > 0)
                    m_DamageCooldown -= p_Time;
                else
                {
                    std::list<Player*> l_TargetList;
                    float l_Radius = 1.5f;

                    JadeCore::AnyPlayerInObjectRangeCheck l_Check(p_AreaTrigger, l_Radius);
                    JadeCore::PlayerListSearcher<JadeCore::AnyPlayerInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                    p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                    for (Player* l_Iter : l_TargetList)
                        l_Iter->CastSpell(l_Iter, eSpell::DominatorBlastDoT, true);

                    m_DamageCooldown = 500;
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const override
        {
            return new areatrigger_foundry_dominator_blast();
        }
};

/// Iron Maidens Boat - 9945
class areatrigger_at_foundry_iron_maidens_boat : public AreaTriggerScript
{
    public:
        areatrigger_at_foundry_iron_maidens_boat() : AreaTriggerScript("areatrigger_at_foundry_iron_maidens_boat") { }

        enum eSpell
        {
            OnABoatPeriodic = 158726
        };

        void OnEnter(Player* p_Player, AreaTriggerEntry const* /*p_AreaTrigger*/) override
        {
            p_Player->CastSpell(p_Player, eSpell::OnABoatPeriodic, true);
        }

        void OnExit(Player* p_Player, AreaTriggerEntry const* /*p_AreaTrigger*/) override
        {
            p_Player->RemoveAura(eSpell::OnABoatPeriodic);
        }
};

#ifndef __clang_analyzer__
void AddSC_boss_iron_maidens()
{
    /// Bosses
    new boss_admiral_garan();
    new boss_enforcer_sorka();
    new boss_marak_the_blooded();

    /// Creatures
    new npc_foundry_loading_chain();
    new npc_foundry_ukurogg();
    new npc_foundry_zipline_stalker();
    new npc_foundry_rapid_fire_stalker();
    new npc_foundry_dominator_turret();

    /// Spells
    new spell_foundry_blood_ritual();
    new spell_foundry_penetrating_shot();
    new spell_foundry_penetrating_shot_damage();
    new spell_foundry_convulsive_shadows();
    new spell_foundry_dark_hunt();

    /// AreaTrigger (spell)
    new areatrigger_foundry_dominator_blast();

    /// AreaTrigger (world)
    new areatrigger_at_foundry_iron_maidens_boat();
}
#endif
