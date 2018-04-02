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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum PriestSpells
{
    SPELL_PRIEST_BLESSED_RECOVERY_R1                = 27813,
    SPELL_PRIEST_ITEM_EFFICIENCY                    = 37595,
    SPELL_PRIEST_MANA_LEECH_PROC                    = 34650,
    SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED        = 33619,
    SPELL_PRIEST_REFLECTIVE_SHIELD_R1               = 33201,
    SPELL_PRIEST_SHADOW_WORD_DEATH                  = 32409,
    SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL              = 15290,
    SPELL_PRIEST_DIVINE_BLESSING                    = 40440,
    SPELL_PRIEST_DIVINE_WRATH                       = 40441,
    SPELL_PRIEST_ORACULAR_HEAL                      = 26170,
    SPELL_PRIEST_ARMOR_OF_FAITH                     = 28810
};

enum PriestSpellIcons
{
    PRIEST_ICON_ID_BORROWED_TIME                    = 2899,
    PRIEST_ICON_ID_PAIN_AND_SUFFERING               = 2874
};

class PowerCheck
{
    public:
        explicit PowerCheck(Powers const power) : _power(power) { }

        bool operator()(WorldObject* obj) const
        {
            if (Unit* target = obj->ToUnit())
                return target->GetPowerType() != _power;

            return true;
        }

    private:
        Powers const _power;
};

class RaidCheck
{
    public:
        explicit RaidCheck(Unit const* caster) : _caster(caster) { }

        bool operator()(WorldObject* obj) const
        {
            if (Unit* target = obj->ToUnit())
                return !_caster->IsInRaidWith(target);

            return true;
        }

    private:
        Unit const* _caster;
};

// 26169 - Oracle Healing Bonus
class spell_pri_aq_3p_bonus : public SpellScriptLoader
{
    public:
        spell_pri_aq_3p_bonus() : SpellScriptLoader("spell_pri_aq_3p_bonus") { }

        class spell_pri_aq_3p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_aq_3p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PRIEST_ORACULAR_HEAL });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                Unit* caster = eventInfo.GetActor();
                if (caster == eventInfo.GetProcTarget())
                    return;

                HealInfo* healInfo = eventInfo.GetHealInfo();
                if (!healInfo || !healInfo->GetHeal())
                    return;

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(CalculatePct(healInfo->GetHeal(), 10));
                caster->CastSpell(caster, SPELL_PRIEST_ORACULAR_HEAL, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_aq_3p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_aq_3p_bonus_AuraScript();
        }
};

// -27811 - Blessed Recovery
class spell_pri_blessed_recovery : public SpellScriptLoader
{
public:
    spell_pri_blessed_recovery() : SpellScriptLoader("spell_pri_blessed_recovery") { }

    class spell_pri_blessed_recovery_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_blessed_recovery_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_PRIEST_BLESSED_RECOVERY_R1 });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            DamageInfo* dmgInfo = eventInfo.GetDamageInfo();
            if (!dmgInfo || !dmgInfo->GetDamage())
                return;

            Unit* target = eventInfo.GetActionTarget();
            uint32 triggerSpell = sSpellMgr->GetSpellWithRank(SPELL_PRIEST_BLESSED_RECOVERY_R1, aurEff->GetSpellInfo()->GetRank());
            SpellInfo const* triggerInfo = sSpellMgr->AssertSpellInfo(triggerSpell);

            int32 bp = CalculatePct(static_cast<int32>(dmgInfo->GetDamage()), aurEff->GetAmount());

            ASSERT(triggerInfo->GetMaxTicks() > 0);
            bp /= triggerInfo->GetMaxTicks();

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(bp);
            target->CastSpell(target, triggerSpell, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pri_blessed_recovery_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pri_blessed_recovery_AuraScript();
    }
};

// 40438 - Priest Tier 6 Trinket
class spell_pri_item_t6_trinket : public SpellScriptLoader
{
    public:
        spell_pri_item_t6_trinket() : SpellScriptLoader("spell_pri_item_t6_trinket") { }

