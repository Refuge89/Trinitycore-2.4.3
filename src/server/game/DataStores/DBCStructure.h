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

#ifndef TRINITY_DBCSTRUCTURE_H
#define TRINITY_DBCSTRUCTURE_H

#include "Define.h"
#include "DBCEnums.h"
#include "SharedDefines.h"
#include "Util.h"
#include <set>
#include <map>

// Structures using to access raw DBC data and required packing to portability
#pragma pack(push, 1)

struct AreaTableEntry
{
    uint32  ID;                                             // 0
    uint32  mapid;                                          // 1
    uint32  zone;                                           // 2 if 0 then it's zone, else it's zone id of this area
    uint32  exploreFlag;                                    // 3
    uint32  flags;                                          // 4, unknown value but 312 for all cities
                                                            // 5-9 unused
    int32   area_level;                                     // 10
    char*   area_name[16];                                  // 11-26
                                                            // 27, string flags, unused
    uint32  team;                                           // 28
    uint32  LiquidTypeOverride;                             // 29 liquid override by type
                                                            // 30-36 unused

    // helpers
    bool IsSanctuary() const
    {
        return (flags & AREA_FLAG_SANCTUARY) != 0;
    }

    bool IsFlyable() const
    {
        if (flags & AREA_FLAG_OUTLAND)
            if (!(flags & AREA_FLAG_NO_FLY_ZONE))
                return true;

        return false;
    }
};

struct AreaPOIEntry
{
    uint32 id;              //0
    //uint32 importance;    //1
    uint32 icon;            //2
    //uint32 factionId      //3
    float x;                //4
    float y;                //5
    float z;                //6
    uint32 mapId;           //7
    //uint32 val1;          //8
    uint32 zoneId;          //9
    //char* name[16];       //10-25
    //uint32 name_flag;     //26
    //char* name2[16];      //27-42
    //uint32 name_flag2;    //43
    uint32 worldState;      //44
};

struct AreaTriggerEntry
{
    uint32  id;                                             // 0        m_ID
    uint32  mapid;                                          // 1        m_ContinentID
    float   x;                                              // 2        m_x
    float   y;                                              // 3        m_y
    float   z;                                              // 4        m_z
    float   radius;                                         // 5        m_radius
    float   box_x;                                          // 6        m_box_length
    float   box_y;                                          // 7        m_box_width
    float   box_z;                                          // 8        m_box_heigh
    float   box_orientation;                                // 9        m_box_yaw
};

struct AuctionHouseEntry
{
    uint32    houseId;                                      // 0 index
    uint32    faction;                                      // 1 id of faction.dbc for player factions associated with city
    uint32    depositPercent;                               // 2 1/3 from real
    uint32    cutPercent;                                   // 3
    //char*     name[16];                                   // 4-19
                                                            // 20 string flag, unused
};

struct BankBagSlotPricesEntry
{
    uint32  ID;
    uint32  price;
};

struct BattlemasterListEntry
{
    uint32  id;                                             // 0
    int32   mapid[8];                                       // 1-8 mapid
    uint32  type;                                           // 9 map type (3 - BG, 4 - arena)
    //uint32 MinLevel;                                      // 10
    //uint32 SomeLevel;                                     // 11, may be max level all 0x46(70)
    uint32 maxGroupSize;                                    // 12 maxGroupSize, used for checking if queue as group
    //uint32 unkown                                         // 13
    //uint32 canJoinAsGroup;                                // 14 (0 or 1)
    char*   name[16];                                       // 15-30
    //uint32 nameFlags                                      // 31 string flag, unused
    //uint32 unkown                                         // 32
};

struct CharacterFacialHairStylesEntry
{
    uint32 Race;
    uint32 Gender;
    uint32 Variation;
    // uint32 Geoset[5];
};

enum CharSectionFlags
{
    SECTION_FLAG_PLAYER       = 0x01
};

enum CharSectionType
{
    SECTION_TYPE_SKIN         = 0,
    SECTION_TYPE_FACE         = 1,
    SECTION_TYPE_FACIAL_HAIR  = 2,
    SECTION_TYPE_HAIR         = 3,
    SECTION_TYPE_UNDERWEAR    = 4
};

struct CharSectionsEntry
{
    //uint32 Id;
    uint32 Race;
    uint32 Gender;
    uint32 GenType;
    uint32 Type;
    uint32 Color;
    //char* TexturePath[3];
    uint32 Flags;

    inline bool HasFlag(CharSectionFlags flag) const { return !!(Flags & flag); }
};

#define MAX_OUTFIT_ITEMS 12

struct CharStartOutfitEntry
{
    //uint32 Id;                                            // 0
    uint8 Race;                                             // 1
    uint8 Class;                                            // 2
    uint8 Gender;                                           // 3
    //uint8 Unused;                                         // 4
    int32 ItemId[MAX_OUTFIT_ITEMS];                         // 5-28
    //int32 ItemDisplayId[MAX_OUTFIT_ITEMS];                // 29-52 not required at server side
    //int32 ItemInventorySlot[MAX_OUTFIT_ITEMS];            // 53-76 not required at server side
};

struct CharTitlesEntry
{
    uint32  ID;                                             // 0, title ids, for example in Quest::GetCharTitleId()
    //uint32      unk1;                                     // 1 flags?
    char*   nameMale[16];                                   // 2-17
                                                            // 18 string flag, unused
    char*   nameFemale[16];                                 // 19-34
                                                            // 35 string flag, unused
    uint32  bit_index;                                      // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER_FIELD_KNOWN_TITLES
};

struct ChatChannelsEntry
{
    uint32  ChannelID;                                      // 0
    uint32  flags;                                          // 1
    char*   pattern[16];                                    // 3-18
                                                            // 19 string flags, unused
    //char*       name[16];                                 // 20-35 unused
                                                            // 36 string flag, unused
};

