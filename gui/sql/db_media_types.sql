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


-- filling media types list...
INSERT INTO media_types (id,name) VALUES (          1, 'CD-ROM');
INSERT INTO media_types (id,name) VALUES (          2, 'CD-R');
INSERT INTO media_types (id,name) VALUES (          4, 'CD-RW');
INSERT INTO media_types (id,name) VALUES (         12, 'CD-RW HS');
INSERT INTO media_types (id,name) VALUES (         20, 'CD-RW US');
INSERT INTO media_types (id,name) VALUES (         28, 'CD-RW US+');

INSERT INTO media_types (id,name) VALUES (         64, 'DVD-ROM');
INSERT INTO media_types (id,name) VALUES (        128, 'DVD-RAM');
INSERT INTO media_types (id,name) VALUES (        256, 'DVD-R');
INSERT INTO media_types (id,name) VALUES (        512, 'DVD-RW');
INSERT INTO media_types (id,name) VALUES (       1024, 'DVD-RW Sequential');
INSERT INTO media_types (id,name) VALUES (       2048, 'DVD-RW Restricted');
INSERT INTO media_types (id,name) VALUES (       4096, 'DVD-R DL');
INSERT INTO media_types (id,name) VALUES (       8192, 'DVD-R DL (Layer Jump)');

INSERT INTO media_types (id,name) VALUES (      16384, 'DVD+RW');
INSERT INTO media_types (id,name) VALUES (      32768, 'DVD+RW DL');
INSERT INTO media_types (id,name) VALUES (      65536, 'DVD+R');
INSERT INTO media_types (id,name) VALUES (     131072, 'DVD+R DL');

INSERT INTO media_types (id,name) VALUES (     262144, 'DD CD-ROM');
INSERT INTO media_types (id,name) VALUES (     524288, 'DD CD-R');
INSERT INTO media_types (id,name) VALUES (    1048576, 'DD CD-RW');


INSERT INTO media_types (id,name) VALUES (    2097152, 'BD-ROM');
INSERT INTO media_types (id,name) VALUES (    4194304, 'BD-R Sequential');
INSERT INTO media_types (id,name) VALUES (    8388608, 'BD-R Random Write');
INSERT INTO media_types (id,name) VALUES (   16777216, 'BD-RE');

INSERT INTO media_types (id,name) VALUES (   33554432, 'HD DVD-ROM');
INSERT INTO media_types (id,name) VALUES (   67108864, 'HD DVD-R');
INSERT INTO media_types (id,name) VALUES (  134217728, 'HD DVD-RAM');
INSERT INTO media_types (id,name) VALUES (  268435456, 'HD DVD-RW');
INSERT INTO media_types (id,name) VALUES (  536870912, 'HD DVD-R DL');
INSERT INTO media_types (id,name) VALUES ( 1073741824, 'HD DVD-RW DL');

INSERT INTO media_types (id,name) VALUES ( 2147483648, 'DVD-RW DL'); -- (1ULL << 31)


-- (1ULL << 63) -- 'Unknown media'
