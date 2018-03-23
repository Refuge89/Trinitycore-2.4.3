/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: Boss_Vazruden_the_Herald
%Complete: 90
Comment:
Category: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "ScriptMgr.h"
#include "hellfire_ramparts.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "TemporarySummon.h"

enum Says
{
    SAY_INTRO                     = 0,

    SAY_WIPE                      = 0,
    SAY_AGGRO                     = 1,
    SAY_KILL                      = 2,
    SAY_DIE                       = 3,

    EMOTE                         = 0
};

enum Spells
{
    SPELL_FIREBALL                = 34653,
    SPELL_CONE_OF_FIRE            = 30926,
    SPELL_SUMMON_LIQUID_FIRE      = 23971,
    SPELL_SUMMON_LIQUID_FIRE_H    = 30928,
    SPELL_BELLOWING_ROAR          = 39427,
    SPELL_REVENGE                 = 19130,
    SPELL_REVENGE_H               = 40392,
    SPELL_KIDNEY_SHOT             = 30621,
    SPELL_FIRE_NOVA_VISUAL        = 19823
};

enum Events
{
    EVENT_FIREBALL                = 1,
    EVENT_TURN,
    EVENT_BELLOWING_ROAR,
    EVENT_CONE_OF_FIRE,
    EVENT_REVENGE,
    EVENT_KIDNEY_SHOT
};

Position const VazrudenMiddle = { -1406.5f, 1746.5f, 81.2f, 0.0f };

Position const VazrudenRing[2] =
{
    { -1430.0f, 1705.0f, 112.0f, 0.0f },
    { -1377.0f, 1760.0f, 112.0f, 0.0f }
};

enum Misc
{
    POINT_LAND = 1,

    ACTION_ENGAGE = 1
};

class boss_nazan : public CreatureScript
{
    public:
        boss_nazan() : CreatureScript("boss_nazan") { }

        struct boss_nazanAI : public BossAI
        {
            boss_nazanAI(Creature* creature) : BossAI(creature, DATA_NAZAN)
            {
                flight = true;
            }

            void Reset() override
            {
                BossAI::Reset();
            }

            void DoAction(int32 action) override
            {
                if (action == ACTION_ENGAGE)
                {
                    flight = false;
                    events.CancelEvent(EVENT_TURN);
                    if (IsHeroic())
                        events.ScheduleEvent(EVENT_BELLOWING_ROAR, Seconds(6));
                    events.ScheduleEvent(EVENT_CONE_OF_FIRE, Seconds(12));
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(POINT_LAND, VazrudenMiddle);
                    Talk(EMOTE);
                }
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_FIREBALL, Seconds(4));
                events.ScheduleEvent(EVENT_TURN, Seconds(0));
            }

            void AttackStart(Unit* who) override
            {
                if (!who)
                    return;

                me->Attack(who, true);
                if (!flight)
                    me->GetMotionMaster()->MoveChase(who);
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == POINT_LAND)
                {
                    me->SetWalk(true);
                    if (Unit* victim = SelectTarget(SELECT_TARGET_MINDISTANCE, 0))
                        AttackStart(victim);
                    DoStartMovement(me->GetVictim());
                }
            }

            void JustSummoned(Creature* summoned) override
            {
                if (summoned && summoned->GetEntry() == NPC_LIQUID_FIRE)
                {
                    summoned->SetLevel(me->getLevel());
                    summoned->SetFaction(me->GetFaction());
                    summoned->CastSpell(summoned, DUNGEON_MODE(SPELL_SUMMON_LIQUID_FIRE, SPELL_SUMMON_LIQUID_FIRE_H), true);
                    summoned->CastSpell(summoned, SPELL_FIRE_NOVA_VISUAL, true);
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* entry) override
            {
                if (target && entry->Id == uint32(SPELL_FIREBALL))
                    me->SummonCreature(NPC_LIQUID_FIRE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBALL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_FIREBALL, true);
                            events.Repeat(randtime(Seconds(4), Seconds(7)));
                            break;
                        case EVENT_BELLOWING_ROAR:
                            DoCastAOE(SPELL_BELLOWING_ROAR);
                            events.Repeat(Seconds(45));
                            break;
                        case EVENT_CONE_OF_FIRE:
                            DoCastAOE(SPELL_CONE_OF_FIRE);
                            events.Repeat(Seconds(12));
                            break;
                        case EVENT_TURN:
                            uint32 waypoint = urand(0, 1);
                            if (!me->IsWithinDist3d(&VazrudenRing[waypoint], 5.0f))
                                me->GetMotionMaster()->MovePoint(0, VazrudenRing[waypoint]);
                            events.Repeat(Seconds(10));
                            break;
                    }
                }

                if (!flight)
                    DoMeleeAttackIfReady();
            }

            private:
                bool flight;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetHellfireRampartsAI<boss_nazanAI>(creature);
        }
};