struct ChrClassesEntry
{
    uint32  ClassID;                                        // 0
                                                            // 1, unused
    uint32  powerType;                                      // 2
                                                            // 3-4, unused
    char*   name[16];                                       // 5-20 unused
                                                            // 21 string flag, unused
    //char*       nameFemale[16];                           // 21-36 unused, if different from base (male) case
                                                            // 37 string flag, unused
    //char*       nameNeutralGender[16];                    // 38-53 unused, if different from base (male) case
                                                            // 54 string flag, unused
                                                            // 55, unused
    uint32  spellfamily;                                    // 56
                                                            // 57, unused
};

struct ChrRacesEntry
{
    uint32      RaceID;                                     // 0
    uint32      Flags;                                      // 1
    uint32      FactionID;                                  // 2 facton template id
                                                            // 3 unused
    uint32      model_m;                                    // 4
    uint32      model_f;                                    // 5
                                                            // 6 unused
                                                            // 7 unused
    uint32      TeamID;                                     // 8 (7-Alliance 1-Horde)
                                                            // 9-12 unused
    uint32      CinematicSequence;                          // 13 id from CinematicSequences.dbc
    char*       name[16];                                   // 14-29 used for DBC language detection/selection
                                                            // 30 string flags, unused
    //char*       nameFemale[16];                           // 31-46, if different from base (male) case
                                                            // 47 string flags, unused
    //char*       nameNeutralGender[16];                    // 48-63, if different from base (male) case
                                                            // 64 string flags, unused
                                                            // 65-67 unused
    uint32      expansion;                                  // 68 (0 - original race, 1 - tbc addon, ...)
};

struct CinematicCameraEntry
{
    uint32 ID;                                              // 0
    char const* Model;                                      // 1    Model filename (translate .mdx to .m2)
    uint32 SoundID;                                         // 2    Sound ID       (voiceover for cinematic)
    DBCPosition3D Origin;                                   // 3-5  Position in map used for basis for M2 co-ordinates
    float OriginFacing;                                     // 6    Orientation in map used for basis for M2 co-ordinates
};

struct CinematicSequencesEntry
{
    uint32      Id;                                         // 0 index
    //uint32      unk1;                                     // 1 always 0
    uint32      cinematicCamera;                            // 2 id in CinematicCamera.dbc
                                                            // 3-9 always 0
};

struct CreatureDisplayInfoEntry
{
    uint32      Displayid;                                  // 0        m_ID
    uint32      ModelId;                                    // 1        m_modelID
                                                            // 2        m_soundID
    uint32      ExtraId;                                    // 3        m_extendedDisplayInfoID
    float       scale;                                      // 4        m_creatureModelScale
                                                            // 5        m_creatureModelAlpha
                                                            // 6-8      m_textureVariation[3]
                                                            // 9        m_portraitTextureName
                                                            // 10       m_sizeClass
                                                            // 11       m_bloodID
                                                            // 12       m_NPCSoundID
                                                            // 13       m_particleColorID
};

struct CreatureDisplayInfoExtraEntry
{
    //uint32 Id;                                            // 0
    uint32 Race;                                            // 1
    uint32 Gender;                                          // 2
    //uint32 SkinColor;                                     // 3
    //uint32 FaceType;                                      // 4
    //uint32 HairType;                                      // 5
    //uint32 HairStyle;                                     // 6
    //uint32 FacialHair;                                    // 7
    //uint32 HelmDisplayId;                                 // 8
    //uint32 ShoulderDisplayId;                             // 9
    //uint32 ShirtDisplayId;                                // 10
    //uint32 ChestDisplayId;                                // 11
    //uint32 BeltDisplayId;                                 // 12
    //uint32 LegsDisplayId;                                 // 13
    //uint32 BootsDisplayId;                                // 14
    //uint32 WristDisplayId;                                // 15
    //uint32 GlovesDisplayId;                               // 16
    //uint32 TabardDisplayId;                               // 17
    //uint32 CloakDisplayId;                                // 18
    //uint32 CanEquip;                                      // 19
    //char const* Texture;                                  // 20
};

struct CreatureFamilyEntry
{
    uint32  ID;                                             // 0        m_ID
    float   minScale;                                       // 1        m_minScale
    uint32  minScaleLevel;                                  // 2        m_minScaleLevel
    float   maxScale;                                       // 3        m_maxScale
    uint32  maxScaleLevel;                                  // 4        m_maxScaleLevel
    uint32  skillLine[2];                                   // 5-6      m_skillLine
    uint32  petFoodMask;                                    // 7        m_petFoodMask
    char*   Name[16];                                       // 8-23     m_name_lang
                                                            // 24 string flags
                                                            // 25       m_iconFile
};

struct CreatureModelDataEntry
{
    uint32 Id;
    uint32 Flags;
    char* ModelPath;
    //uint32 Unk1;
    float Scale;                                             // Used in calculation of unit collision data
    //int32 Unk2
    //int32 Unk3
    //uint32 Unk4
    //uint32 Unk5
    //float Unk6
    //uint32 Unk7
    //float Unk8
    //uint32 Unk9
    //uint32 Unk10
    //float CollisionWidth;
    float CollisionHeight;
    float MountHeight;                                       // Used in calculation of unit collision data when mounted
    //float Unks[11]
};

struct CreatureSpellDataEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    spellId[MAX_CREATURE_SPELL_DATA_SLOT];        // 1-4      m_spells[4]
    //uint32    availability[MAX_CREATURE_SPELL_DATA_SLOT]; // 4-7      m_availability[4]
};

struct CreatureTypeEntry
{
    uint32    ID;                                           // 0        m_ID
    //char*   Name[16];                                     // 1-16     name
                                                            // 17       string flags
    //uint32    no_expirience;                              // 18 no exp? critters, non-combat pets, gas cloud.
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                      // 0
    uint32    multiplier[29];                               // 1-29
};

struct DurabilityQualityEntry
{
    uint32    Id;                                           // 0
    float     quality_mod;                                  // 1
};

