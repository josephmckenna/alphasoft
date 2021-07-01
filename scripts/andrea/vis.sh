#!/bin/bash

cd $AGRELEASE/bin
pwd

#./agana.exe -O$DATADIR/test/cosmics904501vis.root $MIDASDATA/run904501sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cyl_l2.5.json --location TRIUMF --Bfield 0 |& tee $AGRELEASE/RunLogs/R904501sub000vis.log
#gdb --ex="set logging on" --ex=r --args ./agana.exe -O$DATADIR/test/cosmics904501vis.root $MIDASDATA/run904501sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cyl_l2.5.json --location TRIUMF --Bfield 0 

#agana.exe -O$DATADIR/test/cosmics3873vis.root $AGMIDASDATA/run03873sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cosm.json --Bfield 0

agana.exe -O$DATADIR/test/cosmics3586vis.root $AGMIDASDATA/run03586sub000.mid.lz4 -- --aged --anasettings $AGRELEASE/ana/cosm.json



cd $AGRELEASE
