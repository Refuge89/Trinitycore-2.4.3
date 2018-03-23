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

/*
 * Scripts for spells with SPELLFAMILY_GENERIC spells used for quests.
 * Ordered alphabetically using questId and scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_q#questID_".
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CreatureAIImpl.h"
#include "CreatureTextMgr.h"
#include "GridNotifiersImpl.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

class spell_generic_quest_update_entry_SpellScript : public SpellScript
{
    PrepareSpellScript(spell_generic_quest_update_entry_SpellScript);
    private:
        uint16 _spellEffect;
        uint8 _effIndex;
        uint32 _originalEntry;
        uint32 _newEntry;
        bool _shouldAttack;
        uint32 _despawnTime;

    public:
        spell_generic_quest_update_entry_SpellScript(uint16 spellEffect, uint8 effIndex, uint32 originalEntry, uint32 newEntry, bool shouldAttack, uint32 despawnTime = 0) :
            SpellScript(), _spellEffect(spellEffect), _effIndex(effIndex), _originalEntry(originalEntry),
            _newEntry(newEntry), _shouldAttack(shouldAttack), _despawnTime(despawnTime) { }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Creature* creatureTarget = GetHitCreature())
                if (!creatureTarget->IsPet() && creatureTarget->GetEntry() == _originalEntry)
                {
                    creatureTarget->UpdateEntry(_newEntry);
                    if (_shouldAttack && creatureTarget->IsAIEnabled)
                        creatureTarget->AI()->AttackStart(GetCaster());

                    if (_despawnTime)
                        creatureTarget->DespawnOrUnsummon(_despawnTime);
                }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_generic_quest_update_entry_SpellScript::HandleDummy, _effIndex, _spellEffect);
        }
};

// http://www.wowhead.com/quest=55 Morbent Fel
// 8913 Sacred Cleansing
enum Quest55Data
{
    NPC_MORBENT             = 1200,
    NPC_WEAKENED_MORBENT    = 24782,
};

class spell_q55_sacred_cleansing : public SpellScriptLoader
{
    public:
        spell_q55_sacred_cleansing() : SpellScriptLoader("spell_q55_sacred_cleansing") { }

        SpellScript* GetSpellScript() const override
        {
            return new spell_generic_quest_update_entry_SpellScript(SPELL_EFFECT_DUMMY, EFFECT_1, NPC_MORBENT, NPC_WEAKENED_MORBENT, true);
        }
};

// 9712 - Thaumaturgy Channel
enum ThaumaturgyChannel
{
    SPELL_THAUMATURGY_CHANNEL = 21029
};

class spell_q2203_thaumaturgy_channel : public SpellScriptLoader
{
    public:
        spell_q2203_thaumaturgy_channel() : SpellScriptLoader("spell_q2203_thaumaturgy_channel") { }

        class spell_q2203_thaumaturgy_channel_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_q2203_thaumaturgy_channel_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_THAUMATURGY_CHANNEL });
            }

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, SPELL_THAUMATURGY_CHANNEL, false);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_q2203_thaumaturgy_channel_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_q2203_thaumaturgy_channel_AuraScript();
        }
};

// http://www.wowhead.com/quest=5206 Marauders of Darrowshire
// 17271 Test Fetid Skull
enum Quest5206Data
{
    SPELL_CREATE_RESONATING_SKULL = 17269,
    SPELL_CREATE_BONE_DUST = 17270
};

class spell_q5206_test_fetid_skull : public SpellScriptLoader
{
    public:
        spell_q5206_test_fetid_skull() : SpellScriptLoader("spell_q5206_test_fetid_skull") { }

        class spell_q5206_test_fetid_skull_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q5206_test_fetid_skull_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spellEntry*/) override
            {
                return ValidateSpellInfo({ SPELL_CREATE_RESONATING_SKULL, SPELL_CREATE_BONE_DUST });
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = roll_chance_i(50) ? SPELL_CREATE_RESONATING_SKULL : SPELL_CREATE_BONE_DUST;
                caster->CastSpell(caster, spellId, true);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_q5206_test_fetid_skull_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q5206_test_fetid_skull_SpellScript();
        }
};

// http://www.wowhead.com/quest=6124 Curing the Sick (A)
// http://www.wowhead.com/quest=6129 Curing the Sick (H)
// 19512 Apply Salve
enum Quests6124_6129Data
{
    NPC_SICKLY_GAZELLE  = 12296,
    NPC_CURED_GAZELLE   = 12297,
    NPC_SICKLY_DEER     = 12298,
    NPC_CURED_DEER      = 12299,
    DESPAWN_TIME        = 30000
};

class spell_q6124_6129_apply_salve : public SpellScriptLoader
{
    public:
        spell_q6124_6129_apply_salve() : SpellScriptLoader("spell_q6124_6129_apply_salve") { }