struct EmotesEntry
{
    uint32  Id;                                             // 0
    //char*   Name;                                         // 1, internal name
    //uint32  AnimationId;                                  // 2, ref to animationData
    uint32  Flags;                                          // 3, bitmask, may be unit_flags
    uint32  EmoteType;                                      // 4, Can be 0, 1 or 2 (determine how emote are shown)
    uint32  UnitStandState;                                 // 5, uncomfirmed, may be enum UnitStandStateType
    //uint32  SoundId;                                      // 6, ref to soundEntries
};

struct EmotesTextEntry
{
    uint32  Id;
    uint32  textid;
};

struct EmotesTextSoundEntry
{
    uint32 Id;                                              // 0
    uint32 EmotesTextId;                                    // 1
    uint32 RaceId;                                          // 2
    uint32 SexId;                                           // 3, 0 male / 1 female
    uint32 SoundId;                                         // 4
};

struct FactionEntry
{
    uint32      ID;                                         // 0        m_ID
    int32       reputationListID;                           // 1        m_reputationIndex
    uint32      BaseRepRaceMask[4];                         // 2-5      m_reputationRaceMask
    uint32      BaseRepClassMask[4];                        // 6-9      m_reputationClassMask
    int32       BaseRepValue[4];                            // 10-13    m_reputationBase
    uint32      ReputationFlags[4];                         // 14-17    m_reputationFlags
    uint32      team;                                       // 18       m_parentFactionID
    char*       name[16];                                   // 19-34    m_name_lang
                                                            // 35 string flags
    //char*     description[16];                            // 36-51    m_description_lang
                                                            // 52 string flags

    // helpers
    bool CanHaveReputation() const
    {
        return reputationListID >= 0;
    }
};

#define MAX_FACTION_RELATIONS 4

struct FactionTemplateEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      faction;                                    // 1        m_faction
    uint32      factionFlags;                               // 2        m_flags
    uint32      ourMask;                                    // 3        m_factionGroup
    uint32      friendlyMask;                               // 4        m_friendGroup
    uint32      hostileMask;                                // 5        m_enemyGroup
    uint32      enemyFaction[MAX_FACTION_RELATIONS];        // 6        m_enemies[MAX_FACTION_RELATIONS]
    uint32      friendFaction[MAX_FACTION_RELATIONS];       // 10       m_friend[MAX_FACTION_RELATIONS]
    //-------------------------------------------------------  end structure

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if (entry.faction)
        {
            for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                if (enemyFaction[i] == entry.faction)
                    return false;
            for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                if (friendFaction[i] == entry.faction)
                    return true;
        }
        return (friendlyMask & entry.ourMask) || (ourMask & entry.friendlyMask);
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if (entry.faction)
        {
            for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                if (enemyFaction[i] == entry.faction)
                    return true;
            for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                if (friendFaction[i] == entry.faction)
                    return false;
        }
        return (hostileMask & entry.ourMask) != 0;
    }
    bool IsHostileToPlayers() const { return (hostileMask & FACTION_MASK_PLAYER) !=0; }
    bool IsNeutralToAll() const
    {
        for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
            if (enemyFaction[i] != 0)
                return false;
        return hostileMask == 0 && friendlyMask == 0;
    }
    bool IsContestedGuardFaction() const { return (factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD) != 0; }
};

struct GameObjectDisplayInfoEntry
{
    uint32      Displayid;                               // 0        m_ID
    char* filename;                                      // 1
    //uint32  unk1[10];                                  //2-11
    float   minX;
    float   minY;
    float   minZ;
    float   maxX;
    float   maxY;
    float   maxZ;
};

struct GemPropertiesEntry
{
    uint32      ID;
    uint32      spellitemenchantement;
    uint32      color;
};

// All Gt* DBC store data for 100 levels, some by 100 per class/race
#define GT_MAX_LEVEL    100
#define GT_MAX_RATING   32

struct GtCombatRatingsEntry
{
    float    ratio;
};

struct GtChanceToMeleeCritBaseEntry
{
    float    base;
};

struct GtChanceToMeleeCritEntry
{
    float    ratio;
};

struct GtChanceToSpellCritBaseEntry
{
    float    base;
};

struct GtNPCManaCostScalerEntry
{
    float    ratio;
};

struct GtChanceToSpellCritEntry
{
    float    ratio;
};

struct GtOCTRegenHPEntry
{
    float    ratio;
};

//struct GtOCTRegenMPEntry
//{
//    float    ratio;
//};

struct GtRegenHPPerSptEntry
{
    float    ratio;
};

struct GtRegenMPPerSptEntry
{
    float    ratio;
};

struct ItemEntry
{
   uint32   ID;                                             // 0
   uint32   DisplayId;                                      // 1
   uint32   InventoryType;                                  // 2
   uint32   Sheath;                                         // 3
};

struct ItemBagFamilyEntry
{
    uint32   ID;                                            // 0
    //char*     name[16]                                    // 1-16     m_name_lang
    //                                                      // 17       name flags
};

struct ItemDisplayInfoEntry
{
    uint32      ID;                                         // 0        m_ID
                                                            // 1        m_modelName[2]
                                                            // 2        m_modelTexture[2]
                                                            // 3        m_inventoryIcon
                                                            // 4        m_geosetGroup[3]
                                                            // 5        m_flags
                                                            // 6        m_spellVisualID
                                                            // 7        m_groupSoundIndex
                                                            // 8        m_helmetGeosetVis[2]
                                                            // 9        m_texture[2]
                                                            // 10       m_itemVisual[8]
                                                            // 11       m_particleColorID
};

//struct ItemCondExtCostsEntry
//{
//    uint32      ID;
//    uint32      condExtendedCost;                         // ItemTemplate::CondExtendedCost
//    uint32      itemextendedcostentry;                    // ItemTemplate::ExtendedCost
//    uint32      arenaseason;                              // arena season number(1-4)
//};

#define MAX_ITEM_EXTENDED_COST_REQUIREMENTS 5

