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
SDName: Boss_Omar_The_Unscarred
SD%Complete: 90
SDComment: Temporary solution for orbital/shadow whip-ability. Needs more core support before making it more proper.
SDCategory: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "ScriptMgr.h"
#include "hellfire_ramparts.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"

enum Says
{
    SAY_AGGRO                    = 0,
    SAY_SUMMON                   = 1,
    SAY_CURSE                    = 2,
    SAY_KILL_1                   = 3,
    SAY_DIE                      = 4,
    SAY_WIPE                     = 5
};

enum Spells
{
    SPELL_ORBITAL_STRIKE         = 30637,
    SPELL_SHADOW_WHIP            = 30638,
    SPELL_TREACHEROUS_AURA       = 30695,
    H_SPELL_BANE_OF_TREACHERY    = 37566,
    SPELL_DEMONIC_SHIELD         = 31901,
    SPELL_SHADOW_BOLT            = 30686,
    H_SPELL_SHADOW_BOLT          = 39297,
    SPELL_SUMMON_FIENDISH_HOUND  = 30707
};

enum Events
{
    EVENT_ORBITAL_STRIKE         = 1,
    EVENT_SHADOW_WHIP,
    EVENT_TREACHEROUS,
    EVENT_SHADOW_BOLT,
    EVENT_SUMMON_HOUND,
    EVENT_DEMONIC_SHIELD
};

class boss_omor_the_unscarred : public CreatureScript
{
    public:
        boss_omor_the_unscarred() : CreatureScript("boss_omor_the_unscarred") { }

        struct boss_omor_the_unscarredAI : public BossAI
        {
            boss_omor_the_unscarredAI(Creature* creature) : BossAI(creature, DATA_OMOR_THE_UNSCARRED)
            {
                SetCombatMovement(false);
            }

            void Reset() override
            {
                Talk(SAY_WIPE);
                BossAI::Reset();
                SummonedCount = 0;
                PlayerGUID.Clear();
                shield = false;
            }

            void JustEngagedWith(Unit* who) override
            {
                BossAI::JustEngagedWith(who);
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_ORBITAL_STRIKE, Seconds(25));
                events.ScheduleEvent(EVENT_TREACHEROUS, Seconds(10));
                events.ScheduleEvent(EVENT_SHADOW_BOLT, Seconds(3));
                events.ScheduleEvent(EVENT_SUMMON_HOUND, Seconds(10));
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                if (rand32() % 2)
                    return;

                Talk(SAY_KILL_1);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (!shield && me->HealthBelowPctDamaged(20, damage))
                {
                    shield = true;
                    DoCastSelf(SPELL_DEMONIC_SHIELD);
                    events.ScheduleEvent(EVENT_DEMONIC_SHIELD, Seconds(15));
                }
            }

            void JustSummoned(Creature* summoned) override
            {
                Talk(SAY_SUMMON);

                if (Unit* random = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    summoned->AI()->AttackStart(random);

                ++SummonedCount;
            }

            void JustDied(Unit* killer) override
            {
                Talk(SAY_DIE);
                BossAI::JustDied(killer);
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
                        case EVENT_SUMMON_HOUND:
                            if (SummonedCount < 2)
                                DoCastSelf(SPELL_SUMMON_FIENDISH_HOUND, true);
                            events.Repeat(randtime(Seconds(15), Seconds(30)));
                            break;
                        case EVENT_TREACHEROUS:
                            Talk(SAY_CURSE);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                                DoCast(target, SPELL_TREACHEROUS_AURA);
                            events.Repeat(randtime(Seconds(8), Seconds(16)));
                            break;
                        case EVENT_SHADOW_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_MAXTHREAT, 0, -7.0f, true))
                                DoCast(target, SPELL_SHADOW_BOLT);
                            events.Repeat(randtime(Seconds(4), Seconds(7)));
                            break;
                        case EVENT_ORBITAL_STRIKE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 7.0f, true))
                            {
                                PlayerGUID = target->GetGUID();
                                DoCast(target, SPELL_ORBITAL_STRIKE);
                                events.ScheduleEvent(EVENT_SHADOW_WHIP, Seconds(1));
                            }
                            events.Repeat(randtime(Seconds(14), Seconds(16)));
                            break;
                        case EVENT_SHADOW_WHIP:
                            if (Player* player = ObjectAccessor::GetPlayer(*me, PlayerGUID))
                                DoCast(player, SPELL_SHADOW_WHIP);
                            PlayerGUID.Clear();
                            break;
                        case EVENT_DEMONIC_SHIELD:
                            DoCastSelf(SPELL_DEMONIC_SHIELD);
                            events.Repeat(Seconds(15));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                uint32 SummonedCount;
                ObjectGuid PlayerGUID;
                bool shield;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetHellfireRampartsAI<boss_omor_the_unscarredAI>(creature);
        }
};

void AddSC_boss_omor_the_unscarred()
{
    new boss_omor_the_unscarred();
}
