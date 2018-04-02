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
 * Scripts for spells with SPELLFAMILY_SHAMAN and SPELLFAMILY_GENERIC spells used by shaman players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_sha_".
 */

#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Item.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Unit.h"

enum ShamanSpells
{
    SPELL_SHAMAN_BIND_SIGHT                     = 6277,
    SPELL_SHAMAN_EARTH_SHIELD_HEAL              = 379,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD          = 23552,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE   = 27635,
    SPELL_SHAMAN_ITEM_MANA_SURGE                = 23571,
    SPELL_SHAMAN_LIGHTNING_SHIELD_R1            = 26364,
    SPELL_SHAMAN_MANA_TIDE_TOTEM                = 39609,
    SPELL_SHAMAN_NATURE_GUARDIAN                = 31616,
    SPELL_SHAMAN_NATURE_GUARDIAN_THREAT         = 39301, // Serverside
    SPELL_SHAMAN_STORM_EARTH_AND_FIRE           = 51483,
    SPELL_SHAMAN_TOTEM_EARTHBIND_EARTHGRAB      = 64695,
    SPELL_SHAMAN_TOTEM_EARTHBIND_TOTEM          = 6474,
    SPELL_SHAMAN_TOTEMIC_MASTERY                = 38437,
    SPELL_SHAMAN_TOTEMIC_POWER_MP5              = 28824,
    SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER      = 28825,
    SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER     = 28826,
    SPELL_SHAMAN_TOTEMIC_POWER_ARMOR            = 28827,
    SPELL_SHAMAN_WINDFURY_WEAPON_R1             = 8232,
    SPELL_SHAMAN_WINDFURY_ATTACK_MH             = 25504,
    SPELL_SHAMAN_WINDFURY_ATTACK_OH             = 33750,
    SPELL_SHAMAN_ENERGY_SURGE                   = 40465,
    SPELL_SHAMAN_POWER_SURGE                    = 40466,
    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1     = 45284,
    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1    = 45297,
    SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC          = 30824
};

// -974 - Earth Shield
class spell_sha_earth_shield : public SpellScriptLoader
{
    public:
        spell_sha_earth_shield() : SpellScriptLoader("spell_sha_earth_shield") { }

        class spell_sha_earth_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earth_shield_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_EARTH_SHIELD_HEAL });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();

                CastSpellExtraArgs args(aurEff);
                args.OriginalCaster = GetCasterGUID();
                args.AddSpellBP0(aurEff->GetAmount());
                GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_EARTH_SHIELD_HEAL, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_earth_shield_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_earth_shield_AuraScript();
        }
};

// 6474 - Earthbind Totem - Fix Talent: Earthen Power
class spell_sha_earthbind_totem : public SpellScriptLoader
{
    public:
        spell_sha_earthbind_totem() : SpellScriptLoader("spell_sha_earthbind_totem") { }

        class spell_sha_earthbind_totem_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_earthbind_totem_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_TOTEM_EARTHBIND_TOTEM });
            }

            void Apply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;
                Player* owner = GetCaster()->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (!owner)
                    return;
                // Storm, Earth and Fire
                if (AuraEffect* aurEff = owner->GetAuraEffectOfRankedSpell(SPELL_SHAMAN_STORM_EARTH_AND_FIRE, EFFECT_1))
                {
                    if (roll_chance_i(aurEff->GetAmount()))
                        GetCaster()->CastSpell(GetCaster(), SPELL_SHAMAN_TOTEM_EARTHBIND_EARTHGRAB, false);
                }
            }

            void Register() override
            {
                 OnEffectApply += AuraEffectApplyFn(spell_sha_earthbind_totem_AuraScript::Apply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_earthbind_totem_AuraScript();
        }
};

// -30675 - Lightning Overload
class spell_sha_lightning_overload : public SpellScriptLoader
{
    public:
        spell_sha_lightning_overload() : SpellScriptLoader("spell_sha_lightning_overload") { }

        class spell_sha_lightning_overload_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_lightning_overload_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1,
                    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                if (!spellInfo)
                    return;

                uint32 spellId;

                // Lightning Bolt
                if (spellInfo->SpellFamilyFlags[0] & 0x00000001)
                    spellId = sSpellMgr->GetSpellWithRank(SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1, spellInfo->GetRank());
                // Chain Lightning
                else
                {
                    // Chain lightning has [LightOverload_Proc_Chance] / [Max_Number_of_Targets] chance to proc of each individual target hit.
                    // A maxed LO would have a 33% / 3 = 11% chance to proc of each target.
                    // LO chance was already "accounted" at the proc chance roll, now need to divide the chance by [Max_Number_of_Targets]
                    float chance = 100.0f / spellInfo->Effects[EFFECT_0].ChainTarget;
                    if (!roll_chance_f(chance))
                        return;

                    spellId = sSpellMgr->GetSpellWithRank(SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1, spellInfo->GetRank());
                }

                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_lightning_overload_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_lightning_overload_AuraScript();
        }
};

