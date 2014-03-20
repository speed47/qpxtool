--
-- This file is part of the QPxTool project.
-- Copyright (C) 2010 Gennady 'ShultZ' Kozlov <qpxtool@mail.ru>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
-- See the file 'COPYING' for the exact licensing terms.
--

CREATE TABLE media_types (
	id	bigint  NOT NULL,	-- identifier, values from libqpxtransport
	name	varchar(32) NOT NULL,	-- human-readable media type name

	CONSTRAINT media_types_pkey PRIMARY KEY (id)
);

CREATE TABLE dev_vendors (
	id		serial NOT NULL,
	name		char(8),

	CONSTRAINT dev_vendors_pkey PRIMARY KEY (id),
	CONSTRAINT dev_vendors_uniq UNIQUE (name)
);

CREATE TABLE dev_models (
	id		serial NOT NULL,
	id_vendor	integer NOT NULL,
	name		char(16),
	plugin		varchar,

	CONSTRAINT dev_models_pkey PRIMARY KEY (id),
	CONSTRAINT dev_models_ref_vendors FOREIGN KEY (id_vendor)
		REFERENCES dev_vendors (id) MATCH FULL
		ON UPDATE RESTRICT ON DELETE RESTRICT,
	CONSTRAINT dev_models_uniq UNIQUE (id_vendor,name)
);

CREATE TABLE reports (
	id			serial,
-- media info
	id_media_type	bigint NOT NULL, -- media type
	label			varchar(128) NOT NULL,   -- media label
	copy_idx		integer NOT NULL DEFAULT 0,	-- backup copy index

-- device info
	dev_id			integer,	-- device model
	dev_fw			char(4),	-- device firmware

	md5				char(32),	-- MD5 sum of written media
	metadata		text,		-- metadata
	data_xml		bytea,		-- report data
	datetime		timestamp without time zone NOT NULL DEFAULT now(), -- db insertion timestamp

	CONSTRAINT reports_pkey PRIMARY KEY (id),
	CONSTRAINT reports_ref_media FOREIGN KEY (id_media_type)
		REFERENCES media_types (id) MATCH FULL
		ON UPDATE RESTRICT ON DELETE RESTRICT,
	CONSTRAINT reports_ref_device FOREIGN KEY (dev_id)
		REFERENCES dev_models (id) MATCH FULL
		ON UPDATE RESTRICT ON DELETE RESTRICT,
	CONSTRAINT reports_label_uniq UNIQUE (label, copy_idx)
);