struct ItemExtendedCostEntry
{
    uint32      ID;                                                 // 0 extended-cost entry id
    uint32      reqhonorpoints;                                     // 1 required honor points
    uint32      reqarenapoints;                                     // 2 required arena points
    uint32      reqitem[MAX_ITEM_EXTENDED_COST_REQUIREMENTS];       // 3-7 required item id
    uint32      reqitemcount[MAX_ITEM_EXTENDED_COST_REQUIREMENTS];  // 8-12 required count of 1st item
    uint32      reqpersonalarenarating;                             // 13 required personal arena rating};
};

#define MAX_ITEM_ENCHANTMENT_EFFECTS 3

struct ItemRandomPropertiesEntry
{
    uint32    ID;                                           // 0        m_ID
    //char*     internalName                                // 1        m_Name
    uint32    enchant_id[MAX_ITEM_ENCHANTMENT_EFFECTS];     // 2-4      m_Enchantment
                                                            // 5-6      unused
    char*     nameSuffix[16];                               // 7-22     m_name_lang
                                                            // 23 name flags
};

struct ItemRandomSuffixEntry
{
    uint32    ID;                                           // 0        m_ID
    char*     nameSuffix[16];                               // 1-16     m_name_lang
                                                            // 17, name flags
                                                            // 18       m_internalName
    uint32    enchant_id[MAX_ITEM_ENCHANTMENT_EFFECTS];     // 19-21    m_enchantment
    uint32    prefix[MAX_ITEM_ENCHANTMENT_EFFECTS];         // 24-26    m_allocationPct
};

#define MAX_ITEM_SET_ITEMS 10
#define MAX_ITEM_SET_SPELLS 8

struct ItemSetEntry
{
    //uint32    id                                          // 0        m_ID
    char*     name[16];                                     // 1-16     m_name_lang
                                                            // 17 string flags, unused
    uint32    itemId[MAX_ITEM_SET_ITEMS];                   // 18-27    m_itemID
    //uint32    unknown[7];                                 // 28-34    unk, all 0
    uint32    spells[MAX_ITEM_SET_SPELLS];                  // 35-42    m_setSpellID
    uint32    items_to_triggerspell[MAX_ITEM_SET_SPELLS];   // 43-50    m_setThreshold
    uint32    required_skill_id;                            // 51       m_requiredSkill
    uint32    required_skill_value;                         // 52       m_requiredSkillRank
};

struct LFGDungeonEntry
{
    uint32  ID;                                             // 0
    char*   name[16];                                       // 1-17 Name lang
    uint32  minlevel;                                       // 18
    uint32  maxlevel;                                       // 19
    uint32  type;                                           // 26
    //uint32  unk;                                          // 27
    //char*   iconname;                                     // 28
    uint32  expansion;                                      // 29
    // Helpers
    uint32 Entry() const { return ID + (type << 24); }
};

struct LightEntry
{
    uint32 Id;
    uint32 MapId;
    float X;
    float Y;
    float Z;
    //float FalloffStart;
    //float FalloffEnd;
    //uint32 SkyAndFog;
    //uint32 WaterSettings;
    //uint32 SunsetParams;
    //uint32 OtherParams;
    //uint32 DeathParams;
};

struct LiquidTypeEntry
{
    uint32 Id;
    //char*  Name;
    uint32 Type;
    uint32 SpellId;
};

#define MAX_LOCK_CASE 8

struct LockEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      Type[MAX_LOCK_CASE];                        // 1-8      m_Type
    uint32      Index[MAX_LOCK_CASE];                       // 9-16     m_Index
    uint32      Skill[MAX_LOCK_CASE];                       // 17-24    m_Skill
    //uint32      Action[MAX_LOCK_CASE];                    // 25-32    m_Action
};

struct MailTemplateEntry
{
    uint32      ID;                                         // 0
    //char*       subject[16];                              // 1-16
                                                            // 17 name flags, unused
    char*       content[16];                                // 18-33
};

struct MapEntry
{
    uint32  MapID;                                          // 0
    //char*       internalname;                             // 1 unused
    uint32  map_type;                                       // 2
                                                            // 3 0 or 1 for battlegrounds (not arenas)
    char*   name[16];                                       // 4-19
                                                            // 20 name flags, unused
                                                            // 21-26 unused
    uint32  linked_zone;                                    // 27 common zone for instance and continent map
    //char*     hordeIntro[16];                             // 28-43 text for PvP Zones
                                                            // 44 intro text flags
    //char*     allianceIntro[16];                          // 45-60 text for PvP Zones
                                                            // 61 intro text flags
    uint32  multimap_id;                                    // 62
    //                                                      // 63-64
    //float BattlefieldMapIconScale;                        // 65
    //char* text[16]                                        // 66-81
    //flags                                                 // 82
    //char* HeroicErrorText[16]                             // 83-98
    //flags                                                 // 99
    //char* text[16]                                        // 100-115
    //flags                                                 // 116
    int32   entrance_map;                                   // 117 map_id of entrance map
    float   entrance_x;                                     // 118 entrance x coordinate (if exist single entry)
    float   entrance_y;                                     // 119 entrance y coordinate (if exist single entry)
    uint32 resetTimeRaid;                                   // 120
    uint32 resetTimeHeroic;                                 // 121
    //unkown                                                // 122
    //uint32 TimeOfDayOverride;                             // 123 -1, 0 and 720
    uint32  addon;                                          // 124 (0-original maps, 1-tbc addon)

    // Helpers
    uint32 Expansion() const { return addon; }

