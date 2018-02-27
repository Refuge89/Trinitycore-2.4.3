DELETE FROM `spell_custom_attr` WHERE `entry` IN (12654, 27813, 27817, 2781812721);
INSERT INTO `spell_custom_attr` (`entry`, `attributes`) VALUES
(12654, 2048), -- Ignite
(27813, 2048), -- Blessed Recovery (Rank 1)
(27817, 2048), -- Blessed Recovery (Rank 2)
(27818, 2048), -- Blessed Recovery (Rank 3)
(12721, 2048); -- Deep Wounds
