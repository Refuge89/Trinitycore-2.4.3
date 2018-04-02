ALTER TABLE `spell_proc`
    DROP COLUMN `SpellFamilyMask2`;

DELETE FROM `spell_proc` WHERE `SpellId` IN (-1130,
-7001,
-16180,
-16880,
-29593,
-29723,
-30482,
-31226,
-31656.
-31785,
-33076,
-41635,
1719,
13163,
16880,
30482,
31785,
33076,
34074,
36032,
37536,
41635);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES 
(31785, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0),
(41635, 0, 0, 0, 0, 664232, 1, 0, 0, 0, 0, 0, 0, 0),
(33076, 0, 0, 0, 0, 664232, 1, 0, 0, 0, 0, 0, 0, 0),
(16880, 72, 7, 103, 58720258, 0, 0, 2, 2, 0, 0, 0, 0, 0),
(30482, 0, 0, 0, 0, 0, 1, 0, 1027, 2, 0, 0, 0, 0),
(-1130, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);

UPDATE `spell_proc` SET `SpellTypeMask`=0, `SpellPhaseMask`=0, `HitMask`=0 WHERE `SpellId`=-15270;
UPDATE `spell_proc` SET `SpellPhaseMask`=0 WHERE `SpellId` IN (15286,
24905);
UPDATE `spell_proc` SET `SpellTypeMask`=0, `SpellPhaseMask`=0 WHERE `SpellId` IN (20375,
21084,
31801);
UPDATE `spell_proc` SET `ProcFlags`=65536 WHERE `SpellId`=-11095;

DELETE FROM `spell_group_stack_rules` WHERE `group_id`=1058;

DELETE FROM `conditions` WHERE `ScriptName` IN ("condition_is_wintergrasp_horde",
"condition_is_wintergrasp_alliance");

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ("spell_gen_wg_water",
"spell_gen_ds_flush_knockback");

DELETE FROM `creature_template` WHERE `entry` IN (23575,
23997,
26712,
26248,
26249);

DELETE FROM `creature_template_addon` WHERE `entry` IN (23575,
23997,
26712,
26248,
26249);

UPDATE `creature_template` SET `ScriptName`="" WHERE `entry` IN (16980,
19668);

DELETE FROM `gameobject_template` WHERE `entry` IN (192951,
192518,
192519,
192520,
188526,
188527,
188528,
193611,
194628,
194739,
188593,
193093,
193094,
194519,
194541,
194542,
194543,
194316,
193963,
202794,
202796,
194370,
194371,
194375,
194377,
202242,
202243,
202244,
202245,
202246,
202223,
202235,
192181,
191579,
188596,
188422,
187373,
186944);

DELETE FROM `gameobject_template_addon` WHERE `entry` IN (192951,
192518,
192519,
192520,
188526,
188527,
188528,
193611,
194628,
194739,
188593,
193093,
193094,
194519,
194541,
194542,
194543,
194316,
193963,
202794,
202796,
194370,
194371,
194375,
194377,
202242,
202243,
202244,
202245,
202246,
202223,
202235,
192181,
191579,
188596,
188422,
187373,
186944);

UPDATE `creature_template` SET `AIName`="" WHERE `entry` IN (1268,
6119);
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (1268,
6119);

DELETE FROM `smart_scripts` WHERE `source_type`=1 AND `entryorguid` IN (192181,
191579,
188596,
188422,
187373,
186944);

DELETE FROM `smart_scripts` WHERE `source_type`=9 AND `entryorguid` IN (19218100,
19157900,
18859600,
18842200,
18737300,
18649100,
18649000,
12939100,
12939000);

DELETE FROM `smart_scripts` WHERE `entryorguid`=4046 AND `source_type`=0 AND `id`=2;
DELETE FROM `smart_scripts` WHERE `entryorguid`=5307 AND `source_type`=0 AND `id`=1;

UPDATE `smart_scripts` SET `id`=1 WHERE `entryorguid`=5307 AND `source_type`=0 AND `id`=2;
UPDATE `smart_scripts` SET `action_param1`=6606 WHERE `entryorguid`=3892 AND `source_type`=0 AND `id`=0;
UPDATE `smart_scripts` SET `action_param1`=5424 WHERE `entryorguid`=5327 AND `source_type`=0 AND `id`=0;
