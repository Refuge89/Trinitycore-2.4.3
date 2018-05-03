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
 * Scripts for spells with SPELLFAMILY_GENERIC which cannot be included in AI script file
 * of creature using it or can't be bound to any player class.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_gen_"
 */

#include "ScriptMgr.h"
#include "Battleground.h"
#include "CellImpl.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Item.h"
#include "LFGMgr.h"
#include "Log.h"
#include "Pet.h"
#include "ReputationMgr.h"
#include "SkillDiscovery.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

class spell_gen_absorb0_hitlimit1 : public AuraScript
{
    PrepareAuraScript(spell_gen_absorb0_hitlimit1);

    uint32 limit = 0;

    bool Load() override
    {
        // Max absorb stored in 1 dummy effect
        limit = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
        return true;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
    {
        absorbAmount = std::min(limit, absorbAmount);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_gen_absorb0_hitlimit1::Absorb, EFFECT_0);
    }
};

// 28764 - Adaptive Warding (Frostfire Regalia Set)
enum AdaptiveWarding
{
    SPELL_GEN_ADAPTIVE_WARDING_FIRE     = 28765,
    SPELL_GEN_ADAPTIVE_WARDING_NATURE   = 28768,
    SPELL_GEN_ADAPTIVE_WARDING_FROST    = 28766,
    SPELL_GEN_ADAPTIVE_WARDING_SHADOW   = 28769,
    SPELL_GEN_ADAPTIVE_WARDING_ARCANE   = 28770
};

class spell_gen_adaptive_warding : public AuraScript
{
    PrepareAuraScript(spell_gen_adaptive_warding);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_GEN_ADAPTIVE_WARDING_FIRE,
            SPELL_GEN_ADAPTIVE_WARDING_NATURE,
            SPELL_GEN_ADAPTIVE_WARDING_FROST,
            SPELL_GEN_ADAPTIVE_WARDING_SHADOW,
            SPELL_GEN_ADAPTIVE_WARDING_ARCANE
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetSpellInfo())
            return false;

        // find Mage Armor
        if (!GetTarget()->GetAuraEffectByFamilyFlags(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT, SPELLFAMILY_MAGE, 0x10000000, 0x0))
            return false;

        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
            case SPELL_SCHOOL_NORMAL:
            case SPELL_SCHOOL_HOLY:
                return false;
            default:
                break;
        }
        return true;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        uint32 spellId = 0;
        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
            case SPELL_SCHOOL_FIRE:
                spellId = SPELL_GEN_ADAPTIVE_WARDING_FIRE;
                break;
            case SPELL_SCHOOL_NATURE:
                spellId = SPELL_GEN_ADAPTIVE_WARDING_NATURE;
                break;
            case SPELL_SCHOOL_FROST:
                spellId = SPELL_GEN_ADAPTIVE_WARDING_FROST;
                break;
            case SPELL_SCHOOL_SHADOW:
                spellId = SPELL_GEN_ADAPTIVE_WARDING_SHADOW;
                break;
            case SPELL_SCHOOL_ARCANE:
                spellId = SPELL_GEN_ADAPTIVE_WARDING_ARCANE;
                break;
            default:
                return;
        }
        GetTarget()->CastSpell(GetTarget(), spellId, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_gen_adaptive_warding::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_gen_adaptive_warding::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_gen_allow_cast_from_item_only : public SpellScript
{
    PrepareSpellScript(spell_gen_allow_cast_from_item_only);

    SpellCastResult CheckRequirement()
    {
        if (!GetCastItem())
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_gen_allow_cast_from_item_only::CheckRequirement);
    }
};

// 430 Drink
// 431 Drink
// 432 Drink
// 1133 Drink
// 1135 Drink
// 1137 Drink
// 10250 Drink
// 22734 Drink
// 27089 Drink
// 34291 Drink
// 43182 Drink
// 43183 Drink
// 46755 Drink
// 49472 Drink Coffee
// 57073 Drink
// 61830 Drink
// 72623 Drink
class spell_gen_arena_drink : public AuraScript
{
    PrepareAuraScript(spell_gen_arena_drink);

    bool Load() override
    {
        return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    bool Validate(SpellInfo const* spellInfo) override
    {
        if (!spellInfo->Effects[EFFECT_0].IsAura() || spellInfo->Effects[EFFECT_0].ApplyAuraName != SPELL_AURA_MOD_POWER_REGEN)
        {
            TC_LOG_ERROR("spells", "Aura %d structure has been changed - first aura is no longer SPELL_AURA_MOD_POWER_REGEN", GetId());
            return false;
        }

        return true;
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& /*amplitude*/)
    {
        // Get SPELL_AURA_MOD_POWER_REGEN aura from spell
        AuraEffect* regen = GetAura()->GetEffect(EFFECT_0);
        if (!regen)
            return;

        // default case - not in arena
        if (!GetCaster()->ToPlayer()->InArena())
            isPeriodic = false;
    }

    void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        AuraEffect* regen = GetAura()->GetEffect(EFFECT_0);
        if (!regen)
            return;

        // default case - not in arena
        if (!GetCaster()->ToPlayer()->InArena())
            regen->ChangeAmount(amount);
    }

    void UpdatePeriodic(AuraEffect* aurEff)
    {
        AuraEffect* regen = GetAura()->GetEffect(EFFECT_0);
        if (!regen)
            return;

        // **********************************************
        // This feature used only in arenas
        // **********************************************
        // Here need increase mana regen per tick (6 second rule)
        // on 0 tick -   0  (handled in 2 second)
        // on 1 tick - 166% (handled in 4 second)
        // on 2 tick - 133% (handled in 6 second)

        // Apply bonus for 1 - 4 tick
        switch (aurEff->GetTickNumber())
        {
            case 1:   // 0%
                regen->ChangeAmount(0);
                break;
            case 2:   // 166%
                regen->ChangeAmount(aurEff->GetAmount() * 5 / 3);
                break;
            case 3:   // 133%
                regen->ChangeAmount(aurEff->GetAmount() * 4 / 3);
                break;
            default:  // 100% - normal regen
                regen->ChangeAmount(aurEff->GetAmount());
                // No need to update after 4th tick
                aurEff->SetPeriodic(false);
                break;
        }
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_gen_arena_drink::CalcPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_arena_drink::CalcAmount, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_gen_arena_drink::UpdatePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 41337 Aura of Anger
class spell_gen_aura_of_anger : public AuraScript
{
    PrepareAuraScript(spell_gen_aura_of_anger);

    void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
    {
        if (AuraEffect* aurEff1 = aurEff->GetBase()->GetEffect(EFFECT_1))
            aurEff1->ChangeAmount(aurEff1->GetAmount() + 5);
        aurEff->SetAmount(100 * aurEff->GetTickNumber());
    }

    void Register() override
    {
        OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_gen_aura_of_anger::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 28313 - Aura of Fear
class spell_gen_aura_of_fear : public AuraScript
{
    PrepareAuraScript(spell_gen_aura_of_fear);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->Effects[EFFECT_0].TriggerSpell });
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        if (!roll_chance_i(GetSpellInfo()->ProcChance))
            return;

        GetTarget()->CastSpell(nullptr, GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_aura_of_fear::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_gen_av_drekthar_presence : public AuraScript
{
    PrepareAuraScript(spell_gen_av_drekthar_presence);

    bool CheckAreaTarget(Unit* target)
    {
        switch (target->GetEntry())
        {
            // alliance
            case 14762: // Dun Baldar North Marshal
            case 14763: // Dun Baldar South Marshal
            case 14764: // Icewing Marshal
            case 14765: // Stonehearth Marshal
            case 11948: // Vandar Stormspike
            // horde
            case 14772: // East Frostwolf Warmaster
            case 14776: // Tower Point Warmaster
            case 14773: // Iceblood Warmaster
            case 14777: // West Frostwolf Warmaster
            case 11946: // Drek'thar
                return true;
            default:
                return false;
        }
    }

    void Register() override
    {
        DoCheckAreaTarget += AuraCheckAreaTargetFn(spell_gen_av_drekthar_presence::CheckAreaTarget);
    }
};

enum GenericBandage
{
    SPELL_RECENTLY_BANDAGED     = 11196
};

class spell_gen_bandage : public SpellScript
{
    PrepareSpellScript(spell_gen_bandage);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_RECENTLY_BANDAGED });
    }

    SpellCastResult CheckCast()
    {
        if (Unit* target = GetExplTargetUnit())
        {
            if (target->HasAura(SPELL_RECENTLY_BANDAGED))
                return SPELL_FAILED_TARGET_AURASTATE;
        }
        return SPELL_CAST_OK;
    }

    void HandleScript()
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_RECENTLY_BANDAGED, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_gen_bandage::CheckCast);
        AfterHit += SpellHitFn(spell_gen_bandage::HandleScript);
    }
};