class boss_vazruden : public CreatureScript
{
    public:
        boss_vazruden() : CreatureScript("boss_vazruden") { }

        struct boss_vazrudenAI : public BossAI
        {
            boss_vazrudenAI(Creature* creature) : BossAI(creature, DATA_VAZRUDEN) { }

            void Reset() override
            {
                BossAI::Reset();
                WipeSaid = false;
                UnsummonCheck = 2000;
            }

            void JustEngagedWith(Unit* who) override
            {
                Talk(SAY_AGGRO);
                BossAI::JustEngagedWith(who);
                events.ScheduleEvent(EVENT_REVENGE, Seconds(4));
            }

            void KilledUnit(Unit* who) override
            {
                if (who && who->GetEntry() != NPC_VAZRUDEN)
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* killer) override
            {
                if (killer && killer != me)
                    Talk(SAY_DIE);
                BossAI::JustDied(killer);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                {
                    if (UnsummonCheck < diff && me->IsAlive())
                    {
                        if (!WipeSaid)
                        {
                            Talk(SAY_WIPE);
                            WipeSaid = true;
                        }
                        me->DisappearAndDie();
                    }
                    else
                        UnsummonCheck -= diff;
                    return;
                }

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_REVENGE)
                {
                    DoCastVictim(DUNGEON_MODE(SPELL_REVENGE, SPELL_REVENGE_H));
                    events.Repeat(Seconds(5));
                }

                DoMeleeAttackIfReady();
            }

            private:
                bool WipeSaid;
                uint32 UnsummonCheck;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetHellfireRampartsAI<boss_vazrudenAI>(creature);
        }
};

class boss_vazruden_the_herald : public CreatureScript
{
    public:
        boss_vazruden_the_herald() : CreatureScript("boss_vazruden_the_herald") { }

        struct boss_vazruden_the_heraldAI : public BossAI
        {
            boss_vazruden_the_heraldAI(Creature* creature) : BossAI(creature, DATA_VAZRUDEN_THE_HERALD)
            {
                Initialize();
                summoned = false;
                sentryDown = false;
            }

            void Initialize()
            {
                phase = 0;
                waypoint = 0;
                check = 0;
            }

            void Reset() override
            {
                Initialize();
                UnsummonAdds();
                BossAI::Reset();
            }

            void UnsummonAdds()
            {
                if (summoned)
                {
                    Creature* Nazan = ObjectAccessor::GetCreature(*me, NazanGUID);
                    if (!Nazan)
                        Nazan = me->FindNearestCreature(NPC_NAZAN, SIZE_OF_GRIDS);
                    if (Nazan)
                    {
                        Nazan->DisappearAndDie();
                        NazanGUID.Clear();
                    }

                    Creature* Vazruden = ObjectAccessor::GetCreature(*me, VazrudenGUID);
                    if (!Vazruden)
                        Vazruden = me->FindNearestCreature(NPC_VAZRUDEN, SIZE_OF_GRIDS);
                    if (Vazruden)
                    {
                        Vazruden->DisappearAndDie();
                        VazrudenGUID.Clear();
                    }
                    summoned = false;
                    me->ClearUnitState(UNIT_STATE_ROOT);
                    me->SetVisible(true);
                }
            }

            void SummonAdds()
            {
                if (!summoned)
                {
                    if (Creature* Vazruden = me->SummonCreature(NPC_VAZRUDEN, VazrudenMiddle, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000))
                        VazrudenGUID = Vazruden->GetGUID();
                    if (Creature* Nazan = me->SummonCreature(NPC_NAZAN, VazrudenMiddle, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000))
                        NazanGUID = Nazan->GetGUID();
                    summoned = true;
                    me->SetVisible(false);
                    me->AddUnitState(UNIT_STATE_ROOT);
                }
            }

