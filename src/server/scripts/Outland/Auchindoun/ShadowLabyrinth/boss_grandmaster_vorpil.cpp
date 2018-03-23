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

/*
Name: Boss_Grandmaster_Vorpil
%Complete: 100
Category: Auchindoun, Shadow Labyrinth
*/

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "shadow_labyrinth.h"
#include "TemporarySummon.h"

enum GrandmasterVorpil
{
    SAY_INTRO                   = 0,
    SAY_AGGRO                   = 1,
    SAY_HELP                    = 2,
    SAY_SLAY                    = 3,
    SAY_DEATH                   = 4,

    SPELL_RAIN_OF_FIRE          = 33617,
    H_SPELL_RAIN_OF_FIRE        = 39363,

    SPELL_DRAW_SHADOWS          = 33563,
    SPELL_SHADOWBOLT_VOLLEY     = 33841,
    SPELL_BANISH                = 38791,

    NPC_VOID_TRAVELER           = 19226,
    SPELL_SUMMON_VOID_TRAVELER_A = 33582,
    SPELL_SUMMON_VOID_TRAVELER_B = 33583,
    SPELL_SUMMON_VOID_TRAVELER_C = 33584,
    SPELL_SUMMON_VOID_TRAVELER_D = 33585,
    SPELL_SUMMON_VOID_TRAVELER_E = 33586,

    SPELL_SACRIFICE             = 33587,
    SPELL_SHADOW_NOVA           = 33846,
    SPELL_EMPOWERING_SHADOWS    = 33783,

    NPC_VOID_PORTAL             = 19224,
    SPELL_SUMMON_PORTAL         = 33566,
    SPELL_VOID_PORTAL_VISUAL    = 33569
};

Position const VorpilPosition = { -252.8820f, -264.3030f, 17.1f, 0.0f };

Position const VoidPortalPosition[5] =
{
    {-283.5894f, -239.5718f, 12.7f, 0.0f},
    {-306.5853f, -258.4539f, 12.7f, 0.0f},
    {-295.8789f, -269.0899f, 12.7f, 0.0f},
    {-209.3401f, -262.7564f, 17.1f, 0.0f},
    {-261.4533f, -297.3298f, 17.1f, 0.0f}
};

enum Events
{
    EVENT_SHADOWBOLT_VOLLEY     = 1,
    EVENT_BANISH,
    EVENT_DRAW_SHADOWS,
    EVENT_SUMMON_TRAVELER,
    EVENT_MOVE
};

class boss_grandmaster_vorpil : public CreatureScript
{
    public:
        boss_grandmaster_vorpil() : CreatureScript("boss_grandmaster_vorpil") { }

        struct boss_grandmaster_vorpilAI : public BossAI
        {
            boss_grandmaster_vorpilAI(Creature* creature) : BossAI(creature, DATA_GRANDMASTER_VORPIL)
            {
                intro = false;
            }

            void Reset() override
            {
                BossAI::Reset();
                helpYell = false;
            }

            void SummonPortals()
            {
                for (uint8 i = 0; i < 5; ++i)
                    if (Creature* portal = me->SummonCreature(NPC_VOID_PORTAL, VoidPortalPosition[i], TEMPSUMMON_CORPSE_DESPAWN, 3000000))
                        portal->CastSpell(portal, SPELL_VOID_PORTAL_VISUAL, true);

                events.ScheduleEvent(EVENT_SUMMON_TRAVELER, Seconds(5));
            }

