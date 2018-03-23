DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_gen_aura_of_fear','spell_gen_choking_vines');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(28313, 'spell_gen_aura_of_fear'),
(35244, 'spell_gen_choking_vines');
