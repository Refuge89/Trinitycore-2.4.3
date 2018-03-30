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
 * Scripts for spells with SPELLFAMILY_DRUID and SPELLFAMILY_GENERIC spells used by druid players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dru_".
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "GameTime.h"
#include "Optional.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum DruidSpells
{
    SPELL_DRUID_BEAR_FORM_PASSIVE           = 1178,
    SPELL_DRUID_DIRE_BEAR_FORM_PASSIVE      = 9635,
    SPELL_DRUID_FORMS_TRINKET_BEAR          = 37340,
    SPELL_DRUID_FORMS_TRINKET_CAT           = 37341,
    SPELL_DRUID_FORMS_TRINKET_MOONKIN       = 37343,
    SPELL_DRUID_FORMS_TRINKET_NONE          = 37344,
    SPELL_DRUID_FORMS_TRINKET_TREE          = 37342,
    SPELL_DRUID_ENRAGE                      = 5229,
    SPELL_DRUID_IDOL_OF_FERAL_SHADOWS       = 34241,
    SPELL_DRUID_LIFEBLOOM_FINAL_HEAL        = 33778,
    SPELL_DRUID_T3_PROC_ENERGIZE_MANA       = 28722,
    SPELL_DRUID_T3_PROC_ENERGIZE_RAGE       = 28723,
    SPELL_DRUID_T3_PROC_ENERGIZE_ENERGY     = 28724,
    SPELL_DRUID_BLESSING_OF_THE_CLAW        = 28750,
    SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL = 34299,
    SPELL_DRUID_EXHILARATE                  = 28742,
    SPELL_DRUID_INFUSION                    = 37238,
    SPELL_DRUID_BLESSING_OF_REMULOS         = 40445,
    SPELL_DRUID_BLESSING_OF_ELUNE           = 40446,
    SPELL_DRUID_BLESSING_OF_CENARIUS        = 40452,
    SPELL_DRUID_FRENZIED_REGENERATION_HEAL  = 22845
};

// 1178 - Bear Form (Passive)
// 9635 - Dire Bear Form (Passive)
class spell_dru_bear_form_passive : public SpellScriptLoader
{
    public:
        spell_dru_bear_form_passive() : SpellScriptLoader("spell_dru_bear_form_passive") { }

        class spell_dru_bear_form_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_bear_form_passive_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_ENRAGE });
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                if (!GetUnitOwner()->HasAura(SPELL_DRUID_ENRAGE))
                    return;

                int32 mod = 0;
                switch (GetId())
                {
                    case SPELL_DRUID_BEAR_FORM_PASSIVE:
                        mod = -27;
                        break;
                    case SPELL_DRUID_DIRE_BEAR_FORM_PASSIVE:
                        mod = -16;
                        break;
                    default:
                        return;
                }
                amount += mod;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_bear_form_passive_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_bear_form_passive_AuraScript();
        }
};

// -1850 - Dash
class spell_dru_dash : public SpellScriptLoader
{
    public:
        spell_dru_dash() : SpellScriptLoader("spell_dru_dash") { }

        class spell_dru_dash_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_dash_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // do not set speed if not in cat form
                if (GetUnitOwner()->GetShapeshiftForm() != FORM_CAT)
                    amount = 0;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_dash_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_SPEED);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_dash_AuraScript();
        }
};

// 37336 - Druid Forms Trinket
class spell_dru_forms_trinket : public SpellScriptLoader
{
public:
    spell_dru_forms_trinket() : SpellScriptLoader("spell_dru_forms_trinket") { }

