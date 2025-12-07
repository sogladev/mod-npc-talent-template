-- DB: characters
-- Unified kits table: represents a logical kit (gear/talent/glyph)
CREATE TABLE IF NOT EXISTS `mod_npc_talent_template_kits` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `kit_type` ENUM('gear','talent','glyph') NOT NULL,
  `playerClass` ENUM('Warrior','Paladin','Hunter','Rogue','Priest','Death Knight','Shaman','Mage','Warlock','Druid') DEFAULT NULL,
  `name` VARCHAR(120) NOT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  UNIQUE KEY (`kit_type`,`name`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4 COMMENT='Logical kits (component groups) for gear/talents/glyphs';

-- Gear entries belong to a gear kit (one kit -> many gear rows). playerRaceMask kept per-row.
CREATE TABLE IF NOT EXISTS `mod_npc_talent_template_gear` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `kit_id` INT UNSIGNED NOT NULL,
  `playerRaceMask` int unsigned NOT NULL DEFAULT '0',
  `pos` int unsigned NOT NULL DEFAULT '0',
  `itemEntry` int unsigned NOT NULL DEFAULT '0',
  `enchant` int unsigned NOT NULL DEFAULT '0',
  `socket1` int unsigned NOT NULL DEFAULT '0',
  `socket2` int unsigned NOT NULL DEFAULT '0',
  `socket3` int unsigned NOT NULL DEFAULT '0',
  `bonusEnchant` int unsigned NOT NULL DEFAULT '0',
  `prismaticEnchant` int DEFAULT NULL,
  INDEX (`kit_id`),
  CONSTRAINT `fk_gear_kit` FOREIGN KEY (`kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4 COMMENT='Templates Gear entries linked to kit';

-- Talent entries belong to a talent kit (one kit -> many talent IDs)
CREATE TABLE IF NOT EXISTS `mod_npc_talent_template_talents` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `kit_id` INT UNSIGNED NOT NULL,
  `talentId` int unsigned NOT NULL DEFAULT '0',
  INDEX (`kit_id`),
  CONSTRAINT `fk_talent_kit` FOREIGN KEY (`kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4 COMMENT='Templates talent entries linked to kit';

-- Glyph entries belong to a glyph kit (one kit -> many glyph rows)
CREATE TABLE IF NOT EXISTS `mod_npc_talent_template_glyphs` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `kit_id` INT UNSIGNED NOT NULL,
  `slot` tinyint unsigned NOT NULL DEFAULT '0',
  `glyph` smallint unsigned NOT NULL DEFAULT '0',
  INDEX (`kit_id`),
  CONSTRAINT `fk_glyph_kit` FOREIGN KEY (`kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4 COMMENT='Templates glyph entries linked to kit';

-- Templates reference kit ids for each component type
CREATE TABLE IF NOT EXISTS `mod_npc_talent_template_templates` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `playerClass` ENUM('Warrior','Paladin','Hunter','Rogue','Priest','Death Knight','Shaman','Mage','Warlock','Druid') NOT NULL COMMENT 'Player class',
  `mask` int unsigned NOT NULL DEFAULT '0',
  `category` ENUM('pve','pvp','vip','utility') NOT NULL DEFAULT 'pve',
  `minLevel` INT UNSIGNED NOT NULL DEFAULT 1,
  `maxLevel` INT UNSIGNED NOT NULL DEFAULT 255,
  `gear_kit_id` INT UNSIGNED DEFAULT NULL,
  `talent_kit_id` INT UNSIGNED DEFAULT NULL,
  `glyph_kit_id` INT UNSIGNED DEFAULT NULL,
  `lock_requirement` VARCHAR(200) DEFAULT '',
  -- shown to player in menu
  `template_name` VARCHAR(200) NOT NULL,           -- friendly text shown in menu
  `icon` VARCHAR(200) DEFAULT NULL,                 -- optional icon string
  -- global enable/disable
  `enabled` TINYINT(1) NOT NULL DEFAULT 1,
  UNIQUE KEY (`playerClass`,`template_name`),
  INDEX `idx_gear_kit` (`gear_kit_id`),
  INDEX `idx_talent_kit` (`talent_kit_id`),
  INDEX `idx_glyph_kit` (`glyph_kit_id`),
  CONSTRAINT `fk_template_gear_kit` FOREIGN KEY (`gear_kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE RESTRICT,
  CONSTRAINT `fk_template_talent_kit` FOREIGN KEY (`talent_kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE RESTRICT,
  CONSTRAINT `fk_template_glyph_kit` FOREIGN KEY (`glyph_kit_id`) REFERENCES `mod_npc_talent_template_kits`(`id`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4 COMMENT='Templates Meta referencing kits';