        class spell_q6124_6129_apply_salve_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q6124_6129_apply_salve_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (GetCastItem())
                    if (Creature* creatureTarget = GetHitCreature())
                    {
                        uint32 newEntry = 0;
                        switch (caster->GetTeam())
                        {
                            case HORDE:
                                if (creatureTarget->GetEntry() == NPC_SICKLY_GAZELLE)
                                    newEntry = NPC_CURED_GAZELLE;
                                break;
                            case ALLIANCE:
                                if (creatureTarget->GetEntry() == NPC_SICKLY_DEER)
                                    newEntry = NPC_CURED_DEER;
                                break;
                        }
                        if (newEntry)
                        {
                            creatureTarget->UpdateEntry(newEntry);
                            creatureTarget->DespawnOrUnsummon(DESPAWN_TIME);
                            caster->KilledMonsterCredit(newEntry);
                        }
                    }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_q6124_6129_apply_salve_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q6124_6129_apply_salve_SpellScript();
        }
};

// http://www.wowhead.com/quest=10255 Testing the Antidote
// 34665 Administer Antidote
enum Quest10255Data
{
    NPC_HELBOAR     = 16880,
    NPC_DREADTUSK   = 16992,
};

class spell_q10255_administer_antidote : public SpellScriptLoader
{
    public:
        spell_q10255_administer_antidote() : SpellScriptLoader("spell_q10255_administer_antidote") { }

        SpellScript* GetSpellScript() const override
        {
            return new spell_generic_quest_update_entry_SpellScript(SPELL_EFFECT_DUMMY, EFFECT_0, NPC_HELBOAR, NPC_DREADTUSK, true);
        }
};

// http://www.wowhead.com/quest=11515 Blood for Blood
// 44936 Quest - Fel Siphon Dummy
enum Quest11515Data
{
    NPC_FELBLOOD_INITIATE   = 24918,
    NPC_EMACIATED_FELBLOOD  = 24955
};

class spell_q11515_fel_siphon_dummy : public SpellScriptLoader
{
    public:
        spell_q11515_fel_siphon_dummy() : SpellScriptLoader("spell_q11515_fel_siphon_dummy") { }

        SpellScript* GetSpellScript() const override
        {
            return new spell_generic_quest_update_entry_SpellScript(SPELL_EFFECT_DUMMY, EFFECT_0, NPC_FELBLOOD_INITIATE, NPC_EMACIATED_FELBLOOD, true);
        }
};

enum Whoarethey
{
    SPELL_MALE_DISGUISE = 38080,
    SPELL_FEMALE_DISGUISE = 38081,
    SPELL_GENERIC_DISGUISE = 32756
};

class spell_q10041_q10040_who_are_they : public SpellScriptLoader
{
    public:
        spell_q10041_q10040_who_are_they() : SpellScriptLoader("spell_q10041_q10040_who_are_they") { }

        class spell_q10041_q10040_who_are_they_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q10041_q10040_who_are_they_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/) override
            {
                return ValidateSpellInfo({ SPELL_MALE_DISGUISE, SPELL_FEMALE_DISGUISE, SPELL_GENERIC_DISGUISE });
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Player* target = GetHitPlayer())
                {
                    target->CastSpell(target, target->getGender() == GENDER_MALE ? SPELL_MALE_DISGUISE : SPELL_FEMALE_DISGUISE, true);
                    target->CastSpell(target, SPELL_GENERIC_DISGUISE, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_q10041_q10040_who_are_they_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q10041_q10040_who_are_they_SpellScript();
        }
};

enum symboloflife
{
    SPELL_PERMANENT_FEIGN_DEATH = 29266,
};

// 8593 Symbol of life dummy
class spell_symbol_of_life_dummy : public SpellScriptLoader
{
    public:
        spell_symbol_of_life_dummy() : SpellScriptLoader("spell_symbol_of_life_dummy") { }

        class spell_symbol_of_life_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_symbol_of_life_dummy_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Creature* target = GetHitCreature())
                {
                    if (target->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                    {
                        target->RemoveAurasDueToSpell(SPELL_PERMANENT_FEIGN_DEATH);
                        target->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                        target->SetUInt32Value(UNIT_FIELD_FLAGS_2, 0);
                        target->SetHealth(target->GetMaxHealth() / 2);
                        target->SetPower(POWER_MANA, uint32(target->GetMaxPower(POWER_MANA) * 0.75f));
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_symbol_of_life_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_symbol_of_life_dummy_SpellScript();
        }
};

enum StoppingTheSpread
{
    NPC_VILLAGER_KILL_CREDIT                     = 18240,
    SPELL_FLAMES                                 = 39199
};

class spell_q9874_liquid_fire : public SpellScriptLoader
{
    public:
        spell_q9874_liquid_fire() : SpellScriptLoader("spell_q9874_liquid_fire")
        {
        }

        class spell_q9874_liquid_fire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q9874_liquid_fire_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Creature* target = GetHitCreature())
                    if (!target->HasAura(SPELL_FLAMES))
                    {
                        caster->KilledMonsterCredit(NPC_VILLAGER_KILL_CREDIT);
                        target->CastSpell(target, SPELL_FLAMES, true);
                        target->DespawnOrUnsummon(60000);
                    }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_q9874_liquid_fire_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q9874_liquid_fire_SpellScript();
        }
};