    class spell_dru_forms_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_forms_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
            {
                SPELL_DRUID_FORMS_TRINKET_BEAR,
                SPELL_DRUID_FORMS_TRINKET_CAT,
                SPELL_DRUID_FORMS_TRINKET_MOONKIN,
                SPELL_DRUID_FORMS_TRINKET_NONE,
                SPELL_DRUID_FORMS_TRINKET_TREE
            });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* target = eventInfo.GetActor();

            switch (target->GetShapeshiftForm())
            {
                case FORM_BEAR:
                case FORM_DIREBEAR:
                case FORM_CAT:
                case FORM_MOONKIN:
                case FORM_NONE:
                case FORM_TREE:
                    return true;
                default:
                    break;
            }

            return false;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            Unit* target = eventInfo.GetActor();
            uint32 triggerspell = 0;

            switch (target->GetShapeshiftForm())
            {
                case FORM_BEAR:
                case FORM_DIREBEAR:
                    triggerspell = SPELL_DRUID_FORMS_TRINKET_BEAR;
                    break;
                case FORM_CAT:
                    triggerspell = SPELL_DRUID_FORMS_TRINKET_CAT;
                    break;
                case FORM_MOONKIN:
                    triggerspell = SPELL_DRUID_FORMS_TRINKET_MOONKIN;
                    break;
                case FORM_NONE:
                    triggerspell = SPELL_DRUID_FORMS_TRINKET_NONE;
                    break;
                case FORM_TREE:
                    triggerspell = SPELL_DRUID_FORMS_TRINKET_TREE;
                    break;
                default:
                    return;
            }

            target->CastSpell(target, triggerspell, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dru_forms_trinket_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_dru_forms_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_forms_trinket_AuraScript();
    }
};

// -33943 - Flight Form
class spell_dru_flight_form : public SpellScriptLoader
{
    public:
        spell_dru_flight_form() : SpellScriptLoader("spell_dru_flight_form") { }

        class spell_dru_flight_form_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_flight_form_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->IsInDisallowedMountForm())
                    return SPELL_FAILED_NOT_SHAPESHIFT;

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_flight_form_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_flight_form_SpellScript();
        }
};

// 22842 - Frenzied Regeneration
class spell_dru_frenzied_regeneration : public AuraScript
{
    PrepareAuraScript(spell_dru_frenzied_regeneration);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DRUID_FRENZIED_REGENERATION_HEAL });
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        // Converts up to 10 rage per second into health for $d.  Each point of rage is converted into ${$m2/10}.1% of max health.
        if (GetTarget()->GetPowerType() != POWER_RAGE)
            return;

        uint32 rage = GetTarget()->GetPower(POWER_RAGE);
        // Nothing to do
        if (!rage)
            return;

        int32 const mod = std::min(static_cast<int32>(rage), 100);
        int32 const regen = CalculatePct(GetTarget()->GetMaxHealth(), GetTarget()->CalculateSpellDamage(GetSpellInfo(), EFFECT_1) * mod / 100.f);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(regen);
        GetTarget()->CastSpell(nullptr, SPELL_DRUID_FRENZIED_REGENERATION_HEAL, args);
        GetTarget()->SetPower(POWER_RAGE, rage - mod);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_frenzied_regeneration::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 34246 - Idol of the Emerald Queen
// 60779 - Idol of Lush Moss
class spell_dru_idol_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_idol_lifebloom() : SpellScriptLoader("spell_dru_idol_lifebloom") { }

        class spell_dru_idol_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_idol_lifebloom_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(GetAura());
                    spellMod->op = SPELLMOD_DOT;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                }
                spellMod->value = aurEff->GetAmount() / 7;
            }

            void Register() override
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_dru_idol_lifebloom_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_idol_lifebloom_AuraScript();
        }
};

// 24932 - Leader of the Pack
class spell_dru_leader_of_the_pack : public SpellScriptLoader
{
    public:
        spell_dru_leader_of_the_pack() : SpellScriptLoader("spell_dru_leader_of_the_pack") { }

        class spell_dru_leader_of_the_pack_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_leader_of_the_pack_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                if (!aurEff->GetAmount())
                    return;

                Unit* caster = eventInfo.GetActor();
                if (caster->GetSpellHistory()->HasCooldown(SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL))
                    return;

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(caster->CountPctFromMaxHealth(aurEff->GetAmount()));
                caster->CastSpell(nullptr, SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL, args);

                // Because of how proc system works, we can't store proc cd on db, it would be applied to entire aura
                // so aura could only proc once per 6 seconds, independently of caster
                caster->GetSpellHistory()->AddCooldown(SPELL_DRUID_IMP_LEADER_OF_THE_PACK_HEAL, 0, Seconds(6));
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_leader_of_the_pack_AuraScript::HandleProc, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_leader_of_the_pack_AuraScript();
        }
};