    bool IsDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
    bool IsNonRaidDungeon() const { return map_type == MAP_INSTANCE; }
    bool Instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool IsRaid() const { return map_type == MAP_RAID; }
    bool IsBattleground() const { return map_type == MAP_BATTLEGROUND; }
    bool IsBattleArena() const { return map_type == MAP_ARENA; }
    bool IsBattlegroundOrArena() const { return map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool IsWorldMap() const { return map_type == MAP_COMMON; }

    bool GetEntrancePos(int32 &mapid, float &x, float &y) const
    {
        if (entrance_map < 0)
            return false;
        mapid = entrance_map;
        x = entrance_x;
        y = entrance_y;
        return true;
    }

    bool IsContinent() const
    {
        return MapID == 0 || MapID == 1 || MapID == 530 || MapID == 571;
    }
};

struct NamesProfanityEntry
{
    //uint32    ID;                                         // 0
    char const* Name;                                       // 1
    int32       Language;                                   // 2
};

struct NamesReservedEntry
{
    //uint32    ID;                                         // 0
    char const* Name;                                       // 1
    int32       Language;                                   // 2
};

struct PvPDifficultyEntry
{
    //uint32      id;                                       // 0        m_ID
    uint32      mapId;                                      // 1
    uint32      bracketId;                                  // 2
    uint32      minLevel;                                   // 3
    uint32      maxLevel;                                   // 4
    uint32      difficulty;                                 // 5

