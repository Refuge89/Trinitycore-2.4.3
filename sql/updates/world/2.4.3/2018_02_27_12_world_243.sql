-- Tomb of Seven Event
DELETE FROM `smart_scripts` WHERE `entryorguid`=9040 AND `source_type`=0 AND `id`=6;
DELETE FROM `smart_scripts` WHERE `entryorguid`=9034 AND `source_type`=0 AND `id`=5;
DELETE FROM `smart_scripts` WHERE `entryorguid`=9035 AND `source_type`=0 AND `id`=5;
DELETE FROM `smart_scripts` WHERE `entryorguid`=9036 AND `source_type`=0 AND `id`=5;
DELETE FROM `smart_scripts` WHERE `entryorguid`=9038 AND `source_type`=0 AND `id`=6;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES 
(9040, 0, 6, 0, 6, 0, 100, 3, 0, 0, 0, 0, 34, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dope''rel - On Death - Set Instance Data 20 to 1'),
(9034, 0, 5, 0, 6, 0, 100, 3, 0, 0, 0, 0, 34, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hate''rel - On Death - Set Instance Data 20 to 1'),
(9035, 0, 5, 0, 6, 0, 100, 3, 0, 0, 0, 0, 34, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Anger''rel - On Death - Set Instance Data 20 to 1'),
(9036, 0, 5, 0, 6, 0, 100, 3, 0, 0, 0, 0, 34, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vile''rel - On Death - Set Instance Data 20 to 1'),
(9038, 0, 6, 0, 6, 0, 100, 3, 0, 0, 0, 0, 34, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Seeth''rel - On Death - Set Instance Data 20 to 1');
