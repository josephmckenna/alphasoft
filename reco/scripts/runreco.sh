#!/bin/bash

#{ time $AGRELEASE/build/reco/agreco.exe --rootfile $DATADIR/output/cosmics3873_cyl.root --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 ; }&> RecoR3873.log&
#{ time $AGRELEASE/build/reco/agreco.exe --rootfile $DATADIR/output/cosmics3879_cyl.root --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 ; }&> RecoR3879.log&

#{ time $AGRELEASE/reco/agreco.exe --rootfile $DATADIR/output/cosmics3864.root --anasettings $AGRELEASE/ana/cosm.json ; }&> RecoR3864.log&

#{ time $AGRELEASE/build/reco/agreco.exe --rootfile $DATADIR/test/cosmics904139.root --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 ; }&> Reco904139.log&
time $AGRELEASE/build/reco/agreco.exe --rootfile $DATADIR/test/cosmics904139.root --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 | tee Reco904139.log