// http://old01.wowhead.com/quest=9452 - Red Snapper - Very Tasty!
enum RedSnapperVeryTasty
{
    ITEM_RED_SNAPPER             = 23614,
    SPELL_CAST_NET               = 29866,
    SPELL_FISHED_UP_MURLOC       = 29869
};

class spell_q9452_cast_net: public SpellScriptLoader
{
    public:
        spell_q9452_cast_net() : SpellScriptLoader("spell_q9452_cast_net") { }

        class spell_q9452_cast_net_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q9452_cast_net_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (roll_chance_i(66))
                    caster->AddItem(ITEM_RED_SNAPPER, 1);
                else
                    caster->CastSpell(caster, SPELL_FISHED_UP_MURLOC, true);
            }

            void HandleActiveObject(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitGObj()->SetRespawnTime(roll_chance_i(50) ? 2 * MINUTE : 3 * MINUTE);
                GetHitGObj()->Use(GetCaster());
                GetHitGObj()->SetLootState(GO_JUST_DEACTIVATED);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_q9452_cast_net_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnEffectHitTarget += SpellEffectFn(spell_q9452_cast_net_SpellScript::HandleActiveObject, EFFECT_1, SPELL_EFFECT_ACTIVATE_OBJECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q9452_cast_net_SpellScript();
        }
};

enum PoundDrumSpells
{
    SPELL_SUMMON_DEEP_JORMUNGAR     = 66510,
    SPELL_STORMFORGED_MOLE_MACHINE  = 66492
};

class spell_q14076_14092_pound_drum : public SpellScriptLoader
{
    public:
        spell_q14076_14092_pound_drum() : SpellScriptLoader("spell_q14076_14092_pound_drum") { }

        class spell_q14076_14092_pound_drum_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q14076_14092_pound_drum_SpellScript);

            void HandleSummon()
            {
                Unit* caster = GetCaster();

                if (roll_chance_i(80))
                    caster->CastSpell(caster, SPELL_SUMMON_DEEP_JORMUNGAR, true);
                else
                    caster->CastSpell(caster, SPELL_STORMFORGED_MOLE_MACHINE, true);
            }

            void HandleActiveObject(SpellEffIndex /*effIndex*/)
            {
                GetHitGObj()->SetLootState(GO_JUST_DEACTIVATED);
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_q14076_14092_pound_drum_SpellScript::HandleSummon);
                OnEffectHitTarget += SpellEffectFn(spell_q14076_14092_pound_drum_SpellScript::HandleActiveObject, EFFECT_0, SPELL_EFFECT_ACTIVATE_OBJECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q14076_14092_pound_drum_SpellScript();
        }
};

// "Bombing Run" and "Bomb Them Again!"
enum Quest11010_11102_11023Data
{
    // Spell
    SPELL_FLAK_CANNON_TRIGGER = 40110,
    SPELL_CHOOSE_LOC          = 40056,
    SPELL_AGGRO_CHECK         = 40112,
    // NPCs
    NPC_FEL_CANNON2           = 23082
};

// 40113 Knockdown Fel Cannon: The Aggro Check Aura
class spell_q11010_q11102_q11023_aggro_check_aura : public SpellScriptLoader
{
    public:
        spell_q11010_q11102_q11023_aggro_check_aura() : SpellScriptLoader("spell_q11010_q11102_q11023_aggro_check_aura") { }

        class spell_q11010_q11102_q11023_aggro_check_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_q11010_q11102_q11023_aggro_check_aura_AuraScript);

            void HandleTriggerSpell(AuraEffect const* /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                    // On trigger proccing
                    target->CastSpell(target, SPELL_AGGRO_CHECK);
            }

            void Register() override
            {
               OnEffectPeriodic += AuraEffectPeriodicFn(spell_q11010_q11102_q11023_aggro_check_aura_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_q11010_q11102_q11023_aggro_check_aura_AuraScript();
        }
};

// 40112 Knockdown Fel Cannon: The Aggro Check
class spell_q11010_q11102_q11023_aggro_check : public SpellScriptLoader
{
    public:
        spell_q11010_q11102_q11023_aggro_check() : SpellScriptLoader("spell_q11010_q11102_q11023_aggro_check") { }

