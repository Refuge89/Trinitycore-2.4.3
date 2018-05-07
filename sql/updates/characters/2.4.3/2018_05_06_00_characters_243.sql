ALTER TABLE `characters`
    CHANGE COLUMN `drunk` `drunk` SMALLINT UNSIGNED NOT NULL DEFAULT '0' AFTER `watchedFaction`;
