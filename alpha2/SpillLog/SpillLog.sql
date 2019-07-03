
CREATE TABLE EquipmentName ( EquipmentID     INTEGER,
                             EquipmentName      TEXT,
                             PRIMARY KEY ( EquipmentID )
                             );
INSERT INTO "EquipmentName" VALUES( 0, 'ADBeam' );
INSERT INTO "EquipmentName" VALUES( 1, 'RW' );
INSERT INTO "EquipmentName" VALUES( 2, 'Microwave' );

CREATE TABLE LabviewString ( RunNumber       INTEGER,
                             EquipmentID     INTEGER,
                             SpillID         INTEGER,
                             StringData      TEXT,
                             UnixTime        INTEGER, 
                             PRIMARY KEY ( RunNumber, EquipmentID, SpillID)
                             FOREIGN KEY (EquipmentID) REFERENCES EquipmentName(EquipmentID)
                             );

CREATE TABLE LabviewDouble ( RunNumber   INTEGER,
                    EquipmentID          INTEGER,
                    SpillID              INTEGER,
                    ADCcount             REAL,
                    UnixTime             INTEGER,
                    RunTime              REAL ,
                    PRIMARY KEY (RunNumber ,EquipmentID, SpillID )
                    FOREIGN KEY (EquipmentID) REFERENCES EquipmentName(EquipmentID)
                    );


CREATE TABLE DumpTable ( RunNumber  INTEGER,
                    SeqNum          INTEGER,
                    DumpID          INTEGER,
                    DumpName            TEXT,
                    UnixTime        INTEGER,
                    StartTime       REAL,
                    StopTime        REAL,
                    DetectorCounts_00 INTEGER,
                    DetectorCounts_01 INTEGER,
                    DetectorCounts_02 INTEGER,
                    DetectorCounts_03 INTEGER,
                    DetectorCounts_04 INTEGER,
                    DetectorCounts_05 INTEGER,
                    DetectorCounts_06 INTEGER,
                    DetectorCounts_07 INTEGER,
                    DetectorCounts_08 INTEGER,
                    DetectorCounts_09 INTEGER,
                    DetectorCounts_10 INTEGER,
                    DetectorCounts_11 INTEGER,
                    DetectorCounts_12 INTEGER,
                    DetectorCounts_13 INTEGER,
                    DetectorCounts_14 INTEGER,
                    DetectorCounts_15 INTEGER,
                    DetectorCounts_16 INTEGER,
                    DetectorCounts_17 INTEGER,
                    DetectorCounts_18 INTEGER,
                    DetectorCounts_19 INTEGER,
                    DetectorCounts_20 INTEGER,
                    DetectorCounts_21 INTEGER,
                    DetectorCounts_22 INTEGER,
                    DetectorCounts_23 INTEGER,
                    DetectorCounts_24 INTEGER,
                    DetectorCounts_25 INTEGER,
                    DetectorCounts_26 INTEGER,
                    DetectorCounts_27 INTEGER,
                    DetectorCounts_28 INTEGER,
                    DetectorCounts_29 INTEGER,
                    DetectorCounts_30 INTEGER,
                    DetectorCounts_31 INTEGER,
                    DetectorCounts_32 INTEGER,
                    DetectorCounts_33 INTEGER,
                    DetectorCounts_34 INTEGER,
                    DetectorCounts_35 INTEGER,
                    DetectorCounts_36 INTEGER,
                    DetectorCounts_37 INTEGER,
                    DetectorCounts_38 INTEGER,
                    DetectorCounts_39 INTEGER,
                    DetectorCounts_40 INTEGER,
                    DetectorCounts_41 INTEGER,
                    DetectorCounts_42 INTEGER,
                    DetectorCounts_43 INTEGER,
                    DetectorCounts_44 INTEGER,
                    DetectorCounts_45 INTEGER,
                    DetectorCounts_46 INTEGER,
                    DetectorCounts_47 INTEGER,
                    DetectorCounts_48 INTEGER,
                    DetectorCounts_49 INTEGER,
                    DetectorCounts_50 INTEGER,
                    DetectorCounts_51 INTEGER,
                    DetectorCounts_52 INTEGER,
                    DetectorCounts_53 INTEGER,
                    DetectorCounts_54 INTEGER,
                    DetectorCounts_55 INTEGER,
                    DetectorCounts_56 INTEGER,
                    DetectorCounts_57 INTEGER,
                    DetectorCounts_58 INTEGER,
                    DetectorCounts_59 INTEGER,
                    DetectorCounts_60 INTEGER,
                    DetectorCounts_61 INTEGER,
                    DetectorCounts_62 INTEGER,
                    DetectorCounts_63 INTEGER,
                    PassedCuts        INTEGER,
                    PassMVA           INTEGER,
                    PRIMARY KEY( RunNumber , SeqNum , DumpID )
                      );