    // helpers
    BattlegroundBracketId GetBracketId() const { return BattlegroundBracketId(bracketId); }
};

struct QuestSortEntry
{
    uint32      ID;                                         // 0
    //char*     Name[16];                                  // 1-16
                                                            // 17 string flag
};

struct RandomPropertiesPointsEntry
{
    //uint32  Id;                                           // 0 hidden key
    uint32    itemLevel;                                    // 1
    uint32    EpicPropertiesPoints[5];                      // 2-6
    uint32    RarePropertiesPoints[5];                      // 7-11
    uint32    UncommonPropertiesPoints[5];                  // 12-16
};

//struct SkillLineCategoryEntry{
//    uint32    id;                                         // 0      m_ID
//    char*     name[16];                                   // 1-17   m_name_lang
//                                                          // 18 string flag
//    uint32    displayOrder;                               // 19     m_sortIndex
//};

struct SkillLineEntry
{
    uint32    id;                                           // 0        m_ID
    int32     categoryId;                                   // 1        m_categoryID
    //uint32    skillCostID;                                // 2        m_skillCostsID
    char*     name[16];                                     // 3-18     m_displayName_lang
                                                            // 19 string flags
    //char*     description[16];                            // 20-35    m_description_lang
                                                            // 36 string flags
    uint32    spellIcon;                                    // 37       m_spellIconID
};

struct SkillLineAbilityEntry
{
    uint32    id;                                           // 0        m_ID
    uint32    skillId;                                      // 1        m_skillLine
    uint32    spellId;                                      // 2        m_spell
    uint32    racemask;                                     // 3        m_raceMask
    uint32    classmask;                                    // 4        m_classMask
    //uint32    racemaskNot;                                // 5        m_excludeRace
    //uint32    classmaskNot;                               // 6        m_excludeClass
    uint32    req_skill_value;                              // 7        m_minSkillLineRank
    uint32    forward_spellid;                              // 8        m_supercededBySpell
    uint32    AutolearnType;                                // 9        m_acquireMethod
    uint32    max_value;                                    // 10       m_trivialSkillLineRankHigh
    uint32    min_value;                                    // 11       m_trivialSkillLineRankLow
    //uint32    characterPoints[2];                         // 12-13    m_characterPoints[2]
    uint32    ReqTrainPoints;                               // 14
};

struct SkillRaceClassInfoEntry
{
    //uint32 Id;                                            // 0
    uint32 SkillId;                                         // 1
    uint32 RaceMask;                                        // 2
    uint32 ClassMask;                                       // 3
    uint32 Flags;                                           // 4
    //uint32 MinLevel;                                      // 5
    uint32 SkillTier;                                       // 6
    //uint32 SkillCostType;                                 // 7
};

#define MAX_SKILL_STEP 16

struct SkillTiersEntry
{
    uint32 Id;                                              // 0
    //uint32 StepCost[MAX_SKILL_STEP];                      // 1-16
    uint32 MaxSkill[MAX_SKILL_STEP];                        // 17-32
};

struct SoundEntriesEntry
{
    uint32    Id;                                           // 0        m_ID
    //uint32    Type;                                       // 1        m_soundType
    //char*     InternalName;                               // 2        m_name
    //char*     FileName[10];                               // 3-12     m_File[10]
    //uint32    Unk13[10];                                  // 13-22    m_Freq[10]
    //char*     Path;                                       // 23       m_DirectoryBase
                                                            // 24       m_volumeFloat
                                                            // 25       m_flags
                                                            // 26       m_minDistance
                                                            // 27       m_distanceCutoff
                                                            // 28       m_EAXDef
};

struct SpellEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    Category;                                     // 1        m_category
    //uint32    CastUI                                      // 2
    uint32    Dispel;                                       // 3        m_dispelType
    uint32    Mechanic;                                     // 4        m_mechanic
    uint32    Attributes;                                   // 5        m_attributes
    uint32    AttributesEx;                                 // 6        m_attributesEx
    uint32    AttributesEx2;                                // 7        m_attributesExB
    uint32    AttributesEx3;                                // 8        m_attributesExC
    uint32    AttributesEx4;                                // 9        m_attributesExD
    uint32    AttributesEx5;                                // 10       m_attributesExE
    uint32    AttributesEx6;                                // 11       m_attributesExF
    uint32    Stances;                                      // 12       m_shapeshiftMask
    uint32    StancesNot;                                   // 13       m_shapeshiftExclude
    uint32    Targets;                                      // 14       m_targets
    uint32    TargetCreatureType;                           // 15       m_targetCreatureType
    uint32    RequiresSpellFocus;                           // 16       m_requiresSpellFocus
    uint32    FacingCasterFlags;                            // 17       m_facingCasterFlags
    uint32    CasterAuraState;                              // 18       m_casterAuraState
    uint32    TargetAuraState;                              // 19       m_targetAuraState
    uint32    CasterAuraStateNot;                           // 20       m_excludeCasterAuraState
    uint32    TargetAuraStateNot;                           // 21       m_excludeTargetAuraState
    uint32    CastingTimeIndex;                             // 22       m_castingTimeIndex
    uint32    RecoveryTime;                                 // 23       m_recoveryTime
    uint32    CategoryRecoveryTime;                         // 24       m_categoryRecoveryTime
    uint32    InterruptFlags;                               // 25       m_interruptFlags
    uint32    AuraInterruptFlags;                           // 26       m_auraInterruptFlags
    uint32    ChannelInterruptFlags;                        // 27       m_channelInterruptFlags
    uint32    procFlags;                                    // 28       m_procTypeMask
    uint32    procChance;                                   // 29       m_procChance
    uint32    procCharges;                                  // 30       m_procCharges
    uint32    maxLevel;                                     // 31       m_maxLevel
    uint32    baseLevel;                                    // 32       m_baseLevel
    uint32    spellLevel;                                   // 33       m_spellLevel
    uint32    DurationIndex;                                // 34       m_durationIndex
    uint32    powerType;                                    // 35       m_powerType
    uint32    manaCost;                                     // 36       m_manaCost
    uint32    manaCostPerlevel;                             // 37       m_manaCostPerLevel
    uint32    manaPerSecond;                                // 38       m_manaPerSecond
    uint32    manaPerSecondPerLevel;                        // 39       m_manaPerSecondPerLeve
    uint32    rangeIndex;                                   // 40       m_rangeIndex
    float     speed;                                        // 41       m_speed
    //uint32    modalNextSpell;                             // 42       m_modalNextSpell not used
    uint32    StackAmount;                                  // 43       m_cumulativeAura
    uint32    Totem[2];                                     // 44-45    m_totem
    int32     Reagent[MAX_SPELL_REAGENTS];                  // 46-53    m_reagent
    uint32    ReagentCount[MAX_SPELL_REAGENTS];             // 54-61    m_reagentCount
    int32     EquippedItemClass;                            // 62       m_equippedItemClass (value)
    int32     EquippedItemSubClassMask;                     // 63       m_equippedItemSubclass (mask)
    int32     EquippedItemInventoryTypeMask;                // 64       m_equippedItemInvTypes (mask)
    uint32    Effect[MAX_SPELL_EFFECTS];                    // 65-67    m_effect
    int32     EffectDieSides[MAX_SPELL_EFFECTS];            // 68-70    m_effectDieSides
    int32     EffectBaseDice[MAX_SPELL_EFFECTS];            // 71-73
    float     EffectDicePerLevel[MAX_SPELL_EFFECTS];        // 74-76
    float     EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];  // 77-79    m_effectRealPointsPerLevel
    int32     EffectBasePoints[MAX_SPELL_EFFECTS];          // 80-82    m_effectBasePoints (must not be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    uint32    EffectMechanic[MAX_SPELL_EFFECTS];            // 83-85    m_effectMechanic
    uint32    EffectImplicitTargetA[MAX_SPELL_EFFECTS];     // 86-88    m_implicitTargetA
    uint32    EffectImplicitTargetB[MAX_SPELL_EFFECTS];     // 89-91    m_implicitTargetB
    uint32    EffectRadiusIndex[MAX_SPELL_EFFECTS];         // 92-94    m_effectRadiusIndex - spellradius.dbc
    uint32    EffectApplyAuraName[MAX_SPELL_EFFECTS];       // 95-97    m_effectAura
    uint32    EffectAmplitude[MAX_SPELL_EFFECTS];           // 98-100   m_effectAuraPeriod
    float     EffectValueMultiplier[MAX_SPELL_EFFECTS];     // 101-103
    uint32    EffectChainTarget[MAX_SPELL_EFFECTS];         // 104-106  m_effectChainTargets
    uint32    EffectItemType[MAX_SPELL_EFFECTS];            // 107-109  m_effectItemType
    int32     EffectMiscValue[MAX_SPELL_EFFECTS];           // 110-112  m_effectMiscValue
    int32     EffectMiscValueB[MAX_SPELL_EFFECTS];          // 113-115  m_effectMiscValueB
    uint32    EffectTriggerSpell[MAX_SPELL_EFFECTS];        // 116-118  m_effectTriggerSpell
    float     EffectPointsPerComboPoint[MAX_SPELL_EFFECTS]; // 119-121  m_effectPointsPerCombo
    uint32    SpellVisual[2];                               // 122-123  m_spellVisualID
    uint32    SpellIconID;                                  // 124      m_spellIconID
    uint32    activeIconID;                                 // 125      m_activeIconID
    uint32    SpellPriority;                                // 126      m_spellPriority
    char*     SpellName[16];                                // 127-142  m_name_lang
    //uint32    SpellNameFlag;                              // 143 not used
    char*     Rank[16];                                     // 144-159  m_nameSubtext_lang
    //uint32    RankFlags;                                  // 160 not used
    //char*     Description[16];                            // 161-176  m_description_lang not used
    //uint32    DescriptionFlags;                           // 177 not used
    //char*     ToolTip[16];                                // 178-193  m_auraDescription_lang not used
    //uint32    ToolTipFlags;                               // 194 not used
    uint32    ManaCostPercentage;                           // 195      m_manaCostPct
    uint32    StartRecoveryCategory;                        // 196      m_startRecoveryCategory
    uint32    StartRecoveryTime;                            // 197      m_startRecoveryTime
    uint32    MaxTargetLevel;                               // 198      m_maxTargetLevel
    uint32    SpellFamilyName;                              // 199      m_spellClassSet
    flag64    SpellFamilyFlags;                             // 200-201
    uint32    MaxAffectedTargets;                           // 202      m_maxTargets
    uint32    DmgClass;                                     // 203      m_defenseType
    uint32    PreventionType;                               // 204      m_preventionType
    //uint32    StanceBarOrder;                             // 205      m_stanceBarOrder not used
    float     EffectDamageMultiplier[MAX_SPELL_EFFECTS];    // 206-208  m_effectChainAmplitude
    //uint32    MinFactionId;                               // 209      m_minFactionID not used
    //uint32    MinReputation;                              // 210      m_minReputation not used
    //uint32    RequiredAuraVision;                         // 211      m_requiredAuraVision not used
    uint32    TotemCategory[2];                             // 212-213  m_requiredTotemCategoryID
    int32     AreaGroupId;                                  // 214      m_requiredAreaGroupId
    uint32    SchoolMask;                                   // 215      m_schoolMask
};

