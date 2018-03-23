/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "naxxramas.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_HATEFUL_STRIKE                        = 28308,
    SPELL_FRENZY                                = 28131,
    SPELL_BERSERK                               = 26662,
    SPELL_SLIME_BOLT                            = 32309
};

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_SLAY                                    = 1,
    SAY_DEATH                                   = 2,
    EMOTE_BERSERK                               = 3,
    EMOTE_FRENZY                                = 4
};

enum Events
{
    EVENT_BERSERK = 1,
    EVENT_HATEFUL,
    EVENT_SLIME
};

enum Misc
{
    ACHIEV_MAKE_QUICK_WERK_OF_HIM_STARTING_EVENT  = 10286
};

enum HatefulThreatAmounts
{
    HATEFUL_THREAT_AMT  = 1000,
};

class boss_patchwerk : public CreatureScript
{
    public:
        boss_patchwerk() : CreatureScript("boss_patchwerk") { }

        struct boss_patchwerkAI : public BossAI
        {
            boss_patchwerkAI(Creature* creature) : BossAI(creature, BOSS_PATCHWERK)
            {
                enraged = false;
            }

            void KilledUnit(Unit* /*Victim*/) override
            {
                if (!(rand32() % 5))
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
                enraged = false;
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_HATEFUL, Seconds(1));
                events.ScheduleEvent(EVENT_BERSERK, Minutes(7));
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
                        case EVENT_HATEFUL:
                        {
                            // Hateful Strike targets the highest hp non-MT in melee range
                            Unit* hatefulTarget = NULL;
                            uint32 mostHP = 0;
                            std::list<Unit*> inRangeThreatList;
                            ThreatManager const& mgr = me->GetThreatManager();
                            Unit* currentVictim = mgr.GetCurrentVictim();
                            auto list = mgr.GetModifiableThreatList();
                            for (auto it = list.begin(), end = list.end(); it != end; ++it)
                            {
                                Unit* target = (*it)->GetVictim();
                                if (target->IsAlive() && target->GetTypeId() == TYPEID_PLAYER && me->IsWithinMeleeRange(target))
                                {
                                    inRangeThreatList.push_back(target);
                                    if (target != currentVictim && target->GetHealth() >= mostHP)
                                    {
                                        mostHP = target->GetHealth();
                                        hatefulTarget = target;
                                    }
                                }
                            }

                            if (!hatefulTarget)
                                hatefulTarget = currentVictim;

                            DoCast(hatefulTarget, SPELL_HATEFUL_STRIKE, true);

                            if (inRangeThreatList.size() > 3)
                                inRangeThreatList.resize(3);
                            for (std::list<Unit*>::iterator itr = inRangeThreatList.begin(); itr != inRangeThreatList.end(); ++itr)
                                me->GetThreatManager().AddThreat((*itr), HATEFUL_THREAT_AMT);

                            events.Repeat(Seconds(1));
                            break;
                        }
                        case EVENT_BERSERK:
                            DoCastSelf(SPELL_BERSERK, true);
                            Talk(EMOTE_BERSERK);
                            events.ScheduleEvent(EVENT_SLIME, Seconds(2));
                            break;
                        case EVENT_SLIME:
                            DoCastAOE(SPELL_SLIME_BOLT, true);
                            events.Repeat(Seconds(2));
                            break;
                    }
                }

                if (!enraged && HealthBelowPct(5))
                {
                    DoCastSelf(SPELL_FRENZY, true);
                    Talk(EMOTE_FRENZY);
                    enraged = true;
                }

                DoMeleeAttackIfReady();
            }
        private:
            bool enraged;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetNaxxramasAI<boss_patchwerkAI>(creature);
        }
};

void AddSC_boss_patchwerk()
{
    new boss_patchwerk();
}
