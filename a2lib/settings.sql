BEGIN TRANSACTION;
CREATE TABLE vf48 ( run             INTEGER,
                    modulenumber    INTEGER,
                    modulename      TEXT,
                    frequency       REAL,
                    samples         INTEGER,
                    divisor         INTEGER,
                    soffset         INTEGER,
                    subsample       REAL,
                    offset          INTEGER,
                    notes           TEXT,
                    timeEnter       DATE );

INSERT INTO "vf48" VALUES(7000, 1, 'ab', 40000000.0, 430,  6, 31, 3.0, 0, '40Mhz firmware', '2008');
INSERT INTO "vf48" VALUES(7000, 2, 'ac', 40000000.0, 430,  6, 31, 3.0, 0, '40Mhz firmware', '2008');
INSERT INTO "vf48" VALUES(7000, 4, 'ae', 40000000.0, 430,  6, 31, 3.0, 0, '40Mhz firmware', '2008');
INSERT INTO "vf48" VALUES(7000, 5, 'af', 40000000.0, 430,  6, 31, 3.0, 0, '40Mhz firmware', '2008');

INSERT INTO "vf48" VALUES(10577, 0, 'a0', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 03:24:06');
INSERT INTO "vf48" VALUES(10577, 1, 'ab', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 03:24:14');
INSERT INTO "vf48" VALUES(10577, 2, 'ac', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 03:24:19');
INSERT INTO "vf48" VALUES(10577, 4, 'ae', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 03:24:25');
INSERT INTO "vf48" VALUES(10577, 5, 'af', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 03:24:31');

INSERT INTO "vf48" VALUES(11985, 0, 'a0', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 02:45:09');
INSERT INTO "vf48" VALUES(11985, 1, 'ab', 60000000.0, 406,  6, 13, 3.0, 1,  '60Mhz firmware', '2009-09-23 02:45:18');
INSERT INTO "vf48" VALUES(11985, 2, 'ac', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 02:45:27');
INSERT INTO "vf48" VALUES(11985, 4, 'ae', 60000000.0, 406,  6, 13, 3.0, 1, '60Mhz firmware', '2009-09-23 02:45:37');
INSERT INTO "vf48" VALUES(11985, 5, 'af', 20000000.0, 406,  2, 20, 3.0, 1, '20Mhz firmware', '2009-09-23 02:46:08');

INSERT INTO "vf48" VALUES(15843, 0, 'a0', 60000000.0, 406,  6, 13, 3.0, 1, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 3 samples / strip', '2010-04-01 16:55:09');
INSERT INTO "vf48" VALUES(15843, 1, 'ab', 60000000.0, 406,  6, 13, 3.0, 1, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 3 samples / strip', '2010-04-01 16:55:09');
INSERT INTO "vf48" VALUES(15843, 2, 'ac', 60000000.0, 406,  6, 13, 3.0, 1, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 3 samples / strip', '2010-04-01 16:55:09');
INSERT INTO "vf48" VALUES(15843, 3, 'ad', 60000000.0, 406,  6, 13, 3.0, 1, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 3 samples / strip', '2010-04-01 16:55:09');
INSERT INTO "vf48" VALUES(15843, 4, 'ae', 60000000.0, 406,  6, 13, 3.0, 1, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 3 samples / strip', '2010-04-01 16:55:09');

INSERT INTO "vf48" VALUES(15963, 0, 'a0', 60000000.0, 180, 18,  4, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 1 sample / strip', '2010-05-11 12:41:46');
INSERT INTO "vf48" VALUES(15963, 1, 'ab', 60000000.0, 180, 18,  4, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 1 sample / strip', '2010-05-11 12:41:46');
INSERT INTO "vf48" VALUES(15963, 2, 'ac', 60000000.0, 180, 18,  4, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 1 sample / strip', '2010-05-11 12:41:46');
INSERT INTO "vf48" VALUES(15963, 3, 'ad', 60000000.0, 180, 18,  4, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 1 sample / strip', '2010-05-11 12:41:46');
INSERT INTO "vf48" VALUES(15963, 4, 'ae', 60000000.0, 180, 18,  4, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 1 sample / strip', '2010-05-11 12:41:46');

INSERT INTO "vf48" VALUES(16315, 0, 'a0', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');
INSERT INTO "vf48" VALUES(16315, 1, 'ab', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');
INSERT INTO "vf48" VALUES(16315, 2, 'ac', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');
INSERT INTO "vf48" VALUES(16315, 3, 'ad', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');
INSERT INTO "vf48" VALUES(16315, 4, 'ae', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');
INSERT INTO "vf48" VALUES(16315, 5, 'af', 60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x4bb3ccd4_0x4bb2 firmware : 128 samples', '2010-05-19 18:04:41');

INSERT INTO "vf48" VALUES(30036, 0, 'a0',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 1, 'ab',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 2, 'ac',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 3, 'ad',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 4, 'ae',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 5, 'af',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 6, 'a1',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');
INSERT INTO "vf48" VALUES(30036, 7, 'a2',60000000.0, 128, 18,  0, 1.0, 0, 'VF48_0x07110817_0x4e3ae33f firmware : 128 samples', '2012-09-28 14:00:00');



CREATE TABLE dir_table ( vf48mapping_dir TEXT,
                         detectorgeo_dir TEXT,
                         timeEnter       DATE);

INSERT INTO "dir_table" VALUES('/a2lib/maps/', '/a2lib/geo/', '2012-10-10 15:14:44-07:00');

CREATE TABLE runtable( run INTEGER PRIMARY KEY,
vf48mapping text,
detectorgeo text,
detectorenv text,
detectormat text,
notes       text,
timeEnter date);


INSERT INTO "runtable" VALUES(30036, 'vf48_map.00015', 'detector2_geo.xml', 'environment2_geo.xml', 'material.xml', 'ALPHA2', '2012-09-27 14:31:40');
INSERT INTO "runtable" VALUES(15843, 'vf48_map.00014', 'detector_geo.xml', 'environment_geo.xml', 'material.xml', '2010-run', '2010-05-07 17:43:57');
INSERT INTO "runtable" VALUES(10577, 'vf48_map.00013', 'detector_geo.xml', 'environment_geo.xml', 'material.xml', 'After insertion', '2009-09-23 02:31:40');
INSERT INTO "runtable" VALUES(10349, 'vf48_map.00012', 'detector_geo.xml', 'environment_geo.xml', 'material.xml', 'Pre-insertion', '2009-09-23 02:32:20');
INSERT INTO "runtable" VALUES(7000,  'vf48_map.00011', 'detector_geo.xml', 'environment_geo.xml', 'material.xml', '2008-run is approx., rms doesn''t exist', '2009-09-23 02:40:37');
CREATE TABLE dumptable  (run INTEGER,
                         dumpnum INTEGER,
                         dumpname TEXT);    
INSERT INTO "dumptable" VALUES(0, 0,          '');
INSERT INTO "dumptable" VALUES(42077, 0,      'DD on resonance');
INSERT INTO "dumptable" VALUES(42077, 1,      'CC On resonance');
INSERT INTO "dumptable" VALUES(42077, 2,      'DD off resonance');
INSERT INTO "dumptable" VALUES(42077, 3,      'CC off resonance');



CREATE TABLE sis        ( run         INTEGER,
                          channel     INTEGER,
                          description TEXT );

INSERT INTO "sis" VALUES( 0, 0,       'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 0, 1,       'SIS_AD' );
INSERT INTO "sis" VALUES( 0, 2,       'SIS_PMT_DEG_OR' );
INSERT INTO "sis" VALUES( 0, 3,       'SIS_PMT_TRAP_OR' );
INSERT INTO "sis" VALUES( 0, 4,       'SIS_PMT_11_AND_12' );
INSERT INTO "sis" VALUES( 0, 5,       'SIS_PMT_DOWN_OR' );
INSERT INTO "sis" VALUES( 0, 6,       'SIS_QUENCH_START' );
INSERT INTO "sis" VALUES( 0, 7,       'SIS_QUENCH_STOP' );
INSERT INTO "sis" VALUES( 0, 8,       'SIS_PMT_1_AND_2' );
INSERT INTO "sis" VALUES( 0, 9,       'SIS_PMT_3_AND_4' );

INSERT INTO "sis" VALUES( 0, 12,      'SIS_PMT_9_AND_10' );

INSERT INTO "sis" VALUES( 0, 16,      'SIS_50Mhz_CLK' );
INSERT INTO "sis" VALUES( 0, 17,      'SIS_PMT_5_AND_6' );
INSERT INTO "sis" VALUES( 0, 18,      'SIS_PMT_7_AND_8' );

INSERT INTO "sis" VALUES( 0, 20,      'SIS_PMT_DEG_AND' );

INSERT INTO "sis" VALUES( 0, 22,      'SIS_LNE_BEFORE' );
INSERT INTO "sis" VALUES( 0, 23,      'SIS_LNE_AFTER' );
INSERT INTO "sis" VALUES( 0, 24,      'SIS_BACKGROUND_START' );

INSERT INTO "sis" VALUES( 0, 26,      'SIS_BACKGROUND_STOP' );

INSERT INTO "sis" VALUES( 0, 28,      'SIS_MIXING_START' );
INSERT INTO "sis" VALUES( 0, 29,      'SIS_MIXING_STOP' );
INSERT INTO "sis" VALUES( 0, 30,      'SIS_DUMP_START' );
INSERT INTO "sis" VALUES( 0, 31,      'SIS_DUMP_STOP' );
INSERT INTO "sis" VALUES( 0, 32,      'SIS2_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 0, 33,      'SIS2_AD' );


INSERT INTO "sis" VALUES( 0, 39,      'SIS2_SAP' );

INSERT INTO "sis" VALUES( 0, 43,      'SIS2_TTC_TRIG1' );
INSERT INTO "sis" VALUES( 0, 44,      'SIS2_TTC_TRIG2' );
INSERT INTO "sis" VALUES( 0, 45,      'SIS2_ADC_TRIG' );
INSERT INTO "sis" VALUES( 0, 46,      'SIS2_TTC_TRIG4' );
INSERT INTO "sis" VALUES( 0, 47,      'SIS2_TTC_TRIG5' );
INSERT INTO "sis" VALUES( 0, 48,      'SIS2_TTC_TRIG6' );
INSERT INTO "sis" VALUES( 0, 49,      'SIS2_TTC_1_OR_4' );
INSERT INTO "sis" VALUES( 0, 50,      'SIS2_TTC_2_AND_5' );
INSERT INTO "sis" VALUES( 0, 51,      'SIS2_TTC_SI_TRIG' );
INSERT INTO "sis" VALUES( 0, 52,      'SIS2_AD_IN' );

INSERT INTO "sis" VALUES( 0, 58,      'SIS2_PBAR_SEQ_START' );
INSERT INTO "sis" VALUES( 0, 59,      'SIS2_PBAR_SEQ_TIME' );
INSERT INTO "sis" VALUES( 0, 60,      'SIS_RECATCH_SEQ_START' );
INSERT INTO "sis" VALUES( 0, 61,      'SIS2_TTC_TRIG3' );
INSERT INTO "sis" VALUES( 0, 62,      'SIS2_MIX_SEQ_TIME' );



INSERT INTO "sis" VALUES( 15843, 45,  'IO32_TRIG' );
INSERT INTO "sis" VALUES( 15843, 46,  'IO32_TRIG_NOBUSY' );
INSERT INTO "sis" VALUES( 15843, 13,  '' );
INSERT INTO "sis" VALUES( 15843, 24,  'SIS_TRIG_B_START' );
INSERT INTO "sis" VALUES( 15843, 26,  'SIS_TRIG_B_STOP' );
INSERT INTO "sis" VALUES( 15843, 28,  'SIS_TRIG_A_START' );
INSERT INTO "sis" VALUES( 15843, 29,  'SIS_TRIG_A_STOP' );
INSERT INTO "sis" VALUES( 15843, 32,  'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 15843, 40,  'SIS2_SAT' );
INSERT INTO "sis" VALUES( 15843, 41,  'SIS2_SATC' );
INSERT INTO "sis" VALUES( 15843, 56,  'SIS2_T3_OR_T6' );
INSERT INTO "sis" VALUES( 15843, -10, 'SIS_MIXING_START' );
INSERT INTO "sis" VALUES( 15843, -10, 'SIS_MIXING_STOP' );
INSERT INTO "sis" VALUES( 15843, -10, 'SIS_BACKGROUND_START' );
INSERT INTO "sis" VALUES( 15843, -10, 'SIS_BACKGROUND_STOP' );



INSERT INTO "sis" VALUES( 30036, 0,       'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 30036, 1,       'SIS_AD' );
INSERT INTO "sis" VALUES( 30036, 2,       '' );
INSERT INTO "sis" VALUES( 30036, 3,       'SIS_PMT_CATCH_OR' );
INSERT INTO "sis" VALUES( 30036, 4,       'SIS_PMT_ATOM_OR' );
INSERT INTO "sis" VALUES( 30036, 5,       '' );
INSERT INTO "sis" VALUES( 30036, 6,       'IO32_TRIG' );
INSERT INTO "sis" VALUES( 30036, 7,       'IO32_TRIG_NOBUSY' );
INSERT INTO "sis" VALUES( 30036, 8,       '' );
INSERT INTO "sis" VALUES( 30036, 9,       '' );
INSERT INTO "sis" VALUES( 30036, 10,      '' );
INSERT INTO "sis" VALUES( 30036, 11,      '' );
INSERT INTO "sis" VALUES( 30036, 12,      '' );
INSERT INTO "sis" VALUES( 30036, 13,      '' );
INSERT INTO "sis" VALUES( 30036, 14,      '' );
INSERT INTO "sis" VALUES( 30036, 15,      '' );
INSERT INTO "sis" VALUES( 30036, 16,      'SIS_50Mhz_CLK' );
INSERT INTO "sis" VALUES( 30036, 17,      'SIS_PMT_7_AND_8' );
INSERT INTO "sis" VALUES( 30036, 18,      'SIS_PMT_5_AND_6' );
INSERT INTO "sis" VALUES( 30036, 19,      'SIS_PMT_1_AND_2' );
INSERT INTO "sis" VALUES( 30036, 20,      'SIS_PMT_3_AND_4' );
INSERT INTO "sis" VALUES( 30036, 21,      'SIS_PMT_CATCH_AND' );
INSERT INTO "sis" VALUES( 30036, 22,      'SIS_PMT_ATOM_AND' );
INSERT INTO "sis" VALUES( 30036, 23,      '' );
INSERT INTO "sis" VALUES( 30036, 24,      '' );
INSERT INTO "sis" VALUES( 30036, 25,      '' );
INSERT INTO "sis" VALUES( 30036, 26,      'SIS_RECATCH_DUMP_START' );
INSERT INTO "sis" VALUES( 30036, 27,      'SIS_RECATCH_DUMP_STOP' );
INSERT INTO "sis" VALUES( 30036, 28,      'SIS_ATOM_DUMP_START' );
INSERT INTO "sis" VALUES( 30036, 29,      'SIS_ATOM_DUMP_STOP' );
INSERT INTO "sis" VALUES( 30036, 30,      'SIS_PBAR_DUMP_START' );
INSERT INTO "sis" VALUES( 30036, 31,      'SIS_PBAR_DUMP_STOP' );

INSERT INTO "sis" VALUES( 30036, 32,      'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 30036, 33,      'SIS_AD_FAKE' );
INSERT INTO "sis" VALUES( 30036, 34,      '' );
INSERT INTO "sis" VALUES( 30036, 35,      '' );
INSERT INTO "sis" VALUES( 30036, 36,      '' );
INSERT INTO "sis" VALUES( 30036, 37,      '' );
INSERT INTO "sis" VALUES( 30036, 38,      '' );
INSERT INTO "sis" VALUES( 30036, 39,      '' );
INSERT INTO "sis" VALUES( 30036, 40,      '' );
INSERT INTO "sis" VALUES( 30036, 41,      '' );
INSERT INTO "sis" VALUES( 30036, 42,      '' );
INSERT INTO "sis" VALUES( 30036, 43,      '' );
INSERT INTO "sis" VALUES( 30036, 44,      '' );
INSERT INTO "sis" VALUES( 30036, 45,      '' );
INSERT INTO "sis" VALUES( 30036, 46,      '' );
INSERT INTO "sis" VALUES( 30036, 47,      '' );
INSERT INTO "sis" VALUES( 30036, 48,      '' );
INSERT INTO "sis" VALUES( 30036, 49,      '' );
INSERT INTO "sis" VALUES( 30036, 50,      '' );
INSERT INTO "sis" VALUES( 30036, 51,      '' );
INSERT INTO "sis" VALUES( 30036, 52,      'SIS_DIX_ALPHA' );
INSERT INTO "sis" VALUES( 30036, 53,      '' );
INSERT INTO "sis" VALUES( 30036, 54,      '' );
INSERT INTO "sis" VALUES( 30036, 55,      '' );
INSERT INTO "sis" VALUES( 30036, 56,      '' );
INSERT INTO "sis" VALUES( 30036, 57,      'SIS_AD_2' );
INSERT INTO "sis" VALUES( 30036, 58,      'SIS_PBAR_SEQ_START' );
INSERT INTO "sis" VALUES( 30036, 59,      'SIS_PBAR_SEQ_TS' );
INSERT INTO "sis" VALUES( 30036, 60,      'SIS_RECATCH_SEQ_START' );
INSERT INTO "sis" VALUES( 30036, 61,      'SIS_RECATCH_SEQ_TS' );
INSERT INTO "sis" VALUES( 30036, 62,      'SIS_ATOM_SEQ_START' );
INSERT INTO "sis" VALUES( 30036, 63,      'SIS_ATOM_SEQ_TS' );


INSERT INTO "sis" VALUES( 34513, 0,       'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 34513, 1,       'SIS_AD' );
INSERT INTO "sis" VALUES( 34513, 2,       '' );
INSERT INTO "sis" VALUES( 34513, 3,       'SIS_PMT_CATCH_OR' );
INSERT INTO "sis" VALUES( 34513, 4,       'SIS_PMT_ATOM_OR' );
INSERT INTO "sis" VALUES( 34513, 5,       'POS_TRANS' );
INSERT INTO "sis" VALUES( 34513, 6,       'IO32_TRIG' );
INSERT INTO "sis" VALUES( 34513, 7,       'IO32_TRIG_NOBUSY' );
INSERT INTO "sis" VALUES( 34513, 8,       'PMT1' );
INSERT INTO "sis" VALUES( 34513, 9,       'PMT2' );
INSERT INTO "sis" VALUES( 34513, 10,      'PMT3' );
INSERT INTO "sis" VALUES( 34513, 11,      'PMT4' );
INSERT INTO "sis" VALUES( 34513, 12,      'PMT5' );
INSERT INTO "sis" VALUES( 34513, 13,      'PMT6' );
INSERT INTO "sis" VALUES( 34513, 14,      'PMT7' );
INSERT INTO "sis" VALUES( 34513, 15,      'PMT8' );
INSERT INTO "sis" VALUES( 34513, 16,      'SIS_50Mhz_CLK' );
INSERT INTO "sis" VALUES( 34513, 17,      'SIS_PMT_7_AND_8' );
INSERT INTO "sis" VALUES( 34513, 18,      'SIS_PMT_5_AND_6' );
INSERT INTO "sis" VALUES( 34513, 19,      'SIS_PMT_1_AND_2' );
INSERT INTO "sis" VALUES( 34513, 20,      'SIS_PMT_3_AND_4' );
INSERT INTO "sis" VALUES( 34513, 21,      'SIS_PMT_CATCH_AND' );
INSERT INTO "sis" VALUES( 34513, 22,      'SIS_PMT_ATOM_AND' );
INSERT INTO "sis" VALUES( 34513, 23,      'PMT9' );
INSERT INTO "sis" VALUES( 34513, 24,      '' );
INSERT INTO "sis" VALUES( 34513, 25,      '' );
INSERT INTO "sis" VALUES( 34513, 26,      'SIS_RECATCH_DUMP_START' );
INSERT INTO "sis" VALUES( 34513, 27,      'SIS_RECATCH_DUMP_STOP' );
INSERT INTO "sis" VALUES( 34513, 28,      'SIS_ATOM_DUMP_START' );
INSERT INTO "sis" VALUES( 34513, 29,      'SIS_ATOM_DUMP_STOP' );
INSERT INTO "sis" VALUES( 34513, 30,      'SIS_PBAR_DUMP_START' );
INSERT INTO "sis" VALUES( 34513, 31,      'SIS_PBAR_DUMP_STOP' );

INSERT INTO "sis" VALUES( 34513, 32,      'SIS_10Mhz_CLK' );
INSERT INTO "sis" VALUES( 34513, 33,      'SIS_AD_FAKE' );
INSERT INTO "sis" VALUES( 34513, 34,      '' );
INSERT INTO "sis" VALUES( 34513, 35,      '' );
INSERT INTO "sis" VALUES( 34513, 36,      '' );
INSERT INTO "sis" VALUES( 34513, 37,      '' );
INSERT INTO "sis" VALUES( 34513, 38,      '' );
INSERT INTO "sis" VALUES( 34513, 39,      '' );
INSERT INTO "sis" VALUES( 34513, 40,      'PMT_10' );
INSERT INTO "sis" VALUES( 34513, 41,      'PMT_11' );
INSERT INTO "sis" VALUES( 34513, 42,      'PMT_10_AND_PMT_11' );
INSERT INTO "sis" VALUES( 34513, 43,      '' );
INSERT INTO "sis" VALUES( 34513, 44,      '' );
INSERT INTO "sis" VALUES( 34513, 45,      '' );
INSERT INTO "sis" VALUES( 34513, 46,      '' );
INSERT INTO "sis" VALUES( 34513, 47,      '' );
INSERT INTO "sis" VALUES( 34513, 48,      '' );
INSERT INTO "sis" VALUES( 34513, 49,      '' );
INSERT INTO "sis" VALUES( 34513, 50,      '' );
INSERT INTO "sis" VALUES( 34513, 51,      '' );
INSERT INTO "sis" VALUES( 34513, 52,      'SIS_DIX_ALPHA' );
INSERT INTO "sis" VALUES( 34513, 53,      '' );
INSERT INTO "sis" VALUES( 34513, 54,      '' );
INSERT INTO "sis" VALUES( 34513, 55,      '' );
INSERT INTO "sis" VALUES( 34513, 56,      '' );
INSERT INTO "sis" VALUES( 34513, 57,      'SIS_AD_2' );
INSERT INTO "sis" VALUES( 34513, 58,      'SIS_PBAR_SEQ_START' );
INSERT INTO "sis" VALUES( 34513, 59,      'SIS_PBAR_SEQ_TS' );
INSERT INTO "sis" VALUES( 34513, 60,      'SIS_RECATCH_SEQ_START' );
INSERT INTO "sis" VALUES( 34513, 61,      'SIS_RECATCH_SEQ_TS' );
INSERT INTO "sis" VALUES( 34513, 62,      'SIS_ATOM_SEQ_START' );
INSERT INTO "sis" VALUES( 34513, 63,      'SIS_ATOM_SEQ_TS' );


INSERT INTO "sis" VALUES( 35605, 36,      'MIXING_FLAG' );
INSERT INTO "sis" VALUES( 35605, 35,      'QUENCH_FLAG' );


INSERT INTO "sis" VALUES( 35880, 44,      'PMT_12' );
INSERT INTO "sis" VALUES( 35880, 45,      'PMT_13' );
INSERT INTO "sis" VALUES( 35880, 46,      'PMT_12_AND_13' );

INSERT INTO "sis" VALUES( 37043, 25,      'SIS_VF48_CLOCK' );
INSERT INTO "sis" VALUES( 38080, 24,      'SEQLASERPULSE');
INSERT INTO "sis" VALUES( 38080, 53,      'QPULSE' );
INSERT INTO "sis" VALUES( 38080, 54,      'LASER_SHUTTER_OPEN' );
INSERT INTO "sis" VALUES( 38080, 55,      'LASER_SHUTTER_CLOSE' );
INSERT INTO "sis" VALUES( 38131, 54,      '' );
INSERT INTO "sis" VALUES( 38131, 55,      '' );
INSERT INTO "sis" VALUES( 38900, 39,      'SIS_DIX_WALPHA' );
INSERT INTO "sis" VALUES( 44104, 23,      '' );
INSERT INTO "sis" VALUES( 44104, 40,      'PMT9' );
INSERT INTO "sis" VALUES( 44104, 41,      '' );
INSERT INTO "sis" VALUES( 44104, 42,      '' );
INSERT INTO "sis" VALUES( 44200, 40,      'PMT_10' );
INSERT INTO "sis" VALUES( 44200, 41,      'PMT_11' );
INSERT INTO "sis" VALUES( 44200, 42,      'PMT_10_AND_PMT_11' );
INSERT INTO "sis" VALUES( 47352, 56,      'SiPM_TEST' );
INSERT INTO "sis" VALUES( 48720, 37,      'MIC_SYNTH_STEP_START' );
INSERT INTO "sis" VALUES( 48720, 38,      'MIC_SYNTH_STEP_STOP' );

INSERT INTO "sis" VALUES( 54338, 2,       'ALPHA_G_SiPM_TEST' );
INSERT INTO "sis" VALUES( 54339, 34,      'ALPHA_G_SiPM_TEST_AND_SIS_PMT_5_AND_6' );

/* For some reason these cables got switched some time between run 54280 54452*/
INSERT INTO "sis" VALUES( 54452, 37,      'MIC_SYNTH_STEP_STOP' );
INSERT INTO "sis" VALUES( 54452, 38,      'MIC_SYNTH_STEP_START' );

/* Move ALPHA_G_SiPM_TEST to make space for positron dumps */
INSERT INTO "sis" VALUES( 56538, 2,        '' );
INSERT INTO "sis" VALUES( 56538, 47,       'ALPHA_G_SiPM_TEST' );
INSERT INTO "sis" VALUES( 56539, 4,        'SIS_POS_DUMP_START' );
INSERT INTO "sis" VALUES( 56539, 5,        'SIS_POS_DUMP_STOP' );
INSERT INTO "sis" VALUES( 56539, 2,        'SIS_PMT_ATOM_OR' );  /*Atom OR Moves channel*/
INSERT INTO "sis" VALUES( 56539, 43,       'SIS_POS_SEQ_START' );
INSERT INTO "sis" VALUES( 56757, 34,       'POS_TRANS' );

/* For some reason these cables got switched AGAIN! time around run 56900 */
INSERT INTO "sis" VALUES( 56900, 37,      'MIC_SYNTH_STEP_START' );
INSERT INTO "sis" VALUES( 56900, 38,      'MIC_SYNTH_STEP_STOP' );


CREATE TRIGGER timeEnter_dirtable AFTER  INSERT ON dir_table
BEGIN

UPDATE dir_table SET timeEnter = DATETIME('NOW')
         WHERE rowid = new.rowid;
END;
CREATE TRIGGER timeEnter_vf48 AFTER  INSERT ON vf48
BEGIN

UPDATE vf48 SET timeEnter = DATETIME('NOW')
         WHERE rowid = new.rowid;
END;
COMMIT;
