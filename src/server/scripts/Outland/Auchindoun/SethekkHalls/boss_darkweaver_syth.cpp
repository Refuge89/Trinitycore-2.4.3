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
SDName: Boss_Darkweaver_Syth
SD%Complete: 85
SDComment: Shock spells/times need more work. Heroic partly implemented.
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "sethekk_halls.h"

enum Says
{
    SAY_SUMMON                  = 0,
    SAY_AGGRO                   = 1,
    SAY_SLAY                    = 2,
    SAY_DEATH                   = 3
};

enum Spells
{
    SPELL_FROST_SHOCK           = 21401, //37865
    SPELL_FLAME_SHOCK           = 34354,
    SPELL_SHADOW_SHOCK          = 30138,
    SPELL_ARCANE_SHOCK          = 37132,

    SPELL_CHAIN_LIGHTNING       = 15659, //15305

    SPELL_SUMMON_SYTH_FIRE      = 33537,                   // Spawns 19203
    SPELL_SUMMON_SYTH_ARCANE    = 33538,                   // Spawns 19205
    SPELL_SUMMON_SYTH_FROST     = 33539,                   // Spawns 19204
    SPELL_SUMMON_SYTH_SHADOW    = 33540,                   // Spawns 19206

    SPELL_FLAME_BUFFET          = 33526,
    SPELL_ARCANE_BUFFET         = 33527,
    SPELL_FROST_BUFFET          = 33528,
    SPELL_SHADOW_BUFFET         = 33529
};

enum Events
{
    EVENT_FLAME_SHOCK           = 1,
    EVENT_ARCANE_SHOCK,
    EVENT_FROST_SHOCK,
    EVENT_SHADOW_SHOCK,
    EVENT_CHAIN_LIGHTNING,
    EVENT_ELEMENTAL_SHOCK,
    EVENT_ELEMENTAL_BUFFET
};

class boss_darkweaver_syth : public CreatureScript
{
    public:
        boss_darkweaver_syth() : CreatureScript("boss_darkweaver_syth") { }

        struct boss_darkweaver_sythAI : public BossAI
        {
            boss_darkweaver_sythAI(Creature* creature) : BossAI(creature, DATA_DARKWEAVER_SYTH) { }

            void Reset() override
            {
                BossAI::Reset();
                summon90 = false;
                summon50 = false;
                summon10 = false;
            }

            void JustEngagedWith(Unit* who) override
            {
                BossAI::JustEngagedWith(who);
                events.ScheduleEvent(EVENT_FLAME_SHOCK, Seconds(2));
                events.ScheduleEvent(EVENT_ARCANE_SHOCK, Seconds(4));
                events.ScheduleEvent(EVENT_FROST_SHOCK, Seconds(6));
                events.ScheduleEvent(EVENT_SHADOW_SHOCK, Seconds(8));
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, Seconds(15));

                Talk(SAY_AGGRO);
            }

            void JustDied(Unit* killer) override
            {
                BossAI::JustDied(killer);
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustSummoned(Creature* summoned) override
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    summoned->AI()->AttackStart(target);

                summons.Summon(summoned);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (me->HealthBelowPctDamaged(90, damage) && !summon90)
                {
                    SythSummoning();
                    summon90 = true;
                }

                if (me->HealthBelowPctDamaged(50, damage) && !summon50)
                {
                    SythSummoning();
                    summon50 = true;
                }

                if (me->HealthBelowPctDamaged(10, damage) && !summon10)
                {
                    SythSummoning();
                    summon10 = true;
                }
            }

            void SythSummoning()
            {
                Talk(SAY_SUMMON);

                if (me->IsNonMeleeSpellCast(false))
                    me->InterruptNonMeleeSpells(false);

                DoCastSelf(SPELL_SUMMON_SYTH_ARCANE, true);   //front
                DoCastSelf(SPELL_SUMMON_SYTH_FIRE, true);     //back
                DoCastSelf(SPELL_SUMMON_SYTH_FROST, true);    //left
                DoCastSelf(SPELL_SUMMON_SYTH_SHADOW, true);   //right
            }

