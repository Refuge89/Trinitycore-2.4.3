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
SDName: Boss_Exarch_Maladaar
SD%Complete: 95
SDComment: Most of event implemented, some adjustments to timers remain and possibly make some better code for switching his dark side in to better "images" of player.
SDCategory: Auchindoun, Auchenai Crypts
EndScriptData */

/* ContentData
npc_stolen_soul
boss_exarch_maladaar
npc_avatar_of_martyred
EndContentData */

#include "ScriptMgr.h"
#include "auchenai_crypts.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"

enum Says
{
    SAY_INTRO                   = 0,
    SAY_SUMMON                  = 1,
    SAY_AGGRO                   = 2,
    SAY_ROAR                    = 3,
    SAY_SLAY                    = 4,
    SAY_DEATH                   = 5
};

enum Spells
{
    SPELL_MOONFIRE              = 37328,
    SPELL_FIREBALL              = 37329,
    SPELL_MIND_FLAY             = 37330,
    SPELL_HEMORRHAGE            = 37331,
    SPELL_FROSTSHOCK            = 37332,
    SPELL_CURSE_OF_AGONY        = 37334,
    SPELL_MORTAL_STRIKE         = 37335,
    SPELL_FREEZING_TRAP         = 37368,
    SPELL_HAMMER_OF_JUSTICE     = 37369,

    SPELL_RIBBON_OF_SOULS       = 32422,
    SPELL_SOUL_SCREAM           = 32421,
    SPELL_STOLEN_SOUL           = 32346,
    SPELL_STOLEN_SOUL_VISUAL    = 32395,
    SPELL_SUMMON_AVATAR         = 32424,

    // Avatar of Martyred
    SPELL_AV_MORTAL_STRIKE      = 16856,
    SPELL_AV_SUNDER_ARMOR       = 16145
};

enum Events
{
    EVENT_CLASS_SPELL           = 1,
    EVENT_FEAR,
    EVENT_RIBBON_OF_SOULS,
    EVENT_STOLEN_SOUL,
    EVENT_MORTAL_STRIKE
};

enum Creatures
{
    NPC_STOLEN_SOUL             = 18441,
    NPC_DORE                    = 19412
};

enum Misc
{
    DATA_CLASS                  = 1
};

class npc_stolen_soul : public CreatureScript
{
    public:
        npc_stolen_soul() : CreatureScript("npc_stolen_soul") { }

        struct npc_stolen_soulAI : public ScriptedAI
        {
            npc_stolen_soulAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                myClass = CLASS_NONE;
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_CLASS_SPELL, Seconds(1));
            }

            void SetData(uint32 data, uint32 value)
            {
                if (data == DATA_CLASS)
                    myClass = value;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_CLASS_SPELL)
                {
                    switch (myClass)
                    {
                        case CLASS_WARRIOR:
                            DoCastVictim(SPELL_MORTAL_STRIKE);
                            events.Repeat(Seconds(6));
                            break;
                        case CLASS_PALADIN:
                            DoCastVictim(SPELL_HAMMER_OF_JUSTICE);
                            events.Repeat(Seconds(6));
                            break;
                        case CLASS_HUNTER:
                            DoCastVictim(SPELL_FREEZING_TRAP);
                            events.Repeat(Seconds(20));
                            break;
                        case CLASS_ROGUE:
                            DoCastVictim(SPELL_HEMORRHAGE);
                            events.Repeat(Seconds(10));
                            break;
                        case CLASS_PRIEST:
                            DoCastVictim(SPELL_MIND_FLAY);
                            events.Repeat(Seconds(5));
                            break;
                        case CLASS_SHAMAN:
                            DoCastVictim(SPELL_FROSTSHOCK);
                            events.Repeat(Seconds(8));
                            break;
                        case CLASS_MAGE:
                            DoCastVictim(SPELL_FIREBALL);
                            events.Repeat(Seconds(5));
                            break;
                        case CLASS_WARLOCK:
                            DoCastVictim(SPELL_CURSE_OF_AGONY);
                            events.Repeat(Seconds(20));
                            break;
                        case CLASS_DRUID:
                            DoCastVictim(SPELL_MOONFIRE);
                            events.Repeat(Seconds(10));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            uint32 myClass;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAuchenaiCryptsAI<npc_stolen_soulAI>(creature);
        }
};

