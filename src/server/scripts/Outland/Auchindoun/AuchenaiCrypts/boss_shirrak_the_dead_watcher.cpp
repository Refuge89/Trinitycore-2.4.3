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
Name: Boss_Shirrak_the_dead_watcher
%Complete: 80
Comment: InhibitMagic should stack slower far from the boss, proper Visual for Focus Fire, heroic implemented
Category: Auchindoun, Auchenai Crypts
EndScriptData */

#include "ScriptMgr.h"
#include "auchenai_crypts.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_INHIBITMAGIC          = 32264,
    SPELL_ATTRACTMAGIC          = 32265,
    SPELL_CARNIVOROUSBITE       = 36383,
    SPELL_FIERY_BLAST           = 32302,
    SPELL_FOCUS_FIRE_VISUAL     = 42075 //need to find better visual
};

enum Events
{
    EVENT_ATTRACT_MAGIC         = 1,
    EVENT_CARNIVOROUS_BITE,
    EVENT_FOCUS_FIRE,
    EVENT_FIERY_BLAST
};

enum Say
{
    EMOTE_FOCUSED               = 0
};

enum Creatures
{
    NPC_FOCUS_FIRE              = 18374
};

class boss_shirrak_the_dead_watcher : public CreatureScript
{
    public:
        boss_shirrak_the_dead_watcher() : CreatureScript("boss_shirrak_the_dead_watcher") { }

        struct boss_shirrak_the_dead_watcherAI : public BossAI
        {
            boss_shirrak_the_dead_watcherAI(Creature* creature) : BossAI(creature, DATA_SHIRRAK_THE_DEAD_WATCHER) { }

            void Reset() override
            {
                BossAI::Reset();
                focusedTargetGUID.Clear();

                scheduler.Schedule(Seconds(3), Seconds(4), [this](TaskContext task)
                {
                    DoCastAOE(SPELL_INHIBITMAGIC);
                    task.Repeat();
                });
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_ATTRACT_MAGIC, Seconds(28));
                events.ScheduleEvent(EVENT_CARNIVOROUS_BITE, Seconds(10));
                events.ScheduleEvent(EVENT_FOCUS_FIRE, Seconds(17));
            }

            void JustSummoned(Creature* summoned) override
            {
                if (summoned && summoned->GetEntry() == NPC_FOCUS_FIRE)
                {
                    summoned->CastSpell(summoned, SPELL_FOCUS_FIRE_VISUAL);
                    summoned->SetFaction(me->GetFaction());
                    summoned->SetLevel(me->getLevel());
                    summoned->AddUnitState(UNIT_STATE_ROOT);

                    if (Unit* focusedTarget = ObjectAccessor::GetUnit(*me, focusedTargetGUID))
                        summoned->AI()->AttackStart(focusedTarget);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                scheduler.Update(diff);

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ATTRACT_MAGIC:
                            DoCastAOE(SPELL_ATTRACTMAGIC);
                            events.Repeat(Seconds(30));
                            events.RescheduleEvent(EVENT_CARNIVOROUS_BITE, 1500);
                            break;
                        case EVENT_CARNIVOROUS_BITE:
                            DoCastAOE(SPELL_CARNIVOROUSBITE);
                            events.Repeat(Seconds(10));
                            break;
                        case EVENT_FOCUS_FIRE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, [&](Unit* u) { return u && u->IsAlive() && u->GetTypeId() == TYPEID_PLAYER; }))
                            {
                                focusedTargetGUID = target->GetGUID();
                                me->SummonCreature(NPC_FOCUS_FIRE, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 5500);
                                Talk(EMOTE_FOCUSED, target);
                            }
                            events.Repeat(randtime(Seconds(15), Seconds(20)));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            ObjectGuid focusedTargetGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAuchenaiCryptsAI<boss_shirrak_the_dead_watcherAI>(creature);
        }
};

class npc_focus_fire : public CreatureScript
{
    public:
        npc_focus_fire() : CreatureScript("npc_focus_fire") { }

        struct npc_focus_fireAI : public ScriptedAI
        {
            npc_focus_fireAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                events.Reset();
                fiery = 0;
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_FIERY_BLAST, randtime(Seconds(3), Seconds(4)));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_FIERY_BLAST)
                {
                    DoCastAOE(SPELL_FIERY_BLAST);
                    if (++fiery <= 2)
                        events.Repeat(Seconds(1));
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap events;
            uint32 fiery;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAuchenaiCryptsAI<npc_focus_fireAI>(creature);
        }
};

void AddSC_boss_shirrak_the_dead_watcher()
{
    new boss_shirrak_the_dead_watcher();
    new npc_focus_fire();
}
