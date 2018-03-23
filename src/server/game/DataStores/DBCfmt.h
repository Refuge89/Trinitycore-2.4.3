/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_DBCSFRM_H
#define TRINITY_DBCSFRM_H

char const AreaTableEntryfmt[] = "niiiixxxxxissssssssssssssssxiixxxxx";
char const AreaPOIEntryfmt[] = "nxixfffixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char const AreaTriggerEntryfmt[] = "niffffffff";
char const AuctionHouseEntryfmt[] = "niiixxxxxxxxxxxxxxxxx";
char const BankBagSlotPricesEntryfmt[] = "ni";
char const BattlemasterListEntryfmt[] = "niiiiiiiiixxixxssssssssssssssssxx";
char const CharacterFacialHairStylesfmt[] = "iiixxxxxxxx";
char const CharSectionsEntryfmt[] = "diiiiixxxi";
char const CharStartOutfitEntryfmt[] = "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx";
char const CharTitlesEntryfmt[] = "nxssssssssssssssssxssssssssssssssssxi";
char const ChatChannelsEntryfmt[] = "nixssssssssssssssssxxxxxxxxxxxxxxxxxx";
char const ChrClassesEntryfmt[] = "nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix";
char const ChrRacesEntryfmt[] = "niixiixxixxxxissssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
char const CinematicCameraEntryfmt[] = "nsiffff";
char const CinematicSequencesEntryfmt[] = "nxixxxxxxx";
char const CreatureDisplayInfofmt[] = "nixifxxxxxxxxx";
char const CreatureDisplayInfoExtrafmt[] = "diixxxxxxxxxxxxxxxxxx";
char const CreatureFamilyfmt[] = "nfifiiiissssssssssssssssxx";
char const CreatureModelDatafmt[] = "nisxfxxxxxxxxxxffxxxxxxx";
char const CreatureSpellDatafmt[] = "niiiixxxx";
char const CreatureTypefmt[] = "nxxxxxxxxxxxxxxxxxx";
char const DurabilityCostsfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
char const DurabilityQualityfmt[] = "nf";
char const EmotesEntryfmt[] = "nxxiiix";
char const EmotesTextEntryfmt[] = "nxixxxxxxxxxxxxxxxx";
char const EmotesTextSoundEntryfmt[] = "niiii";
char const FactionEntryfmt[] = "niiiiiiiiiiiiiiiiiissssssssssssssssxxxxxxxxxxxxxxxxxx";
char const FactionTemplateEntryfmt[] = "niiiiiiiiiiiii";
char const GameObjectDisplayInfofmt[] = "nsxxxxxxxxxxffffff";
char const GemPropertiesEntryfmt[] = "nixxi";
char const GtCombatRatingsfmt[] = "f";
char const GtChanceToMeleeCritBasefmt[] = "f";
char const GtChanceToMeleeCritfmt[] = "f";
char const GtChanceToSpellCritBasefmt[] = "f";
char const GtChanceToSpellCritfmt[] = "f";
char const GtNPCManaCostScalerfmt[] = "f";
char const GtOCTRegenHPfmt[] = "f";
//char const GtOCTRegenMPfmt[] = "f";
char const GtRegenHPPerSptfmt[] = "f";
char const GtRegenMPPerSptfmt[] = "f";
char const Holidaysfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiixxsiix";
char const Itemfmt[] = "niii";
char const ItemBagFamilyfmt[] = "nxxxxxxxxxxxxxxxxx";
//char const ItemDisplayTemplateEntryfmt[] = "nxxxxxxxxxxixxxxxxxxxxx";
//char const ItemCondExtCostsEntryfmt[] = "xiii";
char const ItemExtendedCostEntryfmt[] = "niiiiiiiiiiiii";
char const ItemRandomPropertiesfmt[] = "nxiiixxssssssssssssssssx";
char const ItemRandomSuffixfmt[] = "nssssssssssssssssxxiiiiii";
char const ItemSetEntryfmt[] = "dssssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii";
char const LFGDungeonEntryfmt[] = "nssssssssssssssssxiiixxi";
char const LightEntryfmt[] = "nifffxxxxxxx";
char const LiquidTypefmt[] = "nxii";
char const LockEntryfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
char const MailTemplateEntryfmt[] = "nxxxxxxxxxxxxxxxxxssssssssssssssssx";
char const MapEntryfmt[] = "nxixssssssssssssssssxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiffiixxi";
char const NamesProfanityEntryfmt[] = "dsi";
char const NamesReservedEntryfmt[] = "dsi";
char const QuestSortEntryfmt[] = "nxxxxxxxxxxxxxxxxx";
char const RandomPropertiesPointsfmt[] = "niiiiiiiiiiiiiii";
char const SkillLinefmt[] = "nixssssssssssssssssxxxxxxxxxxxxxxxxxxi";
char const SkillLineAbilityfmt[] = "niiiixxiiiiixxi";
char const SkillRaceClassInfofmt[] = "diiiixix";
char const SkillTiersfmt[] = "nxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiii";
char const SoundEntriesfmt[] = "nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const SpellCastTimefmt[] = "nixx";
char const SpellCategoryfmt[] = "ni";
char const SpellDifficultyfmt[] = "niiii";
const std::string CustomSpellDifficultyfmt = "ppppp";
const std::string CustomSpellDifficultyIndex = "id";
char const SpellDurationfmt[] = "niii";
char const SpellEntryfmt[] =  "nixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiissssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxxiiii";
const std::string CustomSpellEntryfmt = "paappppppppppppaaaaaaapaaapapppppppaaaaapaapaaaaaaaaaaaaaaaaaapppppppppaaaaaapppppppppppppppppppppppppppaaappppppppppppaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaapppppppapppaaaaapp";
const std::string CustomSpellEntryIndex = "Id";
char const SpellFocusObjectfmt[] = "nxxxxxxxxxxxxxxxxx";
char const SpellItemEnchantmentfmt[] = "niiiiiixxxiiissssssssssssssssxiiii";
char const SpellItemEnchantmentConditionfmt[] = "nbbbbbxxxxxbbbbbbbbbbiiiiixxxxx";
char const SpellRadiusfmt[] = "nfff";
char const SpellRangefmt[] = "nffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char const SpellShapeshiftfmt[] = "nxxxxxxxxxxxxxxxxxxiixiiixxiiiiiiii";
char const StableSlotPricesfmt[] = "ni";
char const SummonPropertiesfmt[] = "niiiii";
char const TalentEntryfmt[] = "niiiiiiiixxxxixxixxxi";
char const TalentTabEntryfmt[] = "nxxxxxxxxxxxxxxxxxxxiix";
char const TaxiNodesEntryfmt[] = "nifffssssssssssssssssxii";
char const TaxiPathEntryfmt[] = "niii";
char const TaxiPathNodeEntryfmt[] = "diiifffiiii";
char const TotemCategoryEntryfmt[] = "nxxxxxxxxxxxxxxxxxii";
char const TransportAnimationfmt[] = "diifffx";
char const WMOAreaTableEntryfmt[] = "niiixxxxxiixxxxxxxxxxxxxxxxx";
char const WorldMapAreaEntryfmt[] = "xinxffffi";
char const WorldMapOverlayEntryfmt[] = "nxiiiixxxxxxxxxxx";
char const WorldSafeLocsEntryfmt[] = "nifffxxxxxxxxxxxxxxxxx";

#endif
