-- Misc corrections noted while revising table
UPDATE `spell_bonus_data` SET `ap_dot_bonus`=0.02 WHERE `entry`=3674; -- Hunter - Black Arrow
UPDATE `spell_bonus_data` SET `ap_bonus`=0.2, `direct_bonus`=0.32 WHERE `entry`=20187; -- Paladin - Seal of Righteousness unleashed
UPDATE `spell_bonus_data` SET `ap_dot_bonus`=0.025, `dot_bonus`=0.013 WHERE `entry` IN (31803,53742); -- Paladin - Holy Vengeance/Blood Corruption
UPDATE `spell_bonus_data` SET `ap_bonus`=0.14, `direct_bonus`=0.22 WHERE `entry` IN (31804,53733); -- Paladin - Seal of Vengeance/Corruption unleashed
