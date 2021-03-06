#!/bin/bash

#agana.exe -O$DATADIR/agmini/cosmics904501vis.root $MIDASDATA/run904501sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cyl_l2.5.json --location TRIUMF --Bfield 0 | tee RunLogs/R904501sub000.log

#
#gdb -ex="b $AGRELEASE/aged/src/AgedWindow.cxx:180" -ex="b /usr/src/debug/libXt-1.1.5/src/Resources.c:474" --ex=r --args agana.exe -O$DATADIR/agmini/cosmics904501vis.root $MIDASDATA/run904501sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cyl_l2.5.json --location TRIUMF --Bfield 0 
# also  Resources.c:815
gdb --ex=r --args agana.exe -O$DATADIR/agmini/cosmics904501vis.root $MIDASDATA/run904501sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cyl_l2.5.json --location TRIUMF --Bfield 0 

#agana.exe -O$AGRELEASE/cosmics3873vis.root $AGMIDASDATA/run03873sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cosm.json --Bfield 0