// 46394 Brutallus Burn
class spell_gen_burn_brutallus : public AuraScript
{
    PrepareAuraScript(spell_gen_burn_brutallus);

    void HandleEffectPeriodicUpdate(AuraEffect* aurEff)
    {
        if (aurEff->GetTickNumber() % 11 == 0)
            aurEff->SetAmount(aurEff->GetAmount() * 2);
    }

    void Register() override
    {
        OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_gen_burn_brutallus::HandleEffectPeriodicUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

enum CannibalizeSpells
{
    SPELL_CANNIBALIZE_TRIGGERED = 20578
};

class spell_gen_cannibalize : public SpellScript
{
    PrepareSpellScript(spell_gen_cannibalize);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CANNIBALIZE_TRIGGERED });
    }

    SpellCastResult CheckIfCorpseNear()
    {
        Unit* caster = GetCaster();
        float max_range = GetSpellInfo()->GetMaxRange();
        WorldObject* result = nullptr;
        // search for nearby enemy corpse in range
        Trinity::AnyDeadUnitSpellTargetInRangeCheck check(caster, max_range, GetSpellInfo(), TARGET_CHECK_ENEMY);
        Trinity::WorldObjectSearcher<Trinity::AnyDeadUnitSpellTargetInRangeCheck> searcher(caster, result, check);
        Cell::VisitWorldObjects(caster, searcher, max_range);
        if (!result)
            Cell::VisitGridObjects(caster, searcher, max_range);
        if (!result)
            return SPELL_FAILED_NO_EDIBLE_CORPSES;
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_CANNIBALIZE_TRIGGERED, false);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_gen_cannibalize::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_gen_cannibalize::CheckIfCorpseNear);
    }
};

enum ChaosBlast
{
    SPELL_CHAOS_BLAST   = 37675
};

class spell_gen_chaos_blast : public SpellScript
{
    PrepareSpellScript(spell_gen_chaos_blast);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CHAOS_BLAST });
    }
    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        int32 basepoints0 = 100;
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
        {
            CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
            args.AddSpellBP0(basepoints0);
            caster->CastSpell(target, SPELL_CHAOS_BLAST, args);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_chaos_blast::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum ChokingVines
{
    SPELL_CHOKING_WOUND = 35247
};

// 35244 - Choking Vines
class spell_gen_choking_vines : public AuraScript
{
    PrepareAuraScript(spell_gen_choking_vines);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CHOKING_WOUND });
    }

    void HandleChoke(AuraEffect const* /*aurEff*/)
    {
        if (GetStackAmount() != GetSpellInfo()->StackAmount)
            return;

        GetTarget()->CastSpell(nullptr, SPELL_CHOKING_WOUND, true);
        Remove();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_choking_vines::HandleChoke, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 5138 - Drain Mana
// 8129 - Mana Burn
class spell_gen_clear_fear_poly : public SpellScript
{
    PrepareSpellScript(spell_gen_clear_fear_poly);

    void HandleAfterHit()
    {
        if (Unit* unitTarget = GetHitUnit())
            unitTarget->RemoveAurasWithMechanic((1 << MECHANIC_FEAR) | (1 << MECHANIC_POLYMORPH));
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_gen_clear_fear_poly::HandleAfterHit);
    }
};

class spell_gen_clone : public SpellScript
{
    PrepareSpellScript(spell_gen_clone);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        GetHitUnit()->CastSpell(GetCaster(), uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_clone::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        OnEffectHitTarget += SpellEffectFn(spell_gen_clone::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum CloneWeaponSpells
{
    SPELL_COPY_WEAPON_AURA       = 41054,
    SPELL_COPY_OFFHAND_AURA      = 45205
};

class spell_gen_clone_weapon : public SpellScript
{
    PrepareSpellScript(spell_gen_clone_weapon);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        GetHitUnit()->CastSpell(GetCaster(), uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_clone_weapon::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_gen_clone_weapon_aura : public AuraScript
{
    PrepareAuraScript(spell_gen_clone_weapon_aura);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_COPY_WEAPON_AURA,
            SPELL_COPY_OFFHAND_AURA
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        switch (GetSpellInfo()->Id)
        {
            case SPELL_COPY_WEAPON_AURA:
            {
                prevItem = target->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID);
                uint32 newItem = caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID);

                if (Player* player = caster->ToPlayer())
                    if (Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                        newItem = mainItem->GetEntry();

                if (Creature* creature = target->ToCreature())
                    creature->SetVirtualItem(0, newItem);
                else
                    target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, newItem);
                break;
            }
            case SPELL_COPY_OFFHAND_AURA:
            {
                prevItem = target->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1);
                uint32 newItem = caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1);

                if (Player* player = caster->ToPlayer())
                    if (Item* offItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                        newItem = offItem->GetEntry();

                if (Creature* creature = target->ToCreature())
                    creature->SetVirtualItem(1, newItem);
                else
                    target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, newItem);
                break;
            }
            default:
                break;
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();

        switch (GetSpellInfo()->Id)
        {
            case SPELL_COPY_WEAPON_AURA:
                if (Creature* creature = target->ToCreature())
                    creature->SetVirtualItem(0, prevItem);
                else
                    target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, prevItem);
                break;
            case SPELL_COPY_OFFHAND_AURA:
                if (Creature* creature = target->ToCreature())
                    creature->SetVirtualItem(1, prevItem);
                else
                    target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, prevItem);
                break;
            default:
                break;
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_gen_clone_weapon_aura::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectRemove += AuraEffectRemoveFn(spell_gen_clone_weapon_aura::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }

    uint32 prevItem = 0;
};

class spell_gen_count_pct_from_max_hp : public SpellScriptLoader
{
    public:
        spell_gen_count_pct_from_max_hp(char const* name, int32 damagePct = 0) : SpellScriptLoader(name), _damagePct(damagePct) { }

        class spell_gen_count_pct_from_max_hp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_gen_count_pct_from_max_hp_SpellScript);

        public:
            spell_gen_count_pct_from_max_hp_SpellScript(int32 damagePct) : SpellScript(), _damagePct(damagePct) { }

            void RecalculateDamage()
            {
                if (!_damagePct)
                    _damagePct = GetHitDamage();

                SetHitDamage(GetHitUnit()->CountPctFromMaxHealth(_damagePct));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_gen_count_pct_from_max_hp_SpellScript::RecalculateDamage);
            }

        private:
            int32 _damagePct;
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_gen_count_pct_from_max_hp_SpellScript(_damagePct);
        }

    private:
        int32 _damagePct;
};

class spell_gen_creature_permanent_feign_death : public AuraScript
{
    PrepareAuraScript(spell_gen_creature_permanent_feign_death);

    void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);

        if (Creature* creature = target->ToCreature())
            creature->SetReactState(REACT_PASSIVE);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);

        if (Creature* creature = target->ToCreature())
            creature->InitializeReactState();
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_gen_creature_permanent_feign_death::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_gen_creature_permanent_feign_death::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_gen_decay_over_time : public SpellScriptLoader
{
    public:
        spell_gen_decay_over_time(char const* name) : SpellScriptLoader(name) { }

        SpellScript* GetSpellScript() const override
        {
            return new spell_gen_decay_over_time_SpellScript();
        }

    private:
        class spell_gen_decay_over_time_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_gen_decay_over_time_SpellScript);

            void ModAuraStack()
            {
                if (Aura* aur = GetHitAura())
                    aur->SetStackAmount(static_cast<uint8>(GetSpellInfo()->StackAmount));
            }

            void Register() override
            {
                AfterHit += SpellHitFn(spell_gen_decay_over_time_SpellScript::ModAuraStack);
            }
        };

    protected:
        class spell_gen_decay_over_time_AuraScript : public AuraScript
        {
            protected:
                PrepareAuraScript(spell_gen_decay_over_time_AuraScript);

                bool CheckProc(ProcEventInfo& eventInfo)
                {
                    return (eventInfo.GetSpellInfo() == GetSpellInfo());
                }

                void Decay(ProcEventInfo& /*eventInfo*/)
                {
                    PreventDefaultAction();
                    ModStackAmount(-1);
                }

                void Register() override
                {
                    DoCheckProc += AuraCheckProcFn(spell_gen_decay_over_time_AuraScript::CheckProc);
                    OnProc += AuraProcFn(spell_gen_decay_over_time_AuraScript::Decay);
                }

                ~spell_gen_decay_over_time_AuraScript() = default;
        };

        ~spell_gen_decay_over_time() = default;
};

