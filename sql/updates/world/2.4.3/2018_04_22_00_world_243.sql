DELETE FROM `item_enchantment_template` WHERE `entry` IN (161, 162, 163);
INSERT INTO `item_enchantment_template` (`entry`, `ench`, `chance`) VALUES
(161, 58, 48.65),
(161, 59, 51.35),
(162, 58, 51.35),
(162, 60, 48.65),
(163, 58, 55.4),
(163, 60, 44.6);

UPDATE `item_template` SET `spellid_2`=45048 WHERE `entry`=34425;