// 23551 - Lightning Shield T2 Bonus
class spell_sha_item_lightning_shield : public SpellScriptLoader
{
    public:
        spell_sha_item_lightning_shield() : SpellScriptLoader("spell_sha_item_lightning_shield") { }

        class spell_sha_item_lightning_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_item_lightning_shield_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_item_lightning_shield_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_item_lightning_shield_AuraScript();
        }
};

// 23552 - Lightning Shield T2 Bonus
class spell_sha_item_lightning_shield_trigger : public SpellScriptLoader
{
    public:
        spell_sha_item_lightning_shield_trigger() : SpellScriptLoader("spell_sha_item_lightning_shield_trigger") { }

        class spell_sha_item_lightning_shield_trigger_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_item_lightning_shield_trigger_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_item_lightning_shield_trigger_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_item_lightning_shield_trigger_AuraScript();
        }
};

// 23572 - Mana Surge
class spell_sha_item_mana_surge : public SpellScriptLoader
{
    public:
        spell_sha_item_mana_surge() : SpellScriptLoader("spell_sha_item_mana_surge") { }

        class spell_sha_item_mana_surge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_item_mana_surge_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_ITEM_MANA_SURGE });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                if (!spellInfo)
                    return;

                int32 mana = spellInfo->CalcPowerCost(GetTarget(), eventInfo.GetSchoolMask());

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(CalculatePct(mana, 35));
                GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_ITEM_MANA_SURGE, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_item_mana_surge_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_item_mana_surge_AuraScript();
        }
};

// 40463 - Shaman Tier 6 Trinket
class spell_sha_item_t6_trinket : public SpellScriptLoader
{
    public:
        spell_sha_item_t6_trinket() : SpellScriptLoader("spell_sha_item_t6_trinket") { }

        class spell_sha_item_t6_trinket_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_item_t6_trinket_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_ENERGY_SURGE,
                    SPELL_SHAMAN_POWER_SURGE
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

                // Lesser Healing Wave
                if (spellInfo->SpellFamilyFlags[0] & 0x00000080)
                {
                    spellId = SPELL_SHAMAN_ENERGY_SURGE;
                    chance = 10;
                }
                // Lightning Bolt
                else if (spellInfo->SpellFamilyFlags[0] & 0x00000001)
                {
                    spellId = SPELL_SHAMAN_ENERGY_SURGE;
                    chance = 15;
                }
                // Stormstrike
                else if (spellInfo->SpellFamilyFlags[1] & 0x00000010)
                {
                    spellId = SPELL_SHAMAN_POWER_SURGE;
                    chance = 50;
                }
                else
                    return;

                if (roll_chance_i(chance))
                    eventInfo.GetActor()->CastSpell(nullptr, spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_item_t6_trinket_AuraScript();
        }
};

// -324 - Lightning Shield
class spell_sha_lightning_shield : public SpellScriptLoader
{
public:
    spell_sha_lightning_shield() : SpellScriptLoader("spell_sha_lightning_shield") { }

    class spell_sha_lightning_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_lightning_shield_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_SHAMAN_LIGHTNING_SHIELD_R1 });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            if (eventInfo.GetActionTarget())
                return true;
            return false;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            uint32 triggerSpell = sSpellMgr->GetSpellWithRank(SPELL_SHAMAN_LIGHTNING_SHIELD_R1, aurEff->GetSpellInfo()->GetRank());

            eventInfo.GetActionTarget()->CastSpell(eventInfo.GetActor(), triggerSpell, aurEff);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_sha_lightning_shield_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_sha_lightning_shield_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_lightning_shield_AuraScript();
    }
};

// 16191 - Mana Tide
class spell_sha_mana_tide : public AuraScript
{
    PrepareAuraScript(spell_sha_mana_tide);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->Effects[EFFECT_0].TriggerSpell });
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(aurEff->GetAmount());
        GetTarget()->CastSpell(nullptr, GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell, args);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_mana_tide::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 39610 - Mana Tide Totem
class spell_sha_mana_tide_totem : public SpellScriptLoader
{
    public:
        spell_sha_mana_tide_totem() : SpellScriptLoader("spell_sha_mana_tide_totem") { }