        class spell_pri_item_t6_trinket_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_item_t6_trinket_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_PRIEST_DIVINE_BLESSING,
                    SPELL_PRIEST_DIVINE_WRATH
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                Unit* caster = eventInfo.GetActor();
                if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_HEAL)
                    caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_BLESSING, aurEff);

                if (eventInfo.GetSpellTypeMask() & PROC_SPELL_TYPE_DAMAGE)
                    caster->CastSpell(nullptr, SPELL_PRIEST_DIVINE_WRATH, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_item_t6_trinket_AuraScript();
        }
};

// -7001 - Lightwell Renew
class spell_pri_lightwell_renew : public SpellScriptLoader
{
    public:
        spell_pri_lightwell_renew() : SpellScriptLoader("spell_pri_lightwell_renew") { }

        class spell_pri_lightwell_renew_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_lightwell_renew_AuraScript);

            void InitializeAmount(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                // Attacks done to you equal to 30% of your total health will cancel the effect
                _remainingAmount = GetTarget()->CountPctFromMaxHealth(30);
            }

            void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
            {
                DamageInfo* damageInfo = eventInfo.GetDamageInfo();
                if (!damageInfo)
                    return;

                uint32 damage = damageInfo->GetDamage();
                if (_remainingAmount <= damage)
                {
                    DropCharge(AURA_REMOVE_BY_ENEMY_SPELL);
                    return;
                }

                _remainingAmount -= damage;
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_pri_lightwell_renew_AuraScript::InitializeAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectProc += AuraEffectProcFn(spell_pri_lightwell_renew_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }

            uint32 _remainingAmount = 0;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_lightwell_renew_AuraScript();
        }
};

// 28305 - Mana Leech (Passive) (Priest Pet Aura)
class spell_pri_mana_leech : public SpellScriptLoader
{
    public:
        spell_pri_mana_leech() : SpellScriptLoader("spell_pri_mana_leech") { }

        class spell_pri_mana_leech_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_mana_leech_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PRIEST_MANA_LEECH_PROC });
            }

            bool CheckProc(ProcEventInfo& /*eventInfo*/)
            {
                _procTarget = GetTarget()->GetOwner();
                return _procTarget != nullptr;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(_procTarget, SPELL_PRIEST_MANA_LEECH_PROC, aurEff);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_pri_mana_leech_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pri_mana_leech_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }

            Unit* _procTarget = nullptr;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_mana_leech_AuraScript();
        }
};

// -17 - Power Word: Shield
class spell_pri_power_word_shield : public SpellScriptLoader
{
    public:
        spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

        class spell_pri_power_word_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_power_word_shield_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED,
                    SPELL_PRIEST_REFLECTIVE_SHIELD_R1
                });
            }

            void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
            {
                canBeRecalculated = false;
                if (Unit* caster = GetCaster())
                {
                    // +80.68% from sp bonus
                    float bonus = 0.8068f;

                    // Borrowed Time
                    if (AuraEffect const* borrowedTime = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, PRIEST_ICON_ID_BORROWED_TIME, EFFECT_1))
                        bonus += CalculatePct(1.0f, borrowedTime->GetAmount());

                    bonus *= caster->SpellBaseHealingBonusDone(GetSpellInfo()->GetSchoolMask());

                    // Improved PW: Shield: its weird having a SPELLMOD_ALL_EFFECTS here but its blizzards doing :)
                    // Improved PW: Shield is only applied at the spell healing bonus because it was already applied to the base value in CalculateSpellDamage
                    bonus = caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bonus);
                    bonus *= caster->CalculateSpellpowerCoefficientLevelPenalty(GetSpellInfo());

                    amount += int32(bonus);

                    // Twin Disciplines
                    if (AuraEffect const* twinDisciplines = caster->GetAuraEffectByFamilyFlags(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_PRIEST, 0x400000, 0, GetCasterGUID()))
                        AddPct(amount, twinDisciplines->GetAmount());

                    // Focused Power
                    amount *= caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
                }
            }

            void ReflectDamage(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetAttacker() == target)
                    return;

                if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_PRIEST_REFLECTIVE_SHIELD_R1, EFFECT_0))
                {
                    CastSpellExtraArgs args(aurEff);
                    args.AddSpellBP0(CalculatePct(absorbAmount, talentAurEff->GetAmount()));
                    target->CastSpell(dmgInfo.GetAttacker(), SPELL_PRIEST_REFLECTIVE_SHIELD_TRIGGERED, args);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_power_word_shield_AuraScript::ReflectDamage, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_power_word_shield_AuraScript();
        }
};

