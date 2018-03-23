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
SDName: Boss_Magmadar
SD%Complete: 75
SDComment: Conflag on ground nyi
SDCategory: Molten Core
EndScriptData */

#include "ScriptMgr.h"
#include "molten_core.h"
#include "ObjectMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"

enum Texts
{
    EMOTE_FRENZY        = 0
};

enum Spells
{
    SPELL_FRENZY        = 19451,
    SPELL_MAGMA_SPIT    = 19449,
    SPELL_PANIC         = 19408,
    SPELL_LAVA_BOMB_DMG = 19428,
    SPELL_LAVA_BOMB     = 19411,
    SPELL_SUMMON_LAVA_BOMB = 20494
};

enum Events
{
    EVENT_FRENZY        = 1,
    EVENT_PANIC,
    EVENT_LAVA_BOMB
};

class boss_magmadar : public CreatureScript
{
    public:
        boss_magmadar() : CreatureScript("boss_magmadar") { }

        struct boss_magmadarAI : public BossAI
        {
            boss_magmadarAI(Creature* creature) : BossAI(creature, BOSS_MAGMADAR) { }

            void Reset() override
            {
                BossAI::Reset();
                DoCastSelf(SPELL_MAGMA_SPIT, true);
            }

            void JustEngagedWith(Unit* victim) override
            {
                BossAI::JustEngagedWith(victim);
                events.ScheduleEvent(EVENT_FRENZY, Seconds(30));
                events.ScheduleEvent(EVENT_PANIC, Seconds(20));
                events.ScheduleEvent(EVENT_LAVA_BOMB, Seconds(12));
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
                        case EVENT_FRENZY:
                            Talk(EMOTE_FRENZY);
                            DoCast(me, SPELL_FRENZY);
                            events.ScheduleEvent(EVENT_FRENZY, Seconds(15));
                            break;
                        case EVENT_PANIC:
                            DoCastVictim(SPELL_PANIC);
                            events.ScheduleEvent(EVENT_PANIC, Seconds(35));
                            break;
                        case EVENT_LAVA_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, true, -SPELL_LAVA_BOMB_DMG))
                                DoCast(target, SPELL_LAVA_BOMB);
                            events.ScheduleEvent(EVENT_LAVA_BOMB, Seconds(12));
                            break;
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetMoltenCoreAI<boss_magmadarAI>(creature);
        }
};

// 19411 Lava Bomb
class spell_magmadar_lava_bomb : public SpellScript
{
    PrepareSpellScript(spell_magmadar_lava_bomb);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_LAVA_BOMB });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            target->CastSpell(target, SPELL_SUMMON_LAVA_BOMB, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_magmadar_lava_bomb::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_boss_magmadar()
{
    new boss_magmadar();

    RegisterSpellScript(spell_magmadar_lava_bomb);
}