        class spell_sha_mana_tide_totem_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sha_mana_tide_totem_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({SPELL_SHAMAN_MANA_TIDE_TOTEM});
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (unitTarget->GetPowerType() == POWER_MANA)
                        {
                            int32 effValue = GetEffectValue();
                            // Regenerate 6% of Total Mana Every 3 secs
                            CastSpellExtraArgs args(GetOriginalCaster()->GetGUID());
                            args.AddSpellBP0(CalculatePct(unitTarget->GetMaxPower(POWER_MANA), effValue));
                            caster->CastSpell(unitTarget, SPELL_SHAMAN_MANA_TIDE_TOTEM, args);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_sha_mana_tide_totem_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_sha_mana_tide_totem_SpellScript();
        }
};

// -30881 - Nature's Guardian
class spell_sha_nature_guardian : public SpellScriptLoader
{
public:
    spell_sha_nature_guardian() : SpellScriptLoader("spell_sha_nature_guardian") { }

    class spell_sha_nature_guardian_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_nature_guardian_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_NATURE_GUARDIAN,
                    SPELL_SHAMAN_NATURE_GUARDIAN_THREAT
                });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            DamageInfo* damageInfo = eventInfo.GetDamageInfo();
            if (!damageInfo || !damageInfo->GetDamage())
                return false;

            int32 healthpct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
            if (Unit* target = eventInfo.GetActionTarget())
                if (target->HealthBelowPctDamaged(healthpct, damageInfo->GetDamage()))
                    return true;

            return false;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* target = eventInfo.GetActionTarget();
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(CalculatePct(target->GetMaxHealth(), aurEff->GetAmount()));
            target->CastSpell(target, SPELL_SHAMAN_NATURE_GUARDIAN, args);
            if (Unit* attacker = eventInfo.GetActor())
                target->CastSpell(attacker, SPELL_SHAMAN_NATURE_GUARDIAN_THREAT, true);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_sha_nature_guardian_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_sha_nature_guardian_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_nature_guardian_AuraScript();
    }
};

// 6495 - Sentry Totem
class spell_sha_sentry_totem : public SpellScriptLoader
{
    public:
        spell_sha_sentry_totem() : SpellScriptLoader("spell_sha_sentry_totem") { }

        class spell_sha_sentry_totem_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_sentry_totem_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_BIND_SIGHT });
            }

            void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Creature* totem = caster->GetMap()->GetCreature(caster->m_SummonSlot[4]))
                        if (totem->IsTotem())
                            caster->CastSpell(totem, SPELL_SHAMAN_BIND_SIGHT, true);
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                        caster->ToPlayer()->StopCastingBindSight();
            }

            void Register() override
            {
                 AfterEffectApply += AuraEffectApplyFn(spell_sha_sentry_totem_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                 AfterEffectRemove += AuraEffectRemoveFn(spell_sha_sentry_totem_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_sentry_totem_AuraScript();
        }
};

// 30823 - Shamanistic Rage
class spell_sha_shamanistic_rage : public SpellScriptLoader
{
    public:
        spell_sha_shamanistic_rage() : SpellScriptLoader("spell_sha_shamanistic_rage") { }

        class spell_sha_shamanistic_rage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_shamanistic_rage_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();

                Unit* target = GetTarget();
                int32 amount = CalculatePct(static_cast<int32>(target->GetTotalAttackPowerValue(BASE_ATTACK)), aurEff->GetAmount());
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                target->CastSpell(target, SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_shamanistic_rage_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_shamanistic_rage_AuraScript();
        }
};

// 38443 - Totemic Mastery (Tier 6 - 2P)
class spell_sha_totemic_mastery : public SpellScriptLoader
{
public:
    spell_sha_totemic_mastery() : SpellScriptLoader("spell_sha_totemic_mastery") { }

    class spell_sha_totemic_mastery_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_totemic_mastery_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_SHAMAN_TOTEMIC_MASTERY });
        }

        void HandleDummy(AuraEffect const* aurEff)
        {
            Unit* target = GetTarget();
            for (uint8 i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
                if (!target->m_SummonSlot[i])
                    return;

            target->CastSpell(target, SPELL_SHAMAN_TOTEMIC_MASTERY, aurEff);
            PreventDefaultAction();
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_totemic_mastery_AuraScript::HandleDummy, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_totemic_mastery_AuraScript();
    }
};

// 28823 - Totemic Power
class spell_sha_t3_6p_bonus : public SpellScriptLoader
{
    public:
        spell_sha_t3_6p_bonus() : SpellScriptLoader("spell_sha_t3_6p_bonus") { }

        class spell_sha_t3_6p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_t3_6p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_TOTEMIC_POWER_ARMOR,
                    SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER,
                    SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER,
                    SPELL_SHAMAN_TOTEMIC_POWER_MP5
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                uint32 spellId;
                Unit* caster = eventInfo.GetActor();
                Unit* target = eventInfo.GetProcTarget();