            void spawnVoidTraveler()
            {
                uint8 pos = urand(0, 4);
                me->SummonCreature(NPC_VOID_TRAVELER, VoidPortalPosition[pos], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                if (!helpYell)
                {
                    Talk(SAY_HELP);
                    helpYell = true;
                }
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* killer) override
            {
                BossAI::JustDied(killer);
                Talk(SAY_DEATH);
            }

            void JustEngagedWith(Unit* who) override
            {
                BossAI::JustEngagedWith(who);
                events.ScheduleEvent(EVENT_SHADOWBOLT_VOLLEY, randtime(Seconds(7), Seconds(14)));
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_BANISH, Seconds(17));
                events.ScheduleEvent(EVENT_DRAW_SHADOWS, Seconds(45));
                events.ScheduleEvent(EVENT_SUMMON_TRAVELER, Minutes(1) + Seconds(30));

                Talk(SAY_AGGRO);
                SummonPortals();
            }

            void MoveInLineOfSight(Unit* who) override
            {
                BossAI::MoveInLineOfSight(who);

                if (!intro && me->IsWithinLOSInMap(who) && me->IsWithinDistInMap(who, 100) && me->IsValidAttackTarget(who))
                {
                    Talk(SAY_INTRO);
                    intro = true;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHADOWBOLT_VOLLEY:
                            DoCastAOE(SPELL_SHADOWBOLT_VOLLEY);
                            events.Repeat(randtime(Seconds(15), Seconds(30)));
                            break;
                        case EVENT_BANISH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, false))
                                 DoCast(target, SPELL_BANISH);
                            events.Repeat(Seconds(16));
                            break;
                        case EVENT_DRAW_SHADOWS:
                            {
                                Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                    if (Player* i_pl = i->GetSource())
                                        if (i_pl->IsAlive() && !i_pl->HasAura(SPELL_BANISH))
                                            i_pl->TeleportTo(me->GetMapId(), VorpilPosition.GetPositionX(), VorpilPosition.GetPositionY(), VorpilPosition.GetPositionZ(), VorpilPosition.GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);

                                me->UpdatePosition(VorpilPosition);
                                DoCastAOE(SPELL_DRAW_SHADOWS, true);
                                DoCastAOE(SPELL_RAIN_OF_FIRE);
                                events.RescheduleEvent(EVENT_SHADOWBOLT_VOLLEY, Seconds(6));
                                events.Repeat(Seconds(30));
                                break;
                            }
                        case EVENT_SUMMON_TRAVELER:
                            spawnVoidTraveler();
                            // enrage at 20%
                            if (HealthBelowPct(20))
                                events.Repeat(Seconds(5));
                            else
                                events.Repeat(Seconds(10));
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                DoMeleeAttackIfReady();
            }

            private:
                bool intro;
                bool helpYell;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetShadowLabyrinthAI<boss_grandmaster_vorpilAI>(creature);
        }
};

class npc_voidtraveler : public CreatureScript
{
    public:
        npc_voidtraveler() : CreatureScript("npc_voidtraveler") { }

        struct npc_voidtravelerAI : public ScriptedAI
        {
            npc_voidtravelerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset() override
            {
                events.Reset();
                sacrificed = false;
                events.ScheduleEvent(EVENT_MOVE, 0);
            }

            void JustEngagedWith(Unit* /*who*/) override { }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_MOVE)
                {
                    if (Creature* Vorpil = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_GRANDMASTER_VORPIL)))
                    {
                        if (sacrificed)
                        {
                            DoCastAOE(SPELL_EMPOWERING_SHADOWS, true);
                            DoCastAOE(SPELL_SHADOW_NOVA, true);
                            me->KillSelf();
                            return;
                        }

                        me->GetMotionMaster()->MoveFollow(Vorpil, 0, 0);

                        if (me->IsWithinDist(Vorpil, 3.0f))
                        {
                            DoCastSelf(SPELL_SACRIFICE);
                            sacrificed = true;
                            events.Repeat(500);
                            return;
                        }
                    }
                    events.Repeat(Seconds(1));
                }
            }

        private:
            InstanceScript* instance;
            EventMap events;
            bool sacrificed;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetShadowLabyrinthAI<npc_voidtravelerAI>(creature);
        }
};

void AddSC_boss_grandmaster_vorpil()
{
    new boss_grandmaster_vorpil();
    new npc_voidtraveler();
}
