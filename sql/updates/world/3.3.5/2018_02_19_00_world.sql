-- TDB 335.64 world
DELETE FROM `updates_include` WHERE `path`='$/sql/old/2.4.3/world';
INSERT INTO `updates_include` (`path`, `state`) VALUES
('$/sql/old/2.4.3/world', 'ARCHIVED');

UPDATE `version` SET `db_version`='DB 243.01', `cache_id`=1 LIMIT 1;
UPDATE `updates` SET `state`='ARCHIVED';