                switch (target->getClass())
                {
                    case CLASS_PALADIN:
                    case CLASS_PRIEST:
                    case CLASS_SHAMAN:
                    case CLASS_DRUID:
                        spellId = SPELL_SHAMAN_TOTEMIC_POWER_MP5;
                        break;
                    case CLASS_MAGE:
                    case CLASS_WARLOCK:
                        spellId = SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER;
                        break;
                    case CLASS_HUNTER:
                    case CLASS_ROGUE:
                        spellId = SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER;
                        break;
                    case CLASS_WARRIOR:
                        spellId = SPELL_SHAMAN_TOTEMIC_POWER_ARMOR;
                        break;
                    default:
                        return;
                }

                caster->CastSpell(target, spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_sha_t3_6p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_t3_6p_bonus_AuraScript();
        }
};

// 28820 - Lightning Shield
class spell_sha_t3_8p_bonus : public AuraScript
{
    PrepareAuraScript(spell_sha_t3_8p_bonus);

    void PeriodicTick(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        // Need remove self if Lightning Shield not active
        if (!GetTarget()->GetAuraEffectByFamilyFlags(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_SHAMAN, 0x400, 0))
            Remove();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_sha_t3_8p_bonus::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 33757 - Windfury Weapon (Passive)
class spell_sha_windfury_weapon : public SpellScriptLoader
{
    public:
        spell_sha_windfury_weapon() : SpellScriptLoader("spell_sha_windfury_weapon") { }

        class spell_sha_windfury_weapon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sha_windfury_weapon_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_WINDFURY_WEAPON_R1,
                    SPELL_SHAMAN_WINDFURY_ATTACK_MH,
                    SPELL_SHAMAN_WINDFURY_ATTACK_OH
                });
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                Player* player = eventInfo.GetActor()->ToPlayer();
                if (!player)
                    return false;

                Item* item = player->GetItemByGuid(GetAura()->GetCastItemGUID());
                if (!item || !item->IsEquipped())
                    return false;

                WeaponAttackType attType = static_cast<WeaponAttackType>(player->GetAttackBySlot(item->GetSlot()));
                if (attType != BASE_ATTACK && attType != OFF_ATTACK)
                    return false;

                if (((attType == BASE_ATTACK) && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)) ||
                    ((attType == OFF_ATTACK) && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)))
                    return false;

                return true;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Player* player = eventInfo.GetActor()->ToPlayer();

                uint32 spellId = 0;
                WeaponAttackType attType = BASE_ATTACK;
                if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)
                    spellId = SPELL_SHAMAN_WINDFURY_ATTACK_MH;

                if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
                {
                    spellId = SPELL_SHAMAN_WINDFURY_ATTACK_OH;
                    attType = OFF_ATTACK;
                }

                Item* item = ASSERT_NOTNULL(player->GetWeaponForAttack(attType));

                int32 enchantId = static_cast<int32>(item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT));
                int32 extraAttackPower = 0;
                SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_SHAMAN_WINDFURY_WEAPON_R1);
                while (spellInfo)
                {
                    if (spellInfo->Effects[EFFECT_0].MiscValue == enchantId)
                    {
                        extraAttackPower = spellInfo->Effects[EFFECT_1].CalcValue(player);
                        break;
                    }
                    spellInfo = spellInfo->GetNextRankSpell();
                }

                if (!extraAttackPower)
                    return;

                // Value gained from additional AP
                int32 amount = static_cast<int32>(extraAttackPower / 14.f * player->GetAttackTime(attType) / 1000.f);

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                // Attack twice
                for (uint8 i = 0; i < 2; ++i)
                    player->CastSpell(eventInfo.GetProcTarget(), spellId, args);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_sha_windfury_weapon_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_sha_windfury_weapon_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_sha_windfury_weapon_AuraScript();
        }
};

void AddSC_shaman_spell_scripts()
{
    new spell_sha_earth_shield();
    new spell_sha_earthbind_totem();
    new spell_sha_lightning_overload();
    new spell_sha_item_lightning_shield();
    new spell_sha_item_lightning_shield_trigger();
    new spell_sha_item_mana_surge();
    new spell_sha_item_t6_trinket();
    new spell_sha_lightning_shield();
    RegisterAuraScript(spell_sha_mana_tide);
    new spell_sha_mana_tide_totem();
    new spell_sha_nature_guardian();
    new spell_sha_sentry_totem();
    new spell_sha_shamanistic_rage();
    new spell_sha_totemic_mastery();
    new spell_sha_t3_6p_bonus();
    RegisterAuraScript(spell_sha_t3_8p_bonus);
    new spell_sha_windfury_weapon();
}