// -33763 - Lifebloom
class spell_dru_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom() : SpellScriptLoader("spell_dru_lifebloom") { }

        class spell_dru_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lifebloom_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_LIFEBLOOM_FINAL_HEAL });
            }

            void OnRemoveEffect(Unit* target, AuraEffect const* aurEff, uint32 /*stack*/)
            {
                CastSpellExtraArgs args(aurEff);
                args.OriginalCaster = GetCasterGUID();
                args.AddSpellBP0(aurEff->GetAmount());
                target->CastSpell(GetTarget(), SPELL_DRUID_LIFEBLOOM_FINAL_HEAL, args);
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Final heal only on duration end
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                // final heal
                OnRemoveEffect(GetTarget(), aurEff, GetStackAmount());
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* target = GetUnitOwner())
                    if (AuraEffect const* aurEff = GetEffect(EFFECT_1))
                        OnRemoveEffect(target, aurEff, dispelInfo->GetRemovedCharges()); // final heal
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_lifebloom_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterDispel += AuraDispelFn(spell_dru_lifebloom_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_lifebloom_AuraScript();
        }
};

// -16972 - Predatory Strikes
class spell_dru_predatory_strikes : public SpellScriptLoader
{
    public:
        spell_dru_predatory_strikes() : SpellScriptLoader("spell_dru_predatory_strikes") { }

        class spell_dru_predatory_strikes_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_predatory_strikes_AuraScript);

            void UpdateAmount(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* target = GetTarget()->ToPlayer())
                    target->UpdateAttackPowerAndDamage();
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_predatory_strikes_AuraScript::UpdateAmount, EFFECT_ALL, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_predatory_strikes_AuraScript();
        }
};

// -1079 - Rip
class spell_dru_rip : public SpellScriptLoader
{
    public:
        spell_dru_rip() : SpellScriptLoader("spell_dru_rip") { }

        class spell_dru_rip_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rip_AuraScript);

            bool Load() override
            {
                Unit* caster = GetCaster();
                return caster && caster->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;

                if (Unit* caster = GetCaster())
                {
                    // 0.01 * $AP * cp
                    uint8 cp = caster->ToPlayer()->GetComboPoints();

                    // Idol of Feral Shadows. Can't be handled as SpellMod due its dependency from CPs
                    if (AuraEffect const* auraEffIdolOfFeralShadows = caster->GetAuraEffect(SPELL_DRUID_IDOL_OF_FERAL_SHADOWS, EFFECT_0))
                        amount += cp * auraEffIdolOfFeralShadows->GetAmount();

                    amount += int32(CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), cp));
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rip_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_rip_AuraScript();
        }
};

// 40121 - Swift Flight Form (Passive)
class spell_dru_swift_flight_passive : public SpellScriptLoader
{
    public:
        spell_dru_swift_flight_passive() : SpellScriptLoader("spell_dru_swift_flight_passive") { }