enum FungalDecay
{
    // found in sniffs, there is no duration entry we can possibly use
    AURA_DURATION = 12600
};

// 32065 - Fungal Decay
class spell_gen_decay_over_time_fungal_decay : public spell_gen_decay_over_time
{
    public:
        spell_gen_decay_over_time_fungal_decay() : spell_gen_decay_over_time("spell_gen_decay_over_time_fungal_decay") { }

        class spell_gen_decay_over_time_fungal_decay_AuraScript : public spell_gen_decay_over_time_AuraScript
        {
            PrepareAuraScript(spell_gen_decay_over_time_fungal_decay_AuraScript);

            void ModDuration(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                // only on actual reapply, not on stack decay
                if (GetDuration() == GetMaxDuration())
                {
                    SetMaxDuration(AURA_DURATION);
                    SetDuration(AURA_DURATION);
                }
            }

            void Register() override
            {
                spell_gen_decay_over_time_AuraScript::Register();
                OnEffectApply += AuraEffectApplyFn(spell_gen_decay_over_time_fungal_decay_AuraScript::ModDuration, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_gen_decay_over_time_fungal_decay_AuraScript();
        }
};

// 36659 - Tail Sting
class spell_gen_decay_over_time_tail_sting : public spell_gen_decay_over_time
{
    public:
        spell_gen_decay_over_time_tail_sting() : spell_gen_decay_over_time("spell_gen_decay_over_time_tail_sting") { }

        class spell_gen_decay_over_time_tail_sting_AuraScript : public spell_gen_decay_over_time_AuraScript
        {
            PrepareAuraScript(spell_gen_decay_over_time_tail_sting_AuraScript);
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_gen_decay_over_time_tail_sting_AuraScript();
        }
};

class spell_gen_despawn_self : public SpellScript
{
    PrepareSpellScript(spell_gen_despawn_self);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        if (GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_DUMMY || GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_SCRIPT_EFFECT)
            GetCaster()->ToCreature()->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_despawn_self::HandleDummy, EFFECT_ALL, SPELL_EFFECT_ANY);
    }
};

class spell_gen_dungeon_credit : public SpellScript
{
    PrepareSpellScript(spell_gen_dungeon_credit);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void CreditEncounter()
    {
        // This hook is executed for every target, make sure we only credit instance once
        if (_handled)
            return;

        _handled = true;
        Unit* caster = GetCaster();
        if (InstanceScript* instance = caster->GetInstanceScript())
            instance->UpdateEncounterStateForSpellCast(GetSpellInfo()->Id, caster);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_gen_dungeon_credit::CreditEncounter);
    }

    bool _handled = false;
};

enum EluneCandle
{
    // Creatures
    NPC_OMEN                       = 15467,

    // Spells
    SPELL_ELUNE_CANDLE_OMEN_HEAD   = 26622,
    SPELL_ELUNE_CANDLE_OMEN_CHEST  = 26624,
    SPELL_ELUNE_CANDLE_OMEN_HAND_R = 26625,
    SPELL_ELUNE_CANDLE_OMEN_HAND_L = 26649,
    SPELL_ELUNE_CANDLE_NORMAL      = 26636
};

class spell_gen_elune_candle : public SpellScript
{
    PrepareSpellScript(spell_gen_elune_candle);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_ELUNE_CANDLE_OMEN_HEAD,
            SPELL_ELUNE_CANDLE_OMEN_CHEST,
            SPELL_ELUNE_CANDLE_OMEN_HAND_R,
            SPELL_ELUNE_CANDLE_OMEN_HAND_L,
            SPELL_ELUNE_CANDLE_NORMAL
        });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        uint32 spellId = 0;

        if (GetHitUnit()->GetEntry() == NPC_OMEN)
        {
            switch (urand(0, 3))
            {
                case 0:
                    spellId = SPELL_ELUNE_CANDLE_OMEN_HEAD;
                    break;
                case 1:
                    spellId = SPELL_ELUNE_CANDLE_OMEN_CHEST;
                    break;
                case 2:
                    spellId = SPELL_ELUNE_CANDLE_OMEN_HAND_R;
                    break;
                case 3:
                    spellId = SPELL_ELUNE_CANDLE_OMEN_HAND_L;
                    break;
            }
        }
        else
            spellId = SPELL_ELUNE_CANDLE_NORMAL;

        GetCaster()->CastSpell(GetHitUnit(), spellId, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_elune_candle::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum TransporterBackfires
{
    SPELL_TRANSPORTER_MALFUNCTION_POLYMORPH     = 23444,
    SPELL_TRANSPORTER_EVIL_TWIN                 = 23445,
    SPELL_TRANSPORTER_MALFUNCTION_MISS          = 36902
};

class spell_gen_gadgetzan_transporter_backfire : public SpellScript
{
    PrepareSpellScript(spell_gen_gadgetzan_transporter_backfire);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_TRANSPORTER_MALFUNCTION_POLYMORPH,
            SPELL_TRANSPORTER_EVIL_TWIN,
            SPELL_TRANSPORTER_MALFUNCTION_MISS
        });
    }

    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        Unit* caster = GetCaster();
        int32 r = irand(0, 119);
        if (r < 20)                           // Transporter Malfunction - 1/6 polymorph
            caster->CastSpell(caster, SPELL_TRANSPORTER_MALFUNCTION_POLYMORPH, true);
        else if (r < 100)                     // Evil Twin               - 4/6 evil twin
            caster->CastSpell(caster, SPELL_TRANSPORTER_EVIL_TWIN, true);
        else                                    // Transporter Malfunction - 1/6 miss the target
            caster->CastSpell(caster, SPELL_TRANSPORTER_MALFUNCTION_MISS, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_gadgetzan_transporter_backfire::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};


class spell_gen_gift_of_naaru : public AuraScript
{
    PrepareAuraScript(spell_gen_gift_of_naaru);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (!GetCaster() || !aurEff->GetTotalTicks())
            return;

        float heal = 0.0f;
        switch (GetSpellInfo()->SpellFamilyName)
        {
            case SPELLFAMILY_MAGE:
            case SPELLFAMILY_WARLOCK:
            case SPELLFAMILY_PRIEST:
                heal = 1.885f * float(GetCaster()->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()));
                break;
            case SPELLFAMILY_PALADIN:
            case SPELLFAMILY_SHAMAN:
                heal = std::max(1.885f * float(GetCaster()->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask())), 1.1f * float(GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
                break;
            case SPELLFAMILY_WARRIOR:
            case SPELLFAMILY_HUNTER:
                heal = 1.1f * float(std::max(GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK), GetCaster()->GetTotalAttackPowerValue(RANGED_ATTACK)));
                break;
            case SPELLFAMILY_GENERIC:
            default:
                break;
        }

        int32 healTick = std::floor(heal / aurEff->GetTotalTicks());
        amount += int32(std::max(healTick, 0));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_gift_of_naaru::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
    }
};

enum GnomishTransporter
{
    SPELL_TRANSPORTER_SUCCESS                   = 23441,
    SPELL_TRANSPORTER_FAILURE                   = 23446
};

class spell_gen_gnomish_transporter : public SpellScript
{
    PrepareSpellScript(spell_gen_gnomish_transporter);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_TRANSPORTER_SUCCESS,
            SPELL_TRANSPORTER_FAILURE
        });
    }

    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        GetCaster()->CastSpell(GetCaster(), roll_chance_i(50) ? SPELL_TRANSPORTER_SUCCESS : SPELL_TRANSPORTER_FAILURE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_gnomish_transporter::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_gen_lifeblood : public AuraScript
{
    PrepareAuraScript(spell_gen_lifeblood);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (!aurEff->GetTotalTicks())
            return;

        if (Unit* owner = GetUnitOwner())
            amount += int32(CalculatePct(owner->GetMaxHealth(), 1.5f / aurEff->GetTotalTicks()));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_lifeblood::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
    }
};