typedef std::set<uint32> PetFamilySpellsSet;
typedef std::map<uint32, PetFamilySpellsSet> PetFamilySpellsStore;

struct SpellCastTimesEntry
{
    uint32    ID;                                           // 0
    int32     CastTime;                                     // 1
    //float     CastTimePerLevel;                           // 2 unsure / per skill?
    //int32     MinCastTime;                                // 3 unsure
};

struct SpellCategoryEntry
{
    uint32 Id;
    uint32 Flags;
};

struct SpellFocusObjectEntry
{
    uint32    ID;                                           // 0
    //char*     Name[16];                                   // 1-15 unused
                                                            // 16 string flags, unused
};

struct SpellRadiusEntry
{
    uint32    ID;
    float     RadiusMin;
    float     RadiusPerLevel;
    float     RadiusMax;
};

struct SpellRangeEntry
{
    uint32    ID;
    float     minRange;
    float     maxRange;
    uint32    type;
    //char*     Name[16];                                   // 7-23 unused
                                                            // 24 string flags, unused
    //char*     Name2[16];                                  // 25-40 unused
                                                            // 41 string flags, unused
};

#define MAX_SHAPESHIFT_SPELLS 8

struct SpellShapeshiftEntry
{
    uint32 ID;                                              // 0
    //uint32 buttonPosition;                                // 1 unused
    //char*  Name[16];                                      // 2-17 unused
    //uint32 NameFlags;                                     // 18 unused
    uint32 flags1;                                          // 19
    int32  creatureType;                                    // 20 <= 0 humanoid, other normal creature types
    //uint32 unk1;                                          // 21 unused
    uint32 attackSpeed;                                     // 22
    uint32 modelID_A;                                       // 23 alliance modelid
    uint32 modelID_H;                                       // 24 horde modelid (only one form)
    //uint32 unk3;                                          // 25 unused
    //uint32 unk4;                                          // 26 unused
    uint32 stanceSpell[MAX_SHAPESHIFT_SPELLS];                                  // 27 - 34 unused
};

struct SpellDurationEntry
{
    uint32    ID;
    int32     Duration[3];
};

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      type[MAX_ITEM_ENCHANTMENT_EFFECTS];         // 2-4      m_effect[MAX_ITEM_ENCHANTMENT_EFFECTS]
    uint32      amount[MAX_ITEM_ENCHANTMENT_EFFECTS];       // 5-7      m_effectPointsMin[MAX_ITEM_ENCHANTMENT_EFFECTS]
    //uint32      amount2[MAX_ITEM_ENCHANTMENT_EFFECTS]     // 8-10     m_effectPointsMax[MAX_ITEM_ENCHANTMENT_EFFECTS]
    uint32      spellid[MAX_ITEM_ENCHANTMENT_EFFECTS];      // 11-13    m_effectArg[MAX_ITEM_ENCHANTMENT_EFFECTS]
    char*       description[16];                            // 14-29    m_name_lang[16]
    //uint32      descriptionFlags;                         // 30 name flags
    uint32      aura_id;                                    // 31       m_itemVisual
    uint32      slot;                                       // 32       m_flags
    uint32      GemID;                                      // 33       m_src_itemID
    uint32      EnchantmentCondition;                       // 34       m_condition_id
};

struct SpellItemEnchantmentConditionEntry
{
    uint32  ID;                                             // 0        m_ID
    uint8   Color[5];                                       // 1-5
    //uint32  Operand[5];                                   // 6-10
    uint8   Comparator[5];                                  // 11-15
    uint8   CompareColor[5];                                // 16-20
    uint32  Value[5];                                       // 21-25
    //uint8   Logic[5];                                     // 26-30
};

struct StableSlotPricesEntry
{
    uint32 Slot;
    uint32 Price;
};

struct SummonPropertiesEntry
{
    uint32  Id;                                             // 0
    uint32  Category;                                       // 1, 0 - can't be controlled?, 1 - something guardian?, 2 - pet?, 3 - something controllable?, 4 - taxi/mount?
    uint32  Faction;                                        // 2, 14 rows > 0
    uint32  Type;                                           // 3, see enum
    uint32  Slot;                                           // 4, 0-6
    uint32  Flags;                                          // 5
};

struct TalentEntry
{
    uint32    TalentID;                                     // 0
    uint32    TalentTab;                                    // 1 index in TalentTab.dbc (TalentTabEntry)
    uint32    Row;                                          // 2
    uint32    Col;                                          // 3
    uint32    RankID[MAX_TALENT_RANK];                      // 4-8
                                                            // 9-12 not used, always 0, maybe not used high ranks
    uint32    DependsOn;                                    // 13 index in Talent.dbc (TalentEntry)
                                                            // 14-15 not used
    uint32    DependsOnRank;                                // 16
                                                            // 17-18 not used
    //uint32  needAddInSpellBook;                           // 19  also need disable higest ranks on reset talent tree
    uint32    DependsOnSpell;                               // 20
};

struct TalentTabEntry
{
    uint32  TalentTabID;                                    // 0
    //char* name[16];                                       // 1-16, unused
    //uint32  nameFlags;                                    // 17, unused
    //unit32  spellicon;                                    // 18
                                                            // 19 not used
    uint32  ClassMask;                                      // 20
    uint32  tabpage;                                        // 21
    //char* internalname;                                   // 22
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    map_id;                                       // 1        m_ContinentID
    float     x;                                            // 2        m_x
    float     y;                                            // 3        m_y
    float     z;                                            // 4        m_z
    char*     name[16];                                     // 5-21     m_Name_lang
                                                            // 22 string flags
    uint32    MountCreatureID[2];                           // 23-24    m_MountCreatureID[2]
};