            void JustEngagedWith(Unit* who) override
            {
                if (phase == 0)
                {
                    BossAI::JustEngagedWith(who);
                    phase = 1;
                    check = 0;
                    Talk(SAY_INTRO);
                }
            }

            void JustSummoned(Creature* summon) override
            {
                if (!summon)
                    return;

                if (summon->GetEntry() == NPC_NAZAN)
                    summon->SetSpeedRate(MOVE_FLIGHT, 2.5f);

                if (Unit* victim = me->GetVictim())
                    summon->AI()->AttackStart(victim);
            }

            void SentryDownBy(Unit* killer)
            {
                if (sentryDown)
                {
                    AttackStartNoMove(killer);
                    sentryDown = false;
                }
                else
                    sentryDown = true;
            }

            void UpdateAI(uint32 diff) override
            {
                switch (phase)
                {
                    case 0: // circle around the platform
                        return;
                        break;
                    case 1: // go to the middle and begin the fight
                        if (check <= diff)
                        {
                            if (!me->IsWithinDist3d(&VazrudenMiddle, 5.0f))
                            {
                                me->GetMotionMaster()->Clear();
                                me->GetMotionMaster()->MovePoint(0, VazrudenMiddle);
                                check = 1000;
                            }
                            else
                            {
                                SummonAdds();
                                phase = 2;
                                return;
                            }
                        }
                        else
                            check -= diff;
                        break;
                    default: // adds do the job now
                        if (check <= diff)
                        {
                            Creature* Nazan = ObjectAccessor::GetCreature(*me, NazanGUID);
                            Creature* Vazruden = ObjectAccessor::GetCreature(*me, VazrudenGUID);
                            if (!Nazan && !Vazruden)
                                me->KillSelf();
                            if (((Nazan && !Nazan->IsAlive()) && !Vazruden) ||
                                ((Vazruden && !Vazruden->IsAlive()) && !Nazan))
                                me->KillSelf();
                            if ((Nazan && !Nazan->IsAlive()) && Vazruden && !Vazruden->IsAlive())
                                me->KillSelf();

                            if ((Nazan && Nazan->IsAlive()) || (Vazruden && Vazruden->IsAlive()))
                            {
                                if ((Nazan && Nazan->IsInCombat()) || (Vazruden && Vazruden->IsInCombat()))
                                {
                                    if (((Nazan && Nazan->IsAlive()) && (Vazruden && (!Vazruden->IsAlive() || Vazruden->HealthBelowPct(50)))) ||
                                        (Nazan && Nazan->IsAlive() && Nazan->HealthBelowPct(20)))
                                        Nazan->AI()->DoAction(ACTION_ENGAGE);
                                }
                                else
                                {
                                    UnsummonAdds();
                                    EnterEvadeMode();
                                    return;
                                }
                            }
                            check = 2000;
                        }
                        else
                            check -= diff;
                        break;
                }
            }

            private:
                uint32 phase;
                uint32 waypoint;
                uint32 check;
                bool sentryDown;
                ObjectGuid NazanGUID;
                ObjectGuid VazrudenGUID;
                bool summoned;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetHellfireRampartsAI<boss_vazruden_the_heraldAI>(creature);
        }
};

class npc_hellfire_sentry : public CreatureScript
{
    public:
        npc_hellfire_sentry() : CreatureScript("npc_hellfire_sentry") { }

        struct npc_hellfire_sentryAI : public ScriptedAI
        {
            npc_hellfire_sentryAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_KIDNEY_SHOT, randtime(Seconds(3), Seconds(7)));
            }

            void JustDied(Unit* killer) override
            {
                if (!killer)
                    return;

                if (Creature* herald = me->FindNearestCreature(NPC_VAZRUDEN_HERALD, 150))
                    ENSURE_AI(boss_vazruden_the_herald::boss_vazruden_the_heraldAI, herald->AI())->SentryDownBy(killer);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_KIDNEY_SHOT)
                {
                    DoCastVictim(SPELL_KIDNEY_SHOT);
                    events.Repeat(Seconds(20));
                }

                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetHellfireRampartsAI<npc_hellfire_sentryAI>(creature);
        }
};
void AddSC_boss_vazruden_the_herald()
{
    new boss_vazruden_the_herald();
    new boss_vazruden();
    new boss_nazan();
    new npc_hellfire_sentry();
}