        class spell_dru_swift_flight_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swift_flight_passive_AuraScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool & /*canBeRecalculated*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->Has310Flyer(false))
                        amount = 310;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_swift_flight_passive_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_swift_flight_passive_AuraScript();
        }
};

// 28716 - Rejuvenation
class spell_dru_t3_2p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t3_2p_bonus() : SpellScriptLoader("spell_dru_t3_2p_bonus") { }

        class spell_dru_t3_2p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_t3_2p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_DRUID_T3_PROC_ENERGIZE_MANA,
                    SPELL_DRUID_T3_PROC_ENERGIZE_RAGE,
                    SPELL_DRUID_T3_PROC_ENERGIZE_ENERGY
                });
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                if (!roll_chance_i(50))
                    return false;
                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                Unit* target = eventInfo.GetProcTarget();
                uint32 spellId;

                switch (target->GetPowerType())
                {
                    case POWER_MANA:
                        spellId = SPELL_DRUID_T3_PROC_ENERGIZE_MANA;
                        break;
                    case POWER_RAGE:
                        spellId = SPELL_DRUID_T3_PROC_ENERGIZE_RAGE;
                        break;
                    case POWER_ENERGY:
                        spellId = SPELL_DRUID_T3_PROC_ENERGIZE_ENERGY;
                        break;
                    default:
                        return;
                }

                eventInfo.GetActor()->CastSpell(target, spellId, aurEff);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_dru_t3_2p_bonus_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_dru_t3_2p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_t3_2p_bonus_AuraScript();
        }
};

// 28744 - Regrowth
class spell_dru_t3_6p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t3_6p_bonus() : SpellScriptLoader("spell_dru_t3_6p_bonus") { }

        class spell_dru_t3_6p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_t3_6p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_BLESSING_OF_THE_CLAW });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), SPELL_DRUID_BLESSING_OF_THE_CLAW, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_t3_6p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_t3_6p_bonus_AuraScript();
        }
};

// 28719 - Healing Touch
class spell_dru_t3_8p_bonus : public SpellScriptLoader
{
        public:
            spell_dru_t3_8p_bonus() : SpellScriptLoader("spell_dru_t3_8p_bonus") { }

        class spell_dru_t3_8p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_t3_8p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_EXHILARATE });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                if (!spellInfo)
                    return;

                Unit* caster = eventInfo.GetActor();
                int32 amount = CalculatePct(spellInfo->CalcPowerCost(caster, spellInfo->GetSchoolMask()), aurEff->GetAmount());
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                caster->CastSpell(nullptr, SPELL_DRUID_EXHILARATE, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_t3_8p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_t3_8p_bonus_AuraScript();
        }
};

// 37288 - Mana Restore
// 37295 - Mana Restore
class spell_dru_t4_2p_bonus : public SpellScriptLoader
{
    public:
        spell_dru_t4_2p_bonus() : SpellScriptLoader("spell_dru_t4_2p_bonus") { }

        class spell_dru_t4_2p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_t4_2p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_DRUID_INFUSION });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                eventInfo.GetActor()->CastSpell(nullptr, SPELL_DRUID_INFUSION, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_t4_2p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_t4_2p_bonus_AuraScript();
        }
};

// 40442 - Druid Tier 6 Trinket
class spell_dru_item_t6_trinket : public SpellScriptLoader
{
    public:
        spell_dru_item_t6_trinket() : SpellScriptLoader("spell_dru_item_t6_trinket") { }

        class spell_dru_item_t6_trinket_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_item_t6_trinket_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({
                    SPELL_DRUID_BLESSING_OF_REMULOS,
                    SPELL_DRUID_BLESSING_OF_ELUNE,
                    SPELL_DRUID_BLESSING_OF_CENARIUS
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                if (!spellInfo)
                    return;

                uint32 spellId;
                int32 chance;

                // Starfire
                if (spellInfo->SpellFamilyFlags[0] & 0x00000004)
                {
                    spellId = SPELL_DRUID_BLESSING_OF_REMULOS;
                    chance = 25;
                }
                // Rejuvenation
                else if (spellInfo->SpellFamilyFlags[0] & 0x00000010)
                {
                    spellId = SPELL_DRUID_BLESSING_OF_ELUNE;
                    chance = 25;
                }
                // Mangle (Bear) and Mangle (Cat)
                else if (spellInfo->SpellFamilyFlags[1] & 0x00000440)
                {
                    spellId = SPELL_DRUID_BLESSING_OF_CENARIUS;
                    chance = 40;
                }
                else
                    return;

                if (roll_chance_i(chance))
                    eventInfo.GetActor()->CastSpell(nullptr, spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_dru_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_item_t6_trinket_AuraScript();
        }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_bear_form_passive();
    new spell_dru_dash();
    new spell_dru_forms_trinket();
    new spell_dru_flight_form();
    RegisterAuraScript(spell_dru_frenzied_regeneration);
    new spell_dru_idol_lifebloom();
    new spell_dru_leader_of_the_pack();
    new spell_dru_lifebloom();
    new spell_dru_predatory_strikes();
    new spell_dru_rip();
    new spell_dru_swift_flight_passive();
    new spell_dru_t3_2p_bonus();
    new spell_dru_t3_6p_bonus();
    new spell_dru_t3_8p_bonus();
    new spell_dru_t4_2p_bonus();
    new spell_dru_item_t6_trinket();
}