struct TaxiPathEntry
{
    uint32    ID;                                           // 0        m_ID
    uint32    from;                                         // 1        m_FromTaxiNode
    uint32    to;                                           // 2        m_ToTaxiNode
    uint32    price;                                        // 3        m_Cost
};

struct TaxiPathNodeEntry
{
                                                            // 0  ID
    uint32    PathID;                                       // 1
    uint32    NodeIndex;                                    // 2
    uint32    MapID;                                        // 3
    float     LocX;                                         // 4
    float     LocY;                                         // 5
    float     LocZ;                                         // 6
    uint32    Flags;                                        // 7
    uint32    Delay;                                        // 8
    uint32    ArrivalEventID;                               // 9
    uint32    DepartureEventID;                             // 10
};

struct TotemCategoryEntry
{
    uint32    ID;                                           // 0
    //char*   name[16];                                     // 1-16
                                                            // 17 string flags, unused
    uint32    categoryType;                                 // 18 (one for specialization)
    uint32    categoryMask;                                 // 19 (compatibility mask for same type: different for totems, compatible from high to low for rods)
};

struct TransportAnimationEntry
{
    //uint32  Id;
    uint32  TransportEntry;
    uint32  TimeSeg;
    float   X;
    float   Y;
    float   Z;
    //uint32  MovementId;
};

struct WMOAreaTableEntry
{
    uint32 Id;                                              // 0 index
    int32 rootId;                                           // 1 used in root WMO
    int32 adtId;                                            // 2 used in adt file
    int32 groupId;                                          // 3 used in group WMO
    //uint32 field4;
    //uint32 field5;
    //uint32 field6;
    //uint32 field7;
    //uint32 field8;
    uint32 Flags;                                           // 9 used for indoor/outdoor determination
    uint32 areaId;                                          // 10 link to AreaTableEntry.ID
    //char *Name[16];
    //uint32 nameFlags;
};

struct WorldMapAreaEntry
{
    //uint32  ID;                                           // 0
    uint32  map_id;                                         // 1
    uint32  area_id;                                        // 2 index (continent 0 areas ignored)
    //char* internal_name                                   // 3
    float   y1;                                             // 4
    float   y2;                                             // 5
    float   x1;                                             // 6
    float   x2;                                             // 7
    int32   virtual_map_id;                                 // 8 -1 (map_id have correct map) other: virtual map where zone show (map_id - where zone in fact internally)
};

#define MAX_WORLD_MAP_OVERLAY_AREA_IDX 4

struct WorldMapOverlayEntry
{
    uint32    ID;                                           // 0
    //uint32    worldMapAreaId;                             // 1 idx in WorldMapArea.dbc
    uint32    areatableID[MAX_WORLD_MAP_OVERLAY_AREA_IDX];  // 2-5
                                                            // 6-7 always 0, possible part of areatableID[]
    //char* internal_name                                   // 8
                                                            // 9-16 some ints
};

struct WorldSafeLocsEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*   name[16]                                      // 5-20 name, unused
                                                            // 21 name flags, unused
};

/*
struct WorldStateSounds
{
    uint32    ID;                                           // 0        Worldstate
    uint32    unk;                                          // 1
    uint32    areaTable;                                    // 2
    uint32    WMOAreaTable;                                 // 3
    uint32    zoneIntroMusicTable;                          // 4
    uint32    zoneIntroMusic;                               // 5
    uint32    zoneMusic;                                    // 6
    uint32    soundAmbience;                                // 7
    uint32    soundProviderPreferences;                     // 8
};
*/

/*
struct WorldStateUI
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1        Can be -1 to show up everywhere.
    uint32    zone;                                         // 2        Can be zero for "everywhere".
    uint32    phaseMask;                                    // 3        Phase this WorldState is avaliable in
    uint32    icon;                                         // 4        The icon that is used in the interface.
    char*     textureFilename;                              // 5
    char*     text;                                         // 6-21     The worldstate text
    char*     description;                                  // 22-38    Text shown when hovering mouse on icon
    uint32    worldstateID;                                 // 39       This is the actual ID used
    uint32    type;                                         // 40       0 = unknown, 1 = unknown, 2 = not shown in ui, 3 = wintergrasp
    uint32    unk1;                                         // 41
    uint32    unk2;                                         // 43
    uint32    unk3;                                         // 44-58
    uint32    unk4;                                         // 59-61    Used for some progress bars.
    uint32    unk7;                                         // 62       Unused in 3.3.5a
};
*/

#pragma pack(pop)

// Structures not used for casting to loaded DBC data and not required then packing
struct MapDifficulty
{
    MapDifficulty() : resetTime(0), maxPlayers(0), hasErrorMessage(false) { }
    MapDifficulty(uint32 _resetTime, uint32 _maxPlayers, bool _hasErrorMessage) : resetTime(_resetTime), maxPlayers(_maxPlayers), hasErrorMessage(_hasErrorMessage) { }

    uint32 resetTime;
    uint32 maxPlayers;
    bool hasErrorMessage;
};

struct TalentSpellPos
{
    TalentSpellPos() : talent_id(0), rank(0) { }
    TalentSpellPos(uint16 _talent_id, uint8 _rank) : talent_id(_talent_id), rank(_rank) { }

    uint16 talent_id;
    uint8  rank;
};

typedef std::map<uint32, TalentSpellPos> TalentSpellPosMap;

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0), price(0) { }
    TaxiPathBySourceAndDestination(uint32 _id, uint32 _price) : ID(_id), price(_price) { }

    uint32    ID;
    uint32    price;
};
typedef std::map<uint32, TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32, TaxiPathSetForSource> TaxiPathSetBySource;

typedef std::vector<TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#endif
