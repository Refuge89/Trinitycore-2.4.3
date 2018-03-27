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
SDName: Boss_Ragnaros
SD%Complete: 95
SDComment: some spells doesnt work correctly
SDCategory: Molten Core
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "molten_core.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "TemporarySummon.h"

enum Texts
{
    SAY_SUMMON_MAJ              = 0,
    SAY_ARRIVAL1_RAG            = 1,
    SAY_ARRIVAL2_MAJ            = 2,
    SAY_ARRIVAL3_RAG            = 3,
    SAY_ARRIVAL5_RAG            = 4,
    SAY_REINFORCEMENTS1         = 5,
    SAY_REINFORCEMENTS2         = 6,
    SAY_HAND                    = 7,
    SAY_WRATH                   = 8,
    SAY_KILL                    = 9,
    SAY_MAGMABURST              = 10
};

enum Spells
{
    SPELL_HAND_OF_RAGNAROS      = 19780,
    SPELL_WRATH_OF_RAGNAROS     = 20566,
    SPELL_LAVA_BURST            = 21158,
    SPELL_MAGMA_BLAST           = 20565,                   // Ranged attack
    SPELL_SONS_OF_FLAME_DUMMY   = 21108,                   // Server side effect
    SPELL_RAGSUBMERGE           = 21107,                   // Stealth aura
    SPELL_RAGEMERGE             = 20568,
    SPELL_MELT_WEAPON           = 21387,
    SPELL_ELEMENTAL_FIRE        = 20564,
    SPELL_ERRUPTION             = 17731,
    SPELL_LAVA_SHIELD           = 21857,
    SPELL_MIGHT_OF_RAGNAROS     = 21154,
    SPELL_INTENSE_HEAT          = 21155
};

enum Events
{
    EVENT_ERUPTION              = 1,
    EVENT_WRATH_OF_RAGNAROS,
    EVENT_HAND_OF_RAGNAROS,
    EVENT_LAVA_BURST,
    EVENT_ELEMENTAL_FIRE,
    EVENT_SUBMERGE,

    EVENT_INTRO_1,
    EVENT_INTRO_2,
    EVENT_INTRO_3,
    EVENT_INTRO_4,
    EVENT_INTRO_5
};

class RagnarosMeleeTargetSelector
{
    public:
        explicit RagnarosMeleeTargetSelector(Creature* me) : _me(me) { };

        bool operator()(WorldObject* object) const
        {
            if (object->GetTypeId() == TYPEID_PLAYER && _me->CanCreatureAttack(object->ToUnit()))
                return _me->IsWithinMeleeRange(object->ToUnit());

            return false;
        }
    private:
        Creature const* _me;
};

class boss_ragnaros : public CreatureScript
{
    public:
        boss_ragnaros() : CreatureScript("boss_ragnaros") { }

        struct boss_ragnarosAI : public BossAI
        {
            boss_ragnarosAI(Creature* creature) : BossAI(creature, BOSS_RAGNAROS)
            {
                introState = 0;
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                SetCombatMovement(false);
            }

            void Reset() override
            {
                BossAI::Reset();
                emergeTimer = 90000;
                hasYelledMagmaBurst = false;
                hasSubmergedOnce = false;
                isBanished = false;
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
            }

            void JustDied(Unit* killer) override
            {
                BossAI::JustDied(killer);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }

            void JustEngagedWith(Unit* victim) override
            {
                BossAI::JustEngagedWith(victim);
                events.ScheduleEvent(EVENT_ERUPTION, Seconds(15));
                events.ScheduleEvent(EVENT_WRATH_OF_RAGNAROS, Seconds(30));
                events.ScheduleEvent(EVENT_HAND_OF_RAGNAROS, Seconds(25));
                events.ScheduleEvent(EVENT_LAVA_BURST, Seconds(10));
                events.ScheduleEvent(EVENT_ELEMENTAL_FIRE, Seconds(3));
                events.ScheduleEvent(EVENT_SUBMERGE, Minutes(3));
                if (!me->HasAura(SPELL_MELT_WEAPON))
                    DoCastSelf(SPELL_MELT_WEAPON, true);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                if (urand(0, 99) < 25)
                    Talk(SAY_KILL);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                if (me->IsInCombat() && summon->GetEntry() == NPC_SON_OF_FLAME)
                    DoZoneInCombat(summon);
                if (summon->GetEntry() == NPC_FLAME_OF_RAGNAROS)
                    summon->CastSpell(summon, SPELL_INTENSE_HEAT, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (introState != 2)
                {
                    if (!introState)
                    {
                        me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                        events.ScheduleEvent(EVENT_INTRO_1, Seconds(4));
                        events.ScheduleEvent(EVENT_INTRO_2, Seconds(23));
                        events.ScheduleEvent(EVENT_INTRO_3, Seconds(42));
                        events.ScheduleEvent(EVENT_INTRO_4, Seconds(43));
                        events.ScheduleEvent(EVENT_INTRO_5, Seconds(53));
                        introState = 1;
                    }

                    events.Update(diff);

                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_INTRO_1:
                                Talk(SAY_ARRIVAL1_RAG);
                                break;
                            case EVENT_INTRO_2:
                                Talk(SAY_ARRIVAL3_RAG);
                                break;
                            case EVENT_INTRO_3:
                                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                                break;
                            case EVENT_INTRO_4:
                                Talk(SAY_ARRIVAL5_RAG);
                                if (Creature* executus = ObjectAccessor::GetCreature(*me, instance->GetGuidData(BOSS_MAJORDOMO_EXECUTUS)))
                                    Unit::Kill(me, executus);
                                break;
                            case EVENT_INTRO_5:
                                me->SetReactState(REACT_AGGRESSIVE);
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                me->SetImmuneToPC(false);
                                introState = 2;
                                break;
                            default:
                                break;
                        }
                    }
                }
                else
                {
                    if (isBanished && ((emergeTimer <= diff) || (instance->GetData(DATA_RAGNAROS_ADDS)) >= 8))
                    {
                        //Become unbanished again
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetFaction(FACTION_MONSTER);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            AttackStart(target);
                        instance->SetData(DATA_RAGNAROS_ADDS, 0);

                        //DoCast(me, SPELL_RAGEMERGE); //"phase spells" didnt worked correctly so Ive commented them and wrote solution witch doesnt need core support
                        isBanished = false;
                    }
                    else if (isBanished)
                    {
                        emergeTimer -= diff;
                        //Do nothing while banished
                        return;
                    }

                    //Return since we have no target
                    if (!UpdateVictim())
                        return;

                    events.Update(diff);

                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_ERUPTION:
                                DoCastAOE(SPELL_ERRUPTION);
                                events.Repeat(randtime(Seconds(20), Seconds(45)));
                                break;
                            case EVENT_WRATH_OF_RAGNAROS:
                                DoCastAOE(SPELL_WRATH_OF_RAGNAROS);
                                if (urand(0, 1))
                                    Talk(SAY_WRATH);
                                events.Repeat(Seconds(25));
                                break;
                            case EVENT_HAND_OF_RAGNAROS:
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [&](Unit* u) { return u && u->GetTypeId() == TYPEID_PLAYER && u->GetPowerType() == POWER_MANA; }))
                                    DoCast(target, SPELL_HAND_OF_RAGNAROS);
                                if (urand(0, 1))
                                    Talk(SAY_HAND);
                                events.Repeat(Seconds(20));
                                break;
                            case EVENT_LAVA_BURST:
                                DoCastAOE(SPELL_LAVA_BURST);
                                events.Repeat(Seconds(10));
                                break;
                            case EVENT_ELEMENTAL_FIRE:
                                if (me->GetVictim() && me->IsWithinMeleeRange(me->GetVictim()))
                                    DoCastVictim(SPELL_ELEMENTAL_FIRE);
                                else if (Unit* target = SelectTarget(SELECT_TARGET_MAXTHREAT, 0, RagnarosMeleeTargetSelector(me)))
                                    DoCast(target, SPELL_ELEMENTAL_FIRE);
                                events.Repeat(randtime(Seconds(10), Seconds(14)));
                                break;
                            case EVENT_SUBMERGE:
                            {
                                if (!isBanished)
                                {
                                    //Creature spawning and ragnaros becomming unattackable
                                    //is not very well supported in the core //no it really isnt
                                    //so added normaly spawning and banish workaround and attack again after 90 secs.
                                    me->AttackStop();
                                    ResetThreatList();
                                    me->SetReactState(REACT_PASSIVE);
                                    me->InterruptNonMeleeSpells(false);
                                    //Root self
                                    //DoCast(me, 23973);
                                    me->SetFaction(FACTION_FRIENDLY);
                                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_SUBMERGED);
                                    me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
                                    instance->SetData(DATA_RAGNAROS_ADDS, 0);
                                    if (!hasSubmergedOnce)
                                    {
                                        Talk(SAY_REINFORCEMENTS1);
                                        // summon 8 elementals
                                        for (uint8 i = 0; i < 8; ++i)
                                        {
                                            Position summonPos = me->GetRandomPoint(me->GetPosition(), 30.0f);
                                            if (Creature* summoned = me->SummonCreature(NPC_SON_OF_FLAME, summonPos.GetPositionX(), summonPos.GetPositionY(), summonPos.GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 900000))
                                            {
                                                summoned->CastSpell(summoned, SPELL_LAVA_SHIELD, true);
                                                summoned->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
                                            }
                                        }
                                        hasSubmergedOnce = true;
                                        isBanished = true;
                                        //DoCast(me, SPELL_RAGSUBMERGE);
                                        emergeTimer = 90000;
                                    }
                                    else
                                    {
                                        Talk(SAY_REINFORCEMENTS2);
                                        for (uint8 i = 0; i < 8; ++i)
                                        {
                                            Position summonPos = me->GetRandomPoint(me->GetPosition(), 30.0f);
                                            if (Creature* summoned = me->SummonCreature(NPC_SON_OF_FLAME, summonPos.GetPositionX(), summonPos.GetPositionY(), summonPos.GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 900000))
                                            {
                                                summoned->CastSpell(summoned, SPELL_LAVA_SHIELD, true);
                                                summoned->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
                                            }
                                        }
                                        isBanished = true;
                                        //DoCast(me, SPELL_RAGSUBMERGE);
                                        emergeTimer = 90000;
                                    }
                                }
                                events.Repeat(Minutes(3));
                                break;
                            }
                            default:
                                break;
                        }
                    }

                    DoMeleeAttackIfReady();
                }
            }

            void DoMeleeAttackIfReady()
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->IsWithinMeleeRange(me->GetVictim()))
                {
                    if (me->isAttackReady())
                    {
                        me->AttackerStateUpdate(me->GetVictim());
                        me->resetAttackTimer();
                    }
                }
                else if (me->isAttackReady())
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_MAXTHREAT, 0, RagnarosMeleeTargetSelector(me)))
                    {
                        me->AttackerStateUpdate(target);
                        me->resetAttackTimer();
                    }
                    else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    {
                        DoCast(target, SPELL_MAGMA_BLAST);
                        if (!hasYelledMagmaBurst)
                        {
                            Talk(SAY_MAGMABURST);
                            hasYelledMagmaBurst = true;
                        }
                        me->setAttackTimer(BASE_ATTACK, 2000);
                    }
                }
            }

        private:
            uint32 emergeTimer;
            uint8 introState;
            bool hasYelledMagmaBurst;
            bool hasSubmergedOnce;
            bool isBanished;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetMoltenCoreAI<boss_ragnarosAI>(creature);
        }
};

class npc_son_of_flame : public CreatureScript
{
    public:
        npc_son_of_flame() : CreatureScript("npc_SonOfFlame") { }

        struct npc_son_of_flameAI : public ScriptedAI //didnt work correctly in EAI for me...
        {
            npc_son_of_flameAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = me->GetInstanceScript();
            }

            void JustDied(Unit* /*killer*/) override
            {
                instance->SetData(DATA_RAGNAROS_ADDS, 1);
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetMoltenCoreAI<npc_son_of_flameAI>(creature);
        }
};

void AddSC_boss_ragnaros()
{
    new boss_ragnaros();
    new npc_son_of_flame();
}
