DELETE FROM `playercreateinfo` WHERE `class`=6;
DELETE FROM `playercreateinfo_item` WHERE `class`=6;
DELETE FROM `playercreateinfo_skills` WHERE `skill` IN (777,
778);
DELETE FROM `playercreateinfo_skills` WHERE `classMask`=32;
DELETE FROM `playercreateinfo_spell_custom` WHERE `classmask`=32;
DELETE FROM `playercreateinfo_cast_spell` WHERE `classMask`=32;

DELETE FROM `pet_levelstats` WHERE `creature_entry`=26125;