class boss_exarch_maladaar : public CreatureScript
{
    public:
        boss_exarch_maladaar() : CreatureScript("boss_exarch_maladaar") { }

        struct boss_exarch_maladaarAI : public BossAI
        {
            boss_exarch_maladaarAI(Creature* creature) : BossAI(creature, DATA_EXARCH_MALADAAR)
            {
                hasTaunted = false;
            }

            void Reset() override
            {
                BossAI::Reset();
                soulmodel = 0;
                soulholder.Clear();
                avatarSummoned = false;
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (!hasTaunted && me->IsWithinDistInMap(who, 150.0f))
                {
                    Talk(SAY_INTRO);
                    hasTaunted = true;
                }

                BossAI::MoveInLineOfSight(who);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_FEAR, randtime(Seconds(15), Seconds(20)));
                events.ScheduleEvent(EVENT_RIBBON_OF_SOULS, Seconds(5));
                events.ScheduleEvent(EVENT_STOLEN_SOUL, randtime(Seconds(25), Seconds(35)));
            }

            void JustSummoned(Creature* summoned) override
            {
                if (summoned->GetEntry() == NPC_STOLEN_SOUL)
                {
                    //SPELL_STOLEN_SOUL_VISUAL has shapeshift effect, but not implemented feature in Trinity for this spell.
                    summoned->CastSpell(summoned, SPELL_STOLEN_SOUL_VISUAL);
                    summoned->SetDisplayId(soulmodel);
                    summoned->SetFaction(me->GetFaction());

                    if (Unit* target = ObjectAccessor::GetUnit(*me, soulholder))
                    {
                        summoned->AI()->SetData(DATA_CLASS, target->getClass());
                        summoned->AI()->AttackStart(target);
                    }
                }
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                if (urand(0, 2))
                    return;

                Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                //When Exarch Maladar is defeated D'ore appear.
                me->SummonCreature(NPC_DORE, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 600000);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (!avatarSummoned && me->HealthBelowPctDamaged(25, damage))
                {
                    if (me->IsNonMeleeSpellCast(false))
                        me->InterruptNonMeleeSpells(true);

                    Talk(SAY_SUMMON);
                    DoCastSelf(SPELL_SUMMON_AVATAR);
                    avatarSummoned = true;
                }
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
                        case EVENT_FEAR:
                            DoCastAOE(SPELL_SOUL_SCREAM);
                            events.Repeat(randtime(Seconds(15), Seconds(30)));
                            break;
                        case EVENT_RIBBON_OF_SOULS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_RIBBON_OF_SOULS);
                            events.Repeat(randtime(Seconds(5), Seconds(5 + (urand(0, 20)))));
                            break;
                        case EVENT_STOLEN_SOUL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                if (me->IsNonMeleeSpellCast(false))
                                    me->InterruptNonMeleeSpells(true);
                                Talk(SAY_ROAR);
                                soulmodel = target->GetDisplayId();
                                soulholder = target->GetGUID();
                                DoCast(target, SPELL_STOLEN_SOUL);
                                me->SummonCreature(NPC_STOLEN_SOUL, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                                events.Repeat(randtime(Seconds(20), Seconds(30)));
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            uint32 soulmodel;
            ObjectGuid soulholder;
            bool hasTaunted;
            bool avatarSummoned;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAuchenaiCryptsAI<boss_exarch_maladaarAI>(creature);
        }
};

class npc_avatar_of_martyred : public CreatureScript
{
    public:
        npc_avatar_of_martyred() : CreatureScript("npc_avatar_of_martyred") { }

        struct npc_avatar_of_martyredAI : public ScriptedAI
        {
            npc_avatar_of_martyredAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_MORTAL_STRIKE, Seconds(10));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_MORTAL_STRIKE)
                {
                    DoCastVictim(SPELL_AV_MORTAL_STRIKE);
                    events.Repeat(randtime(Seconds(10), Seconds(30)));
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAuchenaiCryptsAI<npc_avatar_of_martyredAI>(creature);
        }
};

void AddSC_boss_exarch_maladaar()
{
    new boss_exarch_maladaar();
    new npc_avatar_of_martyred();
    new npc_stolen_soul();
}