// -32379 - Shadow Word Death
class spell_pri_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

        class spell_pri_shadow_word_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

            void HandleDamage()
            {
                int32 damage = GetHitDamage();

                // Pain and Suffering reduces damage
                if (AuraEffect* aurEff = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_PRIEST, PRIEST_ICON_ID_PAIN_AND_SUFFERING, EFFECT_1))
                    AddPct(damage, aurEff->GetAmount());

                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.AddSpellBP0(damage);
                GetCaster()->CastSpell(GetCaster(), SPELL_PRIEST_SHADOW_WORD_DEATH, args);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pri_shadow_word_death_SpellScript::HandleDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_shadow_word_death_SpellScript();
        }
};

// 15286 - Vampiric Embrace
class spell_pri_vampiric_embrace : public SpellScriptLoader
{
    public:
        spell_pri_vampiric_embrace() : SpellScriptLoader("spell_pri_vampiric_embrace") { }

        class spell_pri_vampiric_embrace_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_vampiric_embrace_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                DamageInfo* damageInfo = eventInfo.GetDamageInfo();
                if (!damageInfo || !damageInfo->GetDamage())
                    return;

                int32 selfHeal = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());
                int32 partyHeal = selfHeal / 5;
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(partyHeal);
                args.AddSpellMod(SPELLVALUE_BASE_POINT1, selfHeal);
                eventInfo.GetActor()->CastSpell(nullptr, SPELL_PRIEST_VAMPIRIC_EMBRACE_HEAL, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_vampiric_embrace_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_vampiric_embrace_AuraScript();
        }
};

// 28809 - Greater Heal
class spell_pri_t3_4p_bonus : public SpellScriptLoader
{
    public:
        spell_pri_t3_4p_bonus() : SpellScriptLoader("spell_pri_t3_4p_bonus") { }

        class spell_pri_t3_4p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_t3_4p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PRIEST_ARMOR_OF_FAITH });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), SPELL_PRIEST_ARMOR_OF_FAITH, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pri_t3_4p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_t3_4p_bonus_AuraScript();
        }
};

// 37594 - Greater Heal Refund
class spell_pri_t5_heal_2p_bonus : public SpellScriptLoader
{
    public:
        spell_pri_t5_heal_2p_bonus() : SpellScriptLoader("spell_pri_t5_heal_2p_bonus") { }

        class spell_pri_t5_heal_2p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_t5_heal_2p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PRIEST_ITEM_EFFICIENCY });
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                if (HealInfo* healInfo = eventInfo.GetHealInfo())
                    if (Unit* healTarget = healInfo->GetTarget())
                        if (healInfo->GetEffectiveHeal())
                            if (healTarget->GetHealth() >= healTarget->GetMaxHealth())
                                return true;

                return false;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_PRIEST_ITEM_EFFICIENCY, aurEff);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_pri_t5_heal_2p_bonus_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pri_t5_heal_2p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_t5_heal_2p_bonus_AuraScript();
        }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_aq_3p_bonus();
    new spell_pri_blessed_recovery();
    new spell_pri_item_t6_trinket();
    new spell_pri_lightwell_renew();
    new spell_pri_mana_leech();
    new spell_pri_power_word_shield();
    new spell_pri_shadow_word_death();
    new spell_pri_vampiric_embrace();
    new spell_pri_t3_4p_bonus();
    new spell_pri_t5_heal_2p_bonus();
}