        class spell_q11010_q11102_q11023_aggro_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q11010_q11102_q11023_aggro_check_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* playerTarget = GetHitPlayer())
                    // Check if found player target is on fly mount or using flying form
                    if (playerTarget->HasAuraType(SPELL_AURA_FLY) || playerTarget->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                        playerTarget->CastSpell(playerTarget, SPELL_FLAK_CANNON_TRIGGER);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_q11010_q11102_q11023_aggro_check_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q11010_q11102_q11023_aggro_check_SpellScript();
        }
};

// 40119 Knockdown Fel Cannon: The Aggro Burst
class spell_q11010_q11102_q11023_aggro_burst : public SpellScriptLoader
{
    public:
        spell_q11010_q11102_q11023_aggro_burst() : SpellScriptLoader("spell_q11010_q11102_q11023_aggro_burst") { }

        class spell_q11010_q11102_q11023_aggro_burst_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_q11010_q11102_q11023_aggro_burst_AuraScript);

            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                    // On each tick cast Choose Loc to trigger summon
                    target->CastSpell(target, SPELL_CHOOSE_LOC);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_q11010_q11102_q11023_aggro_burst_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_q11010_q11102_q11023_aggro_burst_AuraScript();
        }
};

// 40056 Knockdown Fel Cannon: Choose Loc
class spell_q11010_q11102_q11023_choose_loc : public SpellScriptLoader
{
    public:
        spell_q11010_q11102_q11023_choose_loc() : SpellScriptLoader("spell_q11010_q11102_q11023_choose_loc") { }

        class spell_q11010_q11102_q11023_choose_loc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q11010_q11102_q11023_choose_loc_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                // Check for player that is in 65 y range
                std::list<Player*> playerList;
                Trinity::AnyPlayerInObjectRangeCheck checker(caster, 65.0f);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(caster, playerList, checker);
                Cell::VisitWorldObjects(caster, searcher, 65.0f);
                for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    // Check if found player target is on fly mount or using flying form
                    if ((*itr)->HasAuraType(SPELL_AURA_FLY) || (*itr)->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
                        // Summom Fel Cannon (bunny version) at found player
                        caster->SummonCreature(NPC_FEL_CANNON2, (*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ());
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_q11010_q11102_q11023_choose_loc_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q11010_q11102_q11023_choose_loc_SpellScript();
        }
};

// 39844 - Skyguard Blasting Charge
// 40160 - Throw Bomb
class spell_q11010_q11102_q11023_q11008_check_fly_mount : public SpellScriptLoader
{
    public:
        spell_q11010_q11102_q11023_q11008_check_fly_mount() : SpellScriptLoader("spell_q11010_q11102_q11023_q11008_check_fly_mount") { }

        class spell_q11010_q11102_q11023_q11008_check_fly_mount_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_q11010_q11102_q11023_q11008_check_fly_mount_SpellScript);

            SpellCastResult CheckRequirement()
            {
                Unit* caster = GetCaster();
                // This spell will be cast only if caster has one of these auras
                if (!(caster->HasAuraType(SPELL_AURA_FLY) || caster->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_q11010_q11102_q11023_q11008_check_fly_mount_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_q11010_q11102_q11023_q11008_check_fly_mount_SpellScript();
        }
};

enum Fumping
{
    SPELL_SUMMON_SAND_GNOME  = 39240,
    SPELL_SUMMON_BONE_SLICER = 39241
};

// 39238 - Fumping
class spell_q10929_fumping : SpellScriptLoader
{
    public:
        spell_q10929_fumping() : SpellScriptLoader("spell_q10929_fumping") { }

        class spell_q10929_fumpingAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_q10929_fumpingAuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                return ValidateSpellInfo({ SPELL_SUMMON_SAND_GNOME, SPELL_SUMMON_BONE_SLICER });
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster, urand(SPELL_SUMMON_SAND_GNOME, SPELL_SUMMON_BONE_SLICER), true);
            }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_q10929_fumpingAuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_q10929_fumpingAuraScript();
    }
};

void AddSC_quest_spell_scripts()
{
    new spell_q55_sacred_cleansing();
    new spell_q2203_thaumaturgy_channel();
    new spell_q5206_test_fetid_skull();
    new spell_q6124_6129_apply_salve();
    new spell_q10255_administer_antidote();
    new spell_q11515_fel_siphon_dummy();
    new spell_q10041_q10040_who_are_they();
    new spell_symbol_of_life_dummy();
    new spell_q9874_liquid_fire();
    new spell_q9452_cast_net();
    new spell_q11010_q11102_q11023_aggro_check_aura();
    new spell_q11010_q11102_q11023_aggro_check();
    new spell_q11010_q11102_q11023_aggro_burst();
    new spell_q11010_q11102_q11023_choose_loc();
    new spell_q11010_q11102_q11023_q11008_check_fly_mount();
    new spell_q10929_fumping();
}