enum GenericLifebloom
{
    SPELL_HEXLORD_MALACRASS_LIFEBLOOM_FINAL_HEAL        = 43422
};

class spell_gen_lifebloom : public SpellScriptLoader
{
    public:
        spell_gen_lifebloom(char const* name, uint32 spellId) : SpellScriptLoader(name), _spellId(spellId) { }

        class spell_gen_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_gen_lifebloom_AuraScript);

        public:
            spell_gen_lifebloom_AuraScript(uint32 spellId) : AuraScript(), _spellId(spellId) { }

        private:
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ _spellId });
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // final heal only on duration end or dispel
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                    return;

                // final heal
                GetTarget()->CastSpell(GetTarget(), _spellId, { aurEff, GetCasterGUID() });
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_gen_lifebloom_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }

            uint32 _spellId;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_gen_lifebloom_AuraScript(_spellId);
        }

    private:
        uint32 _spellId;
};

enum Mounts
{
    // Magic Broom
    SPELL_MAGIC_BROOM_60                = 42680,
    SPELL_MAGIC_BROOM_100               = 42683,
    SPELL_MAGIC_BROOM_150               = 42667,
    SPELL_MAGIC_BROOM_280               = 42668,

    // Headless Horseman's Mount
    SPELL_HEADLESS_HORSEMAN_MOUNT_60    = 51621,
    SPELL_HEADLESS_HORSEMAN_MOUNT_100   = 48024,
    SPELL_HEADLESS_HORSEMAN_MOUNT_150   = 51617,
    SPELL_HEADLESS_HORSEMAN_MOUNT_280   = 48023
};

class spell_gen_mount : public SpellScriptLoader
{
    public:
        spell_gen_mount(char const* name, uint32 mount0 = 0, uint32 mount60 = 0, uint32 mount100 = 0, uint32 mount150 = 0, uint32 mount280 = 0) : SpellScriptLoader(name),
            _mount0(mount0), _mount60(mount60), _mount100(mount100), _mount150(mount150), _mount280(mount280) { }

        class spell_gen_mount_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_gen_mount_SpellScript);

        public:
            spell_gen_mount_SpellScript(uint32 mount0, uint32 mount60, uint32 mount100, uint32 mount150, uint32 mount280) : SpellScript(),
                _mount0(mount0), _mount60(mount60), _mount100(mount100), _mount150(mount150), _mount280(mount280) { }

        private:
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                bool result = true;
                if (_mount0)
                    result &= ValidateSpellInfo({ _mount0 });
                if (_mount60)
                    result &= ValidateSpellInfo({ _mount60 });
                if (_mount100)
                    result &= ValidateSpellInfo({ _mount100 });
                if (_mount150)
                    result &= ValidateSpellInfo({ _mount150 });
                if (_mount280)
                    result &= ValidateSpellInfo({ _mount280 });

                return result;
            }

            void HandleMount(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (Player* target = GetHitPlayer())
                {
                    // Prevent stacking of mounts and client crashes upon dismounting
                    target->RemoveAurasByType(SPELL_AURA_MOUNTED, ObjectGuid::Empty, GetHitAura());

                    // Triggered spell id dependent on riding skill and zone
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(_mount150);
                    uint32 zoneid, areaid;
                    target->GetZoneAndAreaId(zoneid, areaid);
                    bool const canFly = spellInfo && (spellInfo->CheckLocation(target->GetMapId(), zoneid, areaid, target) == SPELL_CAST_OK);

                    uint32 mount = 0;
                    switch (target->GetBaseSkillValue(SKILL_RIDING))
                    {
                        case 0:
                            mount = _mount0;
                            break;
                        case 75:
                            mount = _mount60;
                            break;
                        case 150:
                            mount = _mount100;
                            break;
                        case 225:
                            if (canFly)
                                mount = _mount150;
                            else
                                mount = _mount100;
                            break;
                        case 300:
                            if (canFly)
                                mount = _mount280;
                            else
                                mount = _mount100;
                            break;
                        default:
                            break;
                    }

                    if (mount)
                        target->CastSpell(target, mount, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_gen_mount_SpellScript::HandleMount, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }

            uint32 _mount0;
            uint32 _mount60;
            uint32 _mount100;
            uint32 _mount150;
            uint32 _mount280;
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_gen_mount_SpellScript(_mount0, _mount60, _mount100, _mount150, _mount280);
        }

    private:
        uint32 _mount0;
        uint32 _mount60;
        uint32 _mount100;
        uint32 _mount150;
        uint32 _mount280;
};

enum MossCoveredFeet
{
    SPELL_FALL_DOWN = 6869
};

// 6870 Moss Covered Feet
// 31399 Moss Covered Feet
class spell_gen_moss_covered_feet : public AuraScript
{
    PrepareAuraScript(spell_gen_moss_covered_feet);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_FALL_DOWN });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActionTarget()->CastSpell(nullptr, SPELL_FALL_DOWN, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_gen_moss_covered_feet::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 46284 - Negative Energy Periodic
class spell_gen_negative_energy_periodic : public AuraScript
{
    PrepareAuraScript(spell_gen_negative_energy_periodic);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->Effects[EFFECT_0].TriggerSpell });
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        CastSpellExtraArgs args(aurEff);
        args.AddSpellMod(SPELLVALUE_MAX_TARGETS, aurEff->GetTickNumber() / 10 + 1);
        GetTarget()->CastSpell(nullptr, GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell, args);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_negative_energy_periodic::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

enum Netherbloom : uint32
{
    SPELL_NETHERBLOOM_POLLEN_1      = 28703
};

// 28702 - Netherbloom
class spell_gen_netherbloom : public SpellScript
{
    PrepareSpellScript(spell_gen_netherbloom);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        for (uint8 i = 0; i < 5; ++i)
            if (!ValidateSpellInfo({ SPELL_NETHERBLOOM_POLLEN_1 + i }))
                return false;

        return true;
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (Unit* target = GetHitUnit())
        {
            // 25% chance of casting a random buff
            if (roll_chance_i(75))
                return;

            // triggered spells are 28703 to 28707
            // Note: some sources say, that there was the possibility of
            //       receiving a debuff. However, this seems to be removed by a patch.

            // don't overwrite an existing aura
            for (uint8 i = 0; i < 5; ++i)
                if (target->HasAura(SPELL_NETHERBLOOM_POLLEN_1 + i))
                    return;

            target->CastSpell(target, SPELL_NETHERBLOOM_POLLEN_1 + urand(0, 4), true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_netherbloom::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum NightmareVine
{
    SPELL_NIGHTMARE_POLLEN      = 28721
};

// 28720 - Nightmare Vine
class spell_gen_nightmare_vine : public SpellScript
{
    PrepareSpellScript(spell_gen_nightmare_vine);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_NIGHTMARE_POLLEN });
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (Unit* target = GetHitUnit())
        {
            // 25% chance of casting Nightmare Pollen
            if (roll_chance_i(25))
                target->CastSpell(target, SPELL_NIGHTMARE_POLLEN, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_nightmare_vine::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 27746 -  Nitrous Boost
class spell_gen_nitrous_boost : public AuraScript
{
    PrepareAuraScript(spell_gen_nitrous_boost);

    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        if (GetCaster() && GetTarget()->GetPower(POWER_MANA) >= 10)
            GetTarget()->ModifyPower(POWER_MANA, -10);
        else
            Remove();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_nitrous_boost::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

enum ObsidianArmor
{
    SPELL_GEN_OBSIDIAN_ARMOR_HOLY       = 27536,
    SPELL_GEN_OBSIDIAN_ARMOR_FIRE       = 27533,
    SPELL_GEN_OBSIDIAN_ARMOR_NATURE     = 27538,
    SPELL_GEN_OBSIDIAN_ARMOR_FROST      = 27534,
    SPELL_GEN_OBSIDIAN_ARMOR_SHADOW     = 27535,
    SPELL_GEN_OBSIDIAN_ARMOR_ARCANE     = 27540
};

// 27539 - Obsidian Armor
class spell_gen_obsidian_armor : public AuraScript
{
    PrepareAuraScript(spell_gen_obsidian_armor);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_GEN_OBSIDIAN_ARMOR_HOLY,
            SPELL_GEN_OBSIDIAN_ARMOR_FIRE,
            SPELL_GEN_OBSIDIAN_ARMOR_NATURE,
            SPELL_GEN_OBSIDIAN_ARMOR_FROST,
            SPELL_GEN_OBSIDIAN_ARMOR_SHADOW,
            SPELL_GEN_OBSIDIAN_ARMOR_ARCANE
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetSpellInfo())
            return false;

        if (GetFirstSchoolInMask(eventInfo.GetSchoolMask()) == SPELL_SCHOOL_NORMAL)
            return false;

        return true;
    }

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        uint32 spellId = 0;
        switch (GetFirstSchoolInMask(eventInfo.GetSchoolMask()))
        {
            case SPELL_SCHOOL_HOLY:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_HOLY;
                break;
            case SPELL_SCHOOL_FIRE:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_FIRE;
                break;
            case SPELL_SCHOOL_NATURE:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_NATURE;
                break;
            case SPELL_SCHOOL_FROST:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_FROST;
                break;
            case SPELL_SCHOOL_SHADOW:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_SHADOW;
                break;
            case SPELL_SCHOOL_ARCANE:
                spellId = SPELL_GEN_OBSIDIAN_ARMOR_ARCANE;
                break;
            default:
                return;
        }
        GetTarget()->CastSpell(GetTarget(), spellId, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_gen_obsidian_armor::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_gen_obsidian_armor::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

enum ParalyticPoison
{
    SPELL_PARALYSIS = 35202
};

// 35201 - Paralytic Poison
class spell_gen_paralytic_poison : public AuraScript
{
    PrepareAuraScript(spell_gen_paralytic_poison);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PARALYSIS });
    }

    void HandleStun(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        GetTarget()->CastSpell(nullptr, SPELL_PARALYSIS, aurEff);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_gen_paralytic_poison::HandleStun, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_gen_proc_below_pct_damaged : public SpellScriptLoader
{
    public:
        spell_gen_proc_below_pct_damaged(char const* name) : SpellScriptLoader(name) { }

        class spell_gen_proc_below_pct_damaged_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_gen_proc_below_pct_damaged_AuraScript);

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                DamageInfo* damageInfo = eventInfo.GetDamageInfo();
                if (!damageInfo || !damageInfo->GetDamage())
                    return false;

                int32 pct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();

                if (eventInfo.GetActionTarget()->HealthBelowPctDamaged(pct, damageInfo->GetDamage()))
                    return true;

                return false;
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_gen_proc_below_pct_damaged_AuraScript::CheckProc);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_gen_proc_below_pct_damaged_AuraScript();
        }
};

class spell_gen_proc_charge_drop_only : public AuraScript
{
    PrepareAuraScript(spell_gen_proc_charge_drop_only);

