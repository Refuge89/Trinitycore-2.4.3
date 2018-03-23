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
SDName: Boss_Ambassador_Hellmaw
SD%Complete: 80
SDComment: Enrage spell missing/not known
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedEscortAI.h"
#include "shadow_labyrinth.h"

enum Yells
{
    SAY_INTRO               = 0,
    SAY_AGGRO               = 1,
    SAY_HELP                = 2,
    SAY_SLAY                = 3,
    SAY_DEATH               = 4
};

enum Spells
{
    SPELL_BANISH            = 30231,
    SPELL_CORROSIVE_ACID    = 33551,
    SPELL_FEAR              = 33547,
    SPELL_ENRAGE            = 34970
};

enum Events
{
    EVENT_CORROSIVE_ACID    = 1,
    EVENT_FEAR,
    EVENT_BERSERK
};

class boss_ambassador_hellmaw : public CreatureScript
{
    public:
        boss_ambassador_hellmaw() : CreatureScript("boss_ambassador_hellmaw") { }

        struct boss_ambassador_hellmawAI : public EscortAI
        {
            boss_ambassador_hellmawAI(Creature* creature) : EscortAI(creature)
            {
                instance = creature->GetInstanceScript();
                intro = false;
                me->setActive(true);
            }

            void Reset() override
            {
                if (!me->IsAlive())
                    return;

                events.Reset();
                instance->SetBossState(DATA_AMBASSADOR_HELLMAW, NOT_STARTED);

                events.ScheduleEvent(EVENT_CORROSIVE_ACID, randtime(Seconds(5), Seconds(10)));
                events.ScheduleEvent(EVENT_FEAR, randtime(Seconds(25), Seconds(30)));
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_BERSERK, Minutes(3));

                DoAction(ACTION_AMBASSADOR_HELLMAW_BANISH);
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (me->HasAura(SPELL_BANISH))
                    return;

                EscortAI::MoveInLineOfSight(who);
            }

            void DoAction(int32 actionId) override
            {
                if (actionId == ACTION_AMBASSADOR_HELLMAW_INTRO)
                    DoIntro();
                else if (actionId == ACTION_AMBASSADOR_HELLMAW_BANISH)
                    if (instance->GetData(DATA_FEL_OVERSEER) && !me->HasAura(SPELL_BANISH))
                        DoCastSelf(SPELL_BANISH, true);
            }

            void DoIntro()
            {
                if (intro)
                    return;

                intro = true;

                if (me->HasAura(SPELL_BANISH))
                    me->RemoveAurasDueToSpell(SPELL_BANISH);

                Talk(SAY_INTRO);
                Start(true, false, ObjectGuid::Empty, nullptr, false, true);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                instance->SetBossState(DATA_AMBASSADOR_HELLMAW, IN_PROGRESS);
                Talk(SAY_AGGRO);
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                instance->SetBossState(DATA_AMBASSADOR_HELLMAW, DONE);
                Talk(SAY_DEATH);
            }

            void UpdateEscortAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasAura(SPELL_BANISH))
                {
                    EnterEvadeMode(EVADE_REASON_OTHER);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CORROSIVE_ACID:
                            DoCastVictim(SPELL_CORROSIVE_ACID);
                            events.Repeat(randtime(Seconds(15), Seconds(25)));
                            break;
                        case EVENT_FEAR:
                            DoCastAOE(SPELL_FEAR);
                            events.Repeat(randtime(Seconds(20), Seconds(35)));
                            break;
                        case EVENT_BERSERK:
                            DoCastSelf(SPELL_ENRAGE, true);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
            EventMap events;
            bool intro;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetShadowLabyrinthAI<boss_ambassador_hellmawAI>(creature);
        }
};

void AddSC_boss_ambassador_hellmaw()
{
    new boss_ambassador_hellmaw();
}