            void ExecuteEvent(uint32 eventId) override
            {
                switch (eventId)
                {
                    case EVENT_FLAME_SHOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_FLAME_SHOCK);
                        events.Repeat(randtime(Seconds(10), Seconds(15)));
                        break;
                    case EVENT_ARCANE_SHOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_ARCANE_SHOCK);
                        events.Repeat(randtime(Seconds(10), Seconds(15)));
                        break;
                    case EVENT_FROST_SHOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_FROST_SHOCK);
                        events.Repeat(randtime(Seconds(10), Seconds(15)));
                        break;
                    case EVENT_SHADOW_SHOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_SHADOW_SHOCK);
                        events.Repeat(randtime(Seconds(10), Seconds(15)));
                        break;
                    case EVENT_CHAIN_LIGHTNING:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_CHAIN_LIGHTNING);
                        events.Repeat(Seconds(25));
                        break;
                    default:
                        break;
                }
            }

            private:
                bool summon90;
                bool summon50;
                bool summon10;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<boss_darkweaver_sythAI>(creature);
        }
};

/* ELEMENTALS */
struct npc_syth_elementalAI : public ScriptedAI
{
    npc_syth_elementalAI(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        events.Reset();
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, spellMask, true);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_ELEMENTAL_SHOCK, Milliseconds(2500));
        events.ScheduleEvent(EVENT_ELEMENTAL_BUFFET, Seconds(5));
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
                case EVENT_ELEMENTAL_SHOCK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, shockSpell);
                    events.Repeat(Seconds(5));
                    break;
                case EVENT_ELEMENTAL_BUFFET:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, buffetSpell);
                    events.Repeat(Seconds(5));
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

    public:
        EventMap events;
        uint32 shockSpell;
        uint32 buffetSpell;
        uint32 spellMask;
};

class npc_syth_fire : public CreatureScript
{
    public:
        npc_syth_fire() : CreatureScript("npc_syth_fire") { }

        struct npc_syth_fireAI : public npc_syth_elementalAI
        {
            npc_syth_fireAI(Creature* creature) : npc_syth_elementalAI(creature)
            {
                shockSpell = SPELL_FLAME_SHOCK;
                buffetSpell = SPELL_FLAME_BUFFET;
                spellMask = SPELL_SCHOOL_MASK_FIRE;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<npc_syth_fireAI>(creature);
        }
};

class npc_syth_arcane : public CreatureScript
{
    public:
        npc_syth_arcane() : CreatureScript("npc_syth_arcane") { }

        struct npc_syth_arcaneAI : public npc_syth_elementalAI
        {
            npc_syth_arcaneAI(Creature* creature) : npc_syth_elementalAI(creature)
            {
                shockSpell = SPELL_ARCANE_SHOCK;
                buffetSpell = SPELL_ARCANE_BUFFET;
                spellMask = SPELL_SCHOOL_MASK_ARCANE;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<npc_syth_arcaneAI>(creature);
        }
};

class npc_syth_frost : public CreatureScript
{
    public:
        npc_syth_frost() : CreatureScript("npc_syth_frost") { }

        struct npc_syth_frostAI : public npc_syth_elementalAI
        {
            npc_syth_frostAI(Creature* creature) : npc_syth_elementalAI(creature)
            {
                shockSpell = SPELL_FROST_SHOCK;
                buffetSpell = SPELL_FROST_BUFFET;
                spellMask = SPELL_SCHOOL_MASK_FROST;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<npc_syth_frostAI>(creature);
        }
};

class npc_syth_shadow : public CreatureScript
{
    public:
        npc_syth_shadow() : CreatureScript("npc_syth_shadow") { }

        struct npc_syth_shadowAI : public npc_syth_elementalAI
        {
            npc_syth_shadowAI(Creature* creature) : npc_syth_elementalAI(creature)
            {
                shockSpell = SPELL_SHADOW_SHOCK;
                buffetSpell = SPELL_SHADOW_BUFFET;
                spellMask = SPELL_SCHOOL_MASK_SHADOW;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<npc_syth_shadowAI>(creature);
        }
};

void AddSC_boss_darkweaver_syth()
{
    new boss_darkweaver_syth();
    new npc_syth_fire();
    new npc_syth_arcane();
    new npc_syth_frost();
    new npc_syth_shadow();
}
