-- COMMAND
DELETE FROM `command` WHERE `name` IN ('templatenpc reload', 'templatenpc kit create', 'templatenpc kit delete', 'templatenpc kit list');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('templatenpc reload', 3, 'Syntax: .templatenpc reload\nType .templatenpc reload to reload Template NPC database changes');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('templatenpc kit create', 3, 'Syntax: .templatenpc kit create <gear|talent|glyph> [kitId] <name>\nCreate a kit for your class using your current equipment/talents/glyphs'),
('templatenpc kit delete', 3, 'Syntax: .templatenpc kit delete <kitId>\nDelete a kit from the DB');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('templatenpc kit list', 3, 'Syntax: .templatenpc kit list [<gear|talent|glyph>] [<Class>]\nList kits, optional filters by kit type and/or class');