    void HandleChargeDrop(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
    }

    void Register() override
    {
        OnProc += AuraProcFn(spell_gen_proc_charge_drop_only::HandleChargeDrop);
    }
};

enum ParachuteSpells
{
    SPELL_PARACHUTE         = 45472,
    SPELL_PARACHUTE_BUFF    = 44795,
};

// 45472 Parachute
class spell_gen_parachute : public AuraScript
{
    PrepareAuraScript(spell_gen_parachute);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_PARACHUTE,
            SPELL_PARACHUTE_BUFF
        });
    }

    void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Player* target = GetTarget()->ToPlayer())
            if (target->IsFalling())
            {
                target->RemoveAurasDueToSpell(SPELL_PARACHUTE);
                target->CastSpell(target, SPELL_PARACHUTE_BUFF, true);
            }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_parachute::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

enum PetSummoned
{
    NPC_DOOMGUARD       = 11859,
    NPC_INFERNAL        = 89,
    NPC_IMP             = 416
};

class spell_gen_pet_summoned : public SpellScript
{
    PrepareSpellScript(spell_gen_pet_summoned);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        if (player->GetLastPetNumber())
        {
            PetType newPetType = (player->getClass() == CLASS_HUNTER) ? HUNTER_PET : SUMMON_PET;
            Pet* newPet = new Pet(player, newPetType);
            if (newPet->LoadPetFromDB(player, 0, player->GetLastPetNumber(), true))
            {
                // revive the pet if it is dead
                if (newPet->getDeathState() == DEAD)
                    newPet->SetDeathState(ALIVE);

                newPet->SetFullHealth();
                newPet->SetPower(newPet->GetPowerType(), newPet->GetMaxPower(newPet->GetPowerType()));

                switch (newPet->GetEntry())
                {
                    case NPC_DOOMGUARD:
                    case NPC_INFERNAL:
                        newPet->SetEntry(NPC_IMP);
                        break;
                    default:
                        break;
                }
            }
            else
                delete newPet;
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_pet_summoned::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_gen_profession_research : public SpellScript
{
    PrepareSpellScript(spell_gen_profession_research);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        if (HasDiscoveredAllSpells(GetSpellInfo()->Id, GetCaster()->ToPlayer()))
        {
            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_NOTHING_TO_DISCOVER);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        return SPELL_CAST_OK;
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        uint32 spellId = GetSpellInfo()->Id;

        // learn random explicit discovery recipe (if any)
        if (uint32 discoveredSpellId = GetExplicitDiscoverySpell(spellId, caster))
            caster->LearnSpell(discoveredSpellId, false);

        caster->UpdateCraftSkill(spellId);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_gen_profession_research::CheckRequirement);
        OnEffectHitTarget += SpellEffectFn(spell_gen_profession_research::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_gen_remove_flight_auras : public SpellScript
{
    PrepareSpellScript(spell_gen_remove_flight_auras);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
        {
            target->RemoveAurasByType(SPELL_AURA_FLY);
            target->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_remove_flight_auras::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 23493 - Restoration
// 24379 - Restoration
class spell_gen_restoration : public AuraScript
{
    PrepareAuraScript(spell_gen_restoration);

    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 heal = caster->CountPctFromMaxHealth(10);
        HealInfo healInfo(caster, GetTarget(), heal, GetSpellInfo(), GetSpellInfo()->GetSchoolMask());
        caster->HealBySpell(healInfo);

        /// @todo: should proc other auras?
        if (int32 mana = caster->GetMaxPower(POWER_MANA))
        {
            mana /= 10;
            caster->EnergizeBySpell(caster, GetId(), mana, POWER_MANA);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_restoration::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 38772 Grievous Wound
// 43937 Grievous Wound
// 62331 Impale
// 62418 Impale
class spell_gen_remove_on_health_pct : public AuraScript
{
    PrepareAuraScript(spell_gen_remove_on_health_pct);

    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        // they apply damage so no need to check for ticks here

        if (GetTarget()->HealthAbovePct(GetSpellInfo()->Effects[EFFECT_1].CalcValue()))
        {
            Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_remove_on_health_pct::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 31956 Grievous Wound
// 38801 Grievous Wound
// 43093 Grievous Throw
// 58517 Grievous Wound
// 59262 Grievous Wound
class spell_gen_remove_on_full_health : public AuraScript
{
    PrepareAuraScript(spell_gen_remove_on_full_health);

    void PeriodicTick(AuraEffect const* aurEff)
    {
        // if it has only periodic effect, allow 1 tick
        bool onlyEffect = !GetSpellInfo()->Effects[EFFECT_1].IsEffect() && !GetSpellInfo()->Effects[EFFECT_2].IsEffect();
        if (onlyEffect && aurEff->GetTickNumber() <= 1)
            return;

        if (GetTarget()->IsFullHealth())
        {
            Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_remove_on_full_health::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 70292 - Glacial Strike
// 71316 - Glacial Strike
// 71317 - Glacial Strike
class spell_gen_remove_on_full_health_pct : public AuraScript
{
    PrepareAuraScript(spell_gen_remove_on_full_health_pct);

    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        // they apply damage so no need to check for ticks here

        if (GetTarget()->IsFullHealth())
        {
            Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_remove_on_full_health_pct::PeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
    }
};

enum Replenishment
{
    SPELL_REPLENISHMENT             = 57669,
    SPELL_INFINITE_REPLENISHMENT    = 61782
};

class ReplenishmentCheck
{
public:
    bool operator()(WorldObject* obj) const
    {
        if (Unit* target = obj->ToUnit())
            return target->GetPowerType() != POWER_MANA;

        return true;
    }
};

class spell_gen_replenishment : public SpellScript
{
    PrepareSpellScript(spell_gen_replenishment);

    void RemoveInvalidTargets(std::list<WorldObject*>& targets)
    {
        // In arenas Replenishment may only affect the caster
        if (Player* caster = GetCaster()->ToPlayer())
        {
            if (caster->InArena())
            {
                targets.clear();
                targets.push_back(caster);
                return;
            }
        }

        targets.remove_if(ReplenishmentCheck());

        uint8 const maxTargets = 10;

        if (targets.size() > maxTargets)
        {
            targets.sort(Trinity::PowerPctOrderPred(POWER_MANA));
            targets.resize(maxTargets);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_gen_replenishment::RemoveInvalidTargets, EFFECT_ALL, TARGET_UNIT_CASTER_AREA_RAID);
    }
};

class spell_gen_replenishment_aura : public AuraScript
{
    PrepareAuraScript(spell_gen_replenishment_aura);

    bool Load() override
    {
        return GetUnitOwner()->GetPowerType() == POWER_MANA;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        switch (GetSpellInfo()->Id)
        {
            case SPELL_REPLENISHMENT:
                amount = GetUnitOwner()->GetMaxPower(POWER_MANA) * 0.002f;
                break;
            case SPELL_INFINITE_REPLENISHMENT:
                amount = GetUnitOwner()->GetMaxPower(POWER_MANA) * 0.0025f;
                break;
            default:
                break;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_replenishment_aura::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
    }
};

class spell_gen_spirit_healer_res : public SpellScript
{
    PrepareSpellScript(spell_gen_spirit_healer_res);

    bool Load() override
    {
        return GetOriginalCaster() && GetOriginalCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        Player* originalCaster = GetOriginalCaster()->ToPlayer();
        if (Unit* target = GetHitUnit())
        {
            WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
            data << uint64(target->GetGUID());
            originalCaster->SendDirectMessage(&data);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_spirit_healer_res::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum SummonElemental
{
    SPELL_SUMMON_FIRE_ELEMENTAL  = 8985,
    SPELL_SUMMON_EARTH_ELEMENTAL = 19704
};

class spell_gen_summon_elemental : public SpellScriptLoader
{
    public:
        spell_gen_summon_elemental(char const* name, uint32 spellId) : SpellScriptLoader(name), _spellId(spellId) { }

        class spell_gen_summon_elemental_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_gen_summon_elemental_AuraScript);

        public:
            spell_gen_summon_elemental_AuraScript(uint32 spellId) : AuraScript(), _spellId(spellId) { }

        private:
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ _spellId });
            }

            void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    if (Unit* owner = GetCaster()->GetOwner())
                        owner->CastSpell(owner, _spellId, true);
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    if (Unit* owner = GetCaster()->GetOwner())
                        if (owner->GetTypeId() == TYPEID_PLAYER) /// @todo this check is maybe wrong
                            owner->ToPlayer()->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
            }

            void Register() override
            {
                 AfterEffectApply += AuraEffectApplyFn(spell_gen_summon_elemental_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                 AfterEffectRemove += AuraEffectRemoveFn(spell_gen_summon_elemental_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }

            uint32 _spellId;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_gen_summon_elemental_AuraScript(_spellId);
        }

    private:
        uint32 _spellId;
};

// 41213, 43416, 69222, 73076 - Throw Shield
class spell_gen_throw_shield : public SpellScript
{
    PrepareSpellScript(spell_gen_throw_shield);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        GetCaster()->CastSpell(GetHitUnit(), uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_throw_shield::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum GMFreeze
{
    SPELL_GM_FREEZE = 9454
};

class spell_gen_gm_freeze : public AuraScript
{
    PrepareAuraScript(spell_gen_gm_freeze);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_GM_FREEZE });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // Do what was done before to the target in HandleFreezeCommand
        if (Player* player = GetTarget()->ToPlayer())
        {
            // stop combat + make player unattackable + duel stop + stop some spells
            player->SetFaction(FACTION_FRIENDLY);
            player->CombatStop();
            if (player->IsNonMeleeSpellCast(true))
                player->InterruptNonMeleeSpells(true);
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            // if player class = hunter || warlock remove pet if alive
            if ((player->getClass() == CLASS_HUNTER) || (player->getClass() == CLASS_WARLOCK))
            {
                if (Pet* pet = player->GetPet())
                {
                    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
                    // not let dismiss dead pet
                    if (pet->IsAlive())
                        player->RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
                }
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // Do what was done before to the target in HandleUnfreezeCommand
        if (Player* player = GetTarget()->ToPlayer())
        {
            // Reset player faction + allow combat + allow duels
            player->setFactionForRace(player->getRace());
            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            // save player
            player->SaveToDB();
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_gen_gm_freeze::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_gen_gm_freeze::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_gen_stand : public SpellScript
{
    PrepareSpellScript(spell_gen_stand);

    void HandleScript(SpellEffIndex /*eff*/)
    {
        Creature* target = GetHitCreature();
        if (!target)
            return;

        target->SetStandState(UNIT_STAND_STATE_STAND);
        target->HandleEmoteCommand(EMOTE_STATE_NONE);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_stand::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum RequiredMixologySpells
{
    // Flasks
    SPELL_FLASK_OF_THE_FROST_WYRM       = 53755,
    SPELL_FLASK_OF_STONEBLOOD           = 53758,
    SPELL_FLASK_OF_ENDLESS_RAGE         = 53760,
    SPELL_FLASK_OF_PURE_MOJO            = 54212,
    SPELL_LESSER_FLASK_OF_RESISTANCE    = 62380,
    SPELL_LESSER_FLASK_OF_TOUGHNESS     = 53752,
    SPELL_FLASK_OF_BLINDING_LIGHT       = 28521,
    SPELL_FLASK_OF_CHROMATIC_WONDER     = 42735,
    SPELL_FLASK_OF_FORTIFICATION        = 28518,
    SPELL_FLASK_OF_MIGHTY_RESTORATION   = 28519,
    SPELL_FLASK_OF_PURE_DEATH           = 28540,
    SPELL_FLASK_OF_RELENTLESS_ASSAULT   = 28520,
    SPELL_FLASK_OF_CHROMATIC_RESISTANCE = 17629,
    SPELL_FLASK_OF_DISTILLED_WISDOM     = 17627,
    SPELL_FLASK_OF_SUPREME_POWER        = 17628,
    SPELL_FLASK_OF_THE_TITANS           = 17626,
    // Elixirs
    SPELL_ELIXIR_OF_MIGHTY_AGILITY      = 28497,
    SPELL_ELIXIR_OF_ACCURACY            = 60340,
    SPELL_ELIXIR_OF_DEADLY_STRIKES      = 60341,
    SPELL_ELIXIR_OF_MIGHTY_DEFENSE      = 60343,
    SPELL_ELIXIR_OF_EXPERTISE           = 60344,
    SPELL_ELIXIR_OF_ARMOR_PIERCING      = 60345,
    SPELL_ELIXIR_OF_LIGHTNING_SPEED     = 60346,
    SPELL_ELIXIR_OF_MIGHTY_FORTITUDE    = 53751,
    SPELL_ELIXIR_OF_MIGHTY_MAGEBLOOD    = 53764,
    SPELL_ELIXIR_OF_MIGHTY_STRENGTH     = 53748,
    SPELL_ELIXIR_OF_MIGHTY_TOUGHTS      = 60347,
    SPELL_ELIXIR_OF_PROTECTION          = 53763,
    SPELL_ELIXIR_OF_SPIRIT              = 53747,
    SPELL_GURUS_ELIXIR                  = 53749,
    SPELL_SHADOWPOWER_ELIXIR            = 33721,
    SPELL_WRATH_ELIXIR                  = 53746,
    SPELL_ELIXIR_OF_EMPOWERMENT         = 28514,
    SPELL_ELIXIR_OF_MAJOR_MAGEBLOOD     = 28509,
    SPELL_ELIXIR_OF_MAJOR_SHADOW_POWER  = 28503,
    SPELL_ELIXIR_OF_MAJOR_DEFENSE       = 28502,
    SPELL_FEL_STRENGTH_ELIXIR           = 38954,
    SPELL_ELIXIR_OF_IRONSKIN            = 39628,
    SPELL_ELIXIR_OF_MAJOR_AGILITY       = 54494,
    SPELL_ELIXIR_OF_DRAENIC_WISDOM      = 39627,
    SPELL_ELIXIR_OF_MAJOR_FIREPOWER     = 28501,
    SPELL_ELIXIR_OF_MAJOR_FROST_POWER   = 28493,
    SPELL_EARTHEN_ELIXIR                = 39626,
    SPELL_ELIXIR_OF_MASTERY             = 33726,
    SPELL_ELIXIR_OF_HEALING_POWER       = 28491,
    SPELL_ELIXIR_OF_MAJOR_FORTITUDE     = 39625,
    SPELL_ELIXIR_OF_MAJOR_STRENGTH      = 28490,
    SPELL_ADEPTS_ELIXIR                 = 54452,
    SPELL_ONSLAUGHT_ELIXIR              = 33720,
    SPELL_MIGHTY_TROLLS_BLOOD_ELIXIR    = 24361,
    SPELL_GREATER_ARCANE_ELIXIR         = 17539,
    SPELL_ELIXIR_OF_THE_MONGOOSE        = 17538,
    SPELL_ELIXIR_OF_BRUTE_FORCE         = 17537,
    SPELL_ELIXIR_OF_SAGES               = 17535,
    SPELL_ELIXIR_OF_SUPERIOR_DEFENSE    = 11348,
    SPELL_ELIXIR_OF_DEMONSLAYING        = 11406,
    SPELL_ELIXIR_OF_GREATER_FIREPOWER   = 26276,
    SPELL_ELIXIR_OF_SHADOW_POWER        = 11474,
    SPELL_MAGEBLOOD_ELIXIR              = 24363,
    SPELL_ELIXIR_OF_GIANTS              = 11405,
    SPELL_ELIXIR_OF_GREATER_AGILITY     = 11334,
    SPELL_ARCANE_ELIXIR                 = 11390,
    SPELL_ELIXIR_OF_GREATER_INTELLECT   = 11396,
    SPELL_ELIXIR_OF_GREATER_DEFENSE     = 11349,
    SPELL_ELIXIR_OF_FROST_POWER         = 21920,
    SPELL_ELIXIR_OF_AGILITY             = 11328,
    SPELL_MAJOR_TROLLS_BLLOOD_ELIXIR    =  3223,
    SPELL_ELIXIR_OF_FORTITUDE           =  3593,
    SPELL_ELIXIR_OF_OGRES_STRENGTH      =  3164,
    SPELL_ELIXIR_OF_FIREPOWER           =  7844,
    SPELL_ELIXIR_OF_LESSER_AGILITY      =  3160,
    SPELL_ELIXIR_OF_DEFENSE             =  3220,
    SPELL_STRONG_TROLLS_BLOOD_ELIXIR    =  3222,
    SPELL_ELIXIR_OF_MINOR_ACCURACY      = 63729,
    SPELL_ELIXIR_OF_WISDOM              =  3166,
    SPELL_ELIXIR_OF_GIANTH_GROWTH       =  8212,
    SPELL_ELIXIR_OF_MINOR_AGILITY       =  2374,
    SPELL_ELIXIR_OF_MINOR_FORTITUDE     =  2378,
    SPELL_WEAK_TROLLS_BLOOD_ELIXIR      =  3219,
    SPELL_ELIXIR_OF_LIONS_STRENGTH      =  2367,
    SPELL_ELIXIR_OF_MINOR_DEFENSE       =   673
};

class spell_gen_mixology_bonus : public AuraScript
{
    PrepareAuraScript(spell_gen_mixology_bonus);

    bool Load() override
    {
        return GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void SetBonusValueForEffect(SpellEffIndex effIndex, int32 value, AuraEffect const* aurEff)
    {
        if (aurEff->GetEffIndex() == uint32(effIndex))
            bonus = value;
    }

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (GetCaster()->HasSpell(GetSpellInfo()->Effects[EFFECT_0].TriggerSpell))
        {
            switch (GetId())
            {
                case SPELL_WEAK_TROLLS_BLOOD_ELIXIR:
                case SPELL_MAGEBLOOD_ELIXIR:
                    bonus = amount;
                    break;
                case SPELL_ELIXIR_OF_FROST_POWER:
                case SPELL_LESSER_FLASK_OF_TOUGHNESS:
                case SPELL_LESSER_FLASK_OF_RESISTANCE:
                    bonus = CalculatePct(amount, 80);
                    break;
                case SPELL_ELIXIR_OF_MINOR_DEFENSE:
                case SPELL_ELIXIR_OF_LIONS_STRENGTH:
                case SPELL_ELIXIR_OF_MINOR_AGILITY:
                case SPELL_MAJOR_TROLLS_BLLOOD_ELIXIR:
                case SPELL_ELIXIR_OF_SHADOW_POWER:
                case SPELL_ELIXIR_OF_BRUTE_FORCE:
                case SPELL_MIGHTY_TROLLS_BLOOD_ELIXIR:
                case SPELL_ELIXIR_OF_GREATER_FIREPOWER:
                case SPELL_ONSLAUGHT_ELIXIR:
                case SPELL_EARTHEN_ELIXIR:
                case SPELL_ELIXIR_OF_MAJOR_AGILITY:
                case SPELL_FLASK_OF_THE_TITANS:
                case SPELL_FLASK_OF_RELENTLESS_ASSAULT:
                case SPELL_FLASK_OF_STONEBLOOD:
                case SPELL_ELIXIR_OF_MINOR_ACCURACY:
                    bonus = CalculatePct(amount, 50);
                    break;
                case SPELL_ELIXIR_OF_PROTECTION:
                    bonus = 280;
                    break;
                case SPELL_ELIXIR_OF_MAJOR_DEFENSE:
                    bonus = 200;
                    break;
                case SPELL_ELIXIR_OF_GREATER_DEFENSE:
                case SPELL_ELIXIR_OF_SUPERIOR_DEFENSE:
                    bonus = 140;
                    break;
                case SPELL_ELIXIR_OF_FORTITUDE:
                    bonus = 100;
                    break;
                case SPELL_FLASK_OF_ENDLESS_RAGE:
                    bonus = 82;
                    break;
                case SPELL_ELIXIR_OF_DEFENSE:
                    bonus = 70;
                    break;
                case SPELL_ELIXIR_OF_DEMONSLAYING:
                    bonus = 50;
                    break;
                case SPELL_FLASK_OF_THE_FROST_WYRM:
                    bonus = 47;
                    break;
                case SPELL_WRATH_ELIXIR:
                    bonus = 32;
                    break;
                case SPELL_ELIXIR_OF_MAJOR_FROST_POWER:
                case SPELL_ELIXIR_OF_MAJOR_FIREPOWER:
                case SPELL_ELIXIR_OF_MAJOR_SHADOW_POWER:
                    bonus = 29;
                    break;
                case SPELL_ELIXIR_OF_MIGHTY_TOUGHTS:
                    bonus = 27;
                    break;
                case SPELL_FLASK_OF_SUPREME_POWER:
                case SPELL_FLASK_OF_BLINDING_LIGHT:
                case SPELL_FLASK_OF_PURE_DEATH:
                case SPELL_SHADOWPOWER_ELIXIR:
                    bonus = 23;
                    break;
                case SPELL_ELIXIR_OF_MIGHTY_AGILITY:
                case SPELL_FLASK_OF_DISTILLED_WISDOM:
                case SPELL_ELIXIR_OF_SPIRIT:
                case SPELL_ELIXIR_OF_MIGHTY_STRENGTH:
                case SPELL_FLASK_OF_PURE_MOJO:
                case SPELL_ELIXIR_OF_ACCURACY:
                case SPELL_ELIXIR_OF_DEADLY_STRIKES:
                case SPELL_ELIXIR_OF_MIGHTY_DEFENSE:
                case SPELL_ELIXIR_OF_EXPERTISE:
                case SPELL_ELIXIR_OF_ARMOR_PIERCING:
                case SPELL_ELIXIR_OF_LIGHTNING_SPEED:
                    bonus = 20;
                    break;
                case SPELL_FLASK_OF_CHROMATIC_RESISTANCE:
                    bonus = 17;
                    break;
                case SPELL_ELIXIR_OF_MINOR_FORTITUDE:
                case SPELL_ELIXIR_OF_MAJOR_STRENGTH:
                    bonus = 15;
                    break;
                case SPELL_FLASK_OF_MIGHTY_RESTORATION:
                    bonus = 13;
                    break;
                case SPELL_ARCANE_ELIXIR:
                    bonus = 12;
                    break;
                case SPELL_ELIXIR_OF_GREATER_AGILITY:
                case SPELL_ELIXIR_OF_GIANTS:
                    bonus = 11;
                    break;
                case SPELL_ELIXIR_OF_AGILITY:
                case SPELL_ELIXIR_OF_GREATER_INTELLECT:
                case SPELL_ELIXIR_OF_SAGES:
                case SPELL_ELIXIR_OF_IRONSKIN:
                case SPELL_ELIXIR_OF_MIGHTY_MAGEBLOOD:
                    bonus = 10;
                    break;
                case SPELL_ELIXIR_OF_HEALING_POWER:
                    bonus = 9;
                    break;
                case SPELL_ELIXIR_OF_DRAENIC_WISDOM:
                case SPELL_GURUS_ELIXIR:
                    bonus = 8;
                    break;
                case SPELL_ELIXIR_OF_FIREPOWER:
                case SPELL_ELIXIR_OF_MAJOR_MAGEBLOOD:
                case SPELL_ELIXIR_OF_MASTERY:
                    bonus = 6;
                    break;
                case SPELL_ELIXIR_OF_LESSER_AGILITY:
                case SPELL_ELIXIR_OF_OGRES_STRENGTH:
                case SPELL_ELIXIR_OF_WISDOM:
                case SPELL_ELIXIR_OF_THE_MONGOOSE:
                    bonus = 5;
                    break;
                case SPELL_STRONG_TROLLS_BLOOD_ELIXIR:
                case SPELL_FLASK_OF_CHROMATIC_WONDER:
                    bonus = 4;
                    break;
                case SPELL_ELIXIR_OF_EMPOWERMENT:
                    bonus = -10;
                    break;
                case SPELL_ADEPTS_ELIXIR:
                    SetBonusValueForEffect(EFFECT_0, 13, aurEff);
                    SetBonusValueForEffect(EFFECT_1, 13, aurEff);
                    SetBonusValueForEffect(EFFECT_2, 8, aurEff);
                    break;
                case SPELL_ELIXIR_OF_MIGHTY_FORTITUDE:
                    SetBonusValueForEffect(EFFECT_0, 160, aurEff);
                    break;
                case SPELL_ELIXIR_OF_MAJOR_FORTITUDE:
                    SetBonusValueForEffect(EFFECT_0, 116, aurEff);
                    SetBonusValueForEffect(EFFECT_1, 6, aurEff);
                    break;
                case SPELL_FEL_STRENGTH_ELIXIR:
                    SetBonusValueForEffect(EFFECT_0, 40, aurEff);
                    SetBonusValueForEffect(EFFECT_1, 40, aurEff);
                    break;
                case SPELL_FLASK_OF_FORTIFICATION:
                    SetBonusValueForEffect(EFFECT_0, 210, aurEff);
                    SetBonusValueForEffect(EFFECT_1, 5, aurEff);
                    break;
                case SPELL_GREATER_ARCANE_ELIXIR:
                    SetBonusValueForEffect(EFFECT_0, 19, aurEff);
                    SetBonusValueForEffect(EFFECT_1, 19, aurEff);
                    SetBonusValueForEffect(EFFECT_2, 5, aurEff);
                    break;
                case SPELL_ELIXIR_OF_GIANTH_GROWTH:
                    SetBonusValueForEffect(EFFECT_0, 5, aurEff);
                    break;
                default:
                    TC_LOG_ERROR("spells", "SpellId %u couldn't be processed in spell_gen_mixology_bonus", GetId());
                    break;
            }
            amount += bonus;
        }
    }

    int32 bonus = 0;

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_mixology_bonus::CalculateAmount, EFFECT_ALL, SPELL_AURA_ANY);
    }
};

// 34098 - ClearAllDebuffs
class spell_gen_clear_debuffs : public SpellScript
{
    PrepareSpellScript(spell_gen_clear_debuffs);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
        {
            target->RemoveOwnedAuras([](Aura const* aura)
            {
                SpellInfo const* spellInfo = aura->GetSpellInfo();
                return !spellInfo->IsPositive() && !spellInfo->IsPassive();
            });
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_clear_debuffs::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_generic_spell_scripts()
{
    RegisterAuraScript(spell_gen_absorb0_hitlimit1);
    RegisterAuraScript(spell_gen_adaptive_warding);
    RegisterSpellScript(spell_gen_allow_cast_from_item_only);
    RegisterAuraScript(spell_gen_arena_drink);
    RegisterAuraScript(spell_gen_aura_of_anger);
    RegisterAuraScript(spell_gen_aura_of_fear);
    RegisterAuraScript(spell_gen_av_drekthar_presence);
    RegisterSpellScript(spell_gen_bandage);
    RegisterAuraScript(spell_gen_burn_brutallus);
    RegisterSpellScript(spell_gen_cannibalize);
    RegisterSpellScript(spell_gen_chaos_blast);
    RegisterAuraScript(spell_gen_choking_vines);
    RegisterSpellScript(spell_gen_clear_fear_poly);
    RegisterSpellScript(spell_gen_clone);
    RegisterSpellScript(spell_gen_clone_weapon);
    RegisterAuraScript(spell_gen_clone_weapon_aura);
    new spell_gen_count_pct_from_max_hp("spell_gen_default_count_pct_from_max_hp");
    new spell_gen_count_pct_from_max_hp("spell_gen_50pct_count_pct_from_max_hp", 50);
    RegisterAuraScript(spell_gen_creature_permanent_feign_death);
    new spell_gen_decay_over_time_fungal_decay();
    new spell_gen_decay_over_time_tail_sting();
    RegisterSpellScript(spell_gen_despawn_self);
    RegisterSpellScript(spell_gen_dungeon_credit);
    RegisterSpellScript(spell_gen_elune_candle);
    RegisterSpellScript(spell_gen_gadgetzan_transporter_backfire);
    RegisterAuraScript(spell_gen_gift_of_naaru);
    RegisterSpellScript(spell_gen_gnomish_transporter);
    RegisterAuraScript(spell_gen_lifeblood);
    new spell_gen_lifebloom("spell_hexlord_lifebloom", SPELL_HEXLORD_MALACRASS_LIFEBLOOM_FINAL_HEAL);
    new spell_gen_mount("spell_magic_broom", 0, SPELL_MAGIC_BROOM_60, SPELL_MAGIC_BROOM_100, SPELL_MAGIC_BROOM_150, SPELL_MAGIC_BROOM_280);
    new spell_gen_mount("spell_headless_horseman_mount", 0, SPELL_HEADLESS_HORSEMAN_MOUNT_60, SPELL_HEADLESS_HORSEMAN_MOUNT_100, SPELL_HEADLESS_HORSEMAN_MOUNT_150, SPELL_HEADLESS_HORSEMAN_MOUNT_280);
    RegisterAuraScript(spell_gen_moss_covered_feet);
    RegisterAuraScript(spell_gen_negative_energy_periodic);
    RegisterSpellScript(spell_gen_netherbloom);
    RegisterSpellScript(spell_gen_nightmare_vine);
    RegisterAuraScript(spell_gen_nitrous_boost);
    RegisterAuraScript(spell_gen_obsidian_armor);
    RegisterAuraScript(spell_gen_paralytic_poison);
    new spell_gen_proc_below_pct_damaged("spell_item_commendation_of_kaelthas");
    RegisterAuraScript(spell_gen_proc_charge_drop_only);
    RegisterAuraScript(spell_gen_parachute);
    RegisterSpellScript(spell_gen_pet_summoned);
    RegisterSpellScript(spell_gen_profession_research);
    RegisterSpellScript(spell_gen_remove_flight_auras);
    RegisterAuraScript(spell_gen_restoration);
    RegisterSpellAndAuraScriptPair(spell_gen_replenishment, spell_gen_replenishment_aura);
    RegisterAuraScript(spell_gen_remove_on_health_pct);
    RegisterAuraScript(spell_gen_remove_on_full_health);
    RegisterAuraScript(spell_gen_remove_on_full_health_pct);
    RegisterSpellScript(spell_gen_spirit_healer_res);
    new spell_gen_summon_elemental("spell_gen_summon_fire_elemental", SPELL_SUMMON_FIRE_ELEMENTAL);
    new spell_gen_summon_elemental("spell_gen_summon_earth_elemental", SPELL_SUMMON_EARTH_ELEMENTAL);
    RegisterSpellScript(spell_gen_throw_shield);
    RegisterAuraScript(spell_gen_gm_freeze);
    RegisterSpellScript(spell_gen_stand);
    RegisterAuraScript(spell_gen_mixology_bonus);
    RegisterSpellScript(spell_gen_clear_debuffs);
}
