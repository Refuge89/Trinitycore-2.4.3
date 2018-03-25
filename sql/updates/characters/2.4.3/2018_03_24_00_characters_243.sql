ALTER TABLE `character_pet`
    ADD COLUMN `loyalty` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `curhappiness`,
    ADD COLUMN `loyaltyPoints` INT(10) NOT NULL DEFAULT '0' AFTER `loyalty`,
    ADD COLUMN `trainingPoints` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `loyaltyPoints`,
    ADD COLUMN `resetTalentsCost` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `trainingPoints`,
    ADD COLUMN `resetTalentsTime` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `resetTalentsCost`;
