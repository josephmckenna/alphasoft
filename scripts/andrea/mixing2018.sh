#!/bin/bash

ls -lh $AGMIDASDATA/run03839sub*.mid.lz4 $AGMIDASDATA/run03840sub*.mid.lz4 $AGMIDASDATA/run03846sub*.mid.lz4 $AGMIDASDATA/run03847sub*.mid.lz4 $AGMIDASDATA/run03848sub*.mid.lz4 $AGMIDASDATA/run03837sub*.mid.lz4 $AGMIDASDATA/run03841sub*.mid.lz4 $AGMIDASDATA/run03842sub*.mid.lz4 $AGMIDASDATA/run03843sub*.mid.lz4 $AGMIDASDATA/run03844sub*.mid.lz4 $AGMIDASDATA/run03834sub*.mid.lz4 $AGMIDASDATA/run03838sub*.mid.lz4 $AGMIDASDATA/run03836sub*.mid.lz4

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#=======================================================================================================================================================================================================================================

agana.exe -O$DATADIR/cosmics/pbar3839.root --mt $AGMIDASDATA/run03839sub*.mid.lz4 -- --usetimerange 402.632 504.094 --stopunpackafter 504.094 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3893cosm2.log &
pids[0]=$!
agana.exe -O$DATADIR/cosmics/pbar3840.root --mt $AGMIDASDATA/run03840sub*.mid.lz4 -- --usetimerange 362.451 463.903 --stopunpackafter 463.903 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3840cosm2.log &
pids[1]=$!
agana.exe -O$DATADIR/cosmics/pbar3846.root --mt $AGMIDASDATA/run03846sub*.mid.lz4 -- --usetimerange 220.73 277.261 --stopunpackafter 277.261 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3846cosm2.log &
pids[2]=$!
agana.exe -O$DATADIR/cosmics/pbar3847.root --mt $AGMIDASDATA/run03847sub*.mid.lz4 -- --usetimerange 183.144 240.437 --stopunpackafter 240.437 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3847cosm2.log &
pids[3]=$!
agana.exe -O$DATADIR/cosmics/pbar3848.root --mt $AGMIDASDATA/run03848sub*.mid.lz4 -- --usetimerange 114.77 176.278 --stopunpackafter 176.278 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3848cosm2.log &
pids[4]=$!
agana.exe -O$DATADIR/cosmics/pbar3837.root --mt $AGMIDASDATA/run03837sub*.mid.lz4 -- --usetimerange 272.363 281.212 --stopunpackafter 281.212 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3837cosm2.log &
pids[5]=$!
agana.exe -O$DATADIR/cosmics/pbar3841.root --mt $AGMIDASDATA/run03841sub*.mid.lz4 -- --usetimerange 291.799 301.021 --stopunpackafter 301.021 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3841cosm2.log &
pids[6]=$!
agana.exe -O$DATADIR/cosmics/pbar3842.root --mt $AGMIDASDATA/run03842sub*.mid.lz4 -- --usetimerange 295.799 304.806 --stopunpackafter 304.806 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3842cosm2.log &
pids[7]=$!
agana.exe -O$DATADIR/cosmics/pbar3843.root --mt $AGMIDASDATA/run03843sub*.mid.lz4 -- --usetimerange 287.513 296.439 --stopunpackafter 296.439 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3843cosm2.log &
pids[8]=$!
agana.exe -O$DATADIR/cosmics/pbar3844.root --mt $AGMIDASDATA/run03844sub*.mid.lz4 -- --usetimerange 266.52 275.44 --stopunpackafter 275.44 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3844cosm2.log &
pids[9]=$!
agana.exe -O$DATADIR/cosmics/pbar3834.root --mt $AGMIDASDATA/run03834sub*.mid.lz4 -- --usetimerange 275.094 275.94 --stopunpackafter 275.94 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3834cosm2.log &
pids[10]=$!
agana.exe -O$DATADIR/cosmics/pbar3838.root --mt $AGMIDASDATA/run03838sub*.mid.lz4 -- --usetimerange 253.818 262.934 --stopunpackafter 262.934 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3838cosm2.log &
pids[11]=$!
agana.exe -O$DATADIR/cosmics/pbar3836.root --mt $AGMIDASDATA/run03836sub*.mid.lz4 -- --usetimerange 317.849 318.728 --stopunpackafter 318.728 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3836cosm2.log &
pids[12]=$!

for pid in ${pids[*]}; do
    wait $pid
done

MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3839.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3840.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3846.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3847.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3848.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3837.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3841.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3842.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3843.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3844.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3834.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3838.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3836.root --plot 0 --save 1 &> /dev/null &

#=======================================================================================================================================================================================================================================


agana.exe -O$DATADIR/output/pbar3839.root --mt $AGMIDASDATA/run03839sub*.mid.lz4 -- --usetimerange 402.632 504.094 --stopunpackafter 504.094 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3893cosm.log &
pids[0]=$!
agana.exe -O$DATADIR/output/pbar3840.root --mt $AGMIDASDATA/run03840sub*.mid.lz4 -- --usetimerange 362.451 463.903 --stopunpackafter 463.903 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3840cosm.log &
pids[1]=$!
agana.exe -O$DATADIR/output/pbar3846.root --mt $AGMIDASDATA/run03846sub*.mid.lz4 -- --usetimerange 220.73 277.261 --stopunpackafter 277.261 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3846cosm.log &
pids[2]=$!
agana.exe -O$DATADIR/output/pbar3847.root --mt $AGMIDASDATA/run03847sub*.mid.lz4 -- --usetimerange 183.144 240.437 --stopunpackafter 240.437 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3847cosm.log &
pids[3]=$!
agana.exe -O$DATADIR/output/pbar3848.root --mt $AGMIDASDATA/run03848sub*.mid.lz4 -- --usetimerange 114.77 176.278 --stopunpackafter 176.278 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3848cosm.log &
pids[4]=$!
agana.exe -O$DATADIR/output/pbar3837.root --mt $AGMIDASDATA/run03837sub*.mid.lz4 -- --usetimerange 272.363 281.212 --stopunpackafter 281.212 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3837cosm.log &
pids[5]=$!
agana.exe -O$DATADIR/output/pbar3841.root --mt $AGMIDASDATA/run03841sub*.mid.lz4 -- --usetimerange 291.799 301.021 --stopunpackafter 301.021 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3841cosm.log &
pids[6]=$!
agana.exe -O$DATADIR/output/pbar3842.root --mt $AGMIDASDATA/run03842sub*.mid.lz4 -- --usetimerange 295.799 304.806 --stopunpackafter 304.806 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3842cosm.log &
pids[7]=$!
agana.exe -O$DATADIR/output/pbar3843.root --mt $AGMIDASDATA/run03843sub*.mid.lz4 -- --usetimerange 287.513 296.439 --stopunpackafter 296.439 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3843cosm.log &
pids[8]=$!
agana.exe -O$DATADIR/output/pbar3844.root --mt $AGMIDASDATA/run03844sub*.mid.lz4 -- --usetimerange 266.52 275.44 --stopunpackafter 275.44 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3844cosm.log &
pids[9]=$!
agana.exe -O$DATADIR/output/pbar3834.root --mt $AGMIDASDATA/run03834sub*.mid.lz4 -- --usetimerange 275.094 275.94 --stopunpackafter 275.94 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3834cosm.log &
pids[10]=$!
agana.exe -O$DATADIR/output/pbar3838.root --mt $AGMIDASDATA/run03838sub*.mid.lz4 -- --usetimerange 253.818 262.934 --stopunpackafter 262.934 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3838cosm.log &
pids[11]=$!
agana.exe -O$DATADIR/output/pbar3836.root --mt $AGMIDASDATA/run03836sub*.mid.lz4 -- --usetimerange 317.849 318.728 --stopunpackafter 318.728 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3836cosm.log &
pids[12]=$!

for pid in ${pids[*]}; do
    wait $pid
done

MainEventTree.exe --rootfile $DATADIR/output/pbar3839.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3840.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3846.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3847.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3848.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3837.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3841.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3842.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3843.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3844.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3834.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3838.root --plot 0 --save 1 &> /dev/null &
MainEventTree.exe --rootfile $DATADIR/output/pbar3836.root --plot 0 --save 1 &> /dev/null &

#=======================================================================================================================================================================================================================================


agana.exe -O$DATADIR/CERN2018/pbar3839.root --mt $AGMIDASDATA/run03839sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3893wholecosm.log &
pids[0]=$!
agana.exe -O$DATADIR/CERN2018/pbar3840.root --mt $AGMIDASDATA/run03840sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3840wholecosm.log &
pids[1]=$!
agana.exe -O$DATADIR/CERN2018/pbar3846.root --mt $AGMIDASDATA/run03846sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3846wholecosm.log &
pids[2]=$!
agana.exe -O$DATADIR/CERN2018/pbar3847.root --mt $AGMIDASDATA/run03847sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3847wholecosm.log &
pids[3]=$!
agana.exe -O$DATADIR/CERN2018/pbar3848.root --mt $AGMIDASDATA/run03848sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3848wholecosm.log &
pids[4]=$!
agana.exe -O$DATADIR/CERN2018/pbar3837.root --mt $AGMIDASDATA/run03837sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3837wholecosm.log &
pids[5]=$!
agana.exe -O$DATADIR/CERN2018/pbar3841.root --mt $AGMIDASDATA/run03841sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3841wholecosm.log &
pids[6]=$!
agana.exe -O$DATADIR/CERN2018/pbar3842.root --mt $AGMIDASDATA/run03842sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3842wholecosm.log &
pids[7]=$!
agana.exe -O$DATADIR/CERN2018/pbar3843.root --mt $AGMIDASDATA/run03843sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3843wholecosm.log &
pids[8]=$!
agana.exe -O$DATADIR/CERN2018/pbar3844.root --mt $AGMIDASDATA/run03844sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3844wholecosm.log &
pids[9]=$!
agana.exe -O$DATADIR/CERN2018/pbar3834.root --mt $AGMIDASDATA/run03834sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3834wholecosm.log &
pids[10]=$!
agana.exe -O$DATADIR/CERN2018/pbar3838.root --mt $AGMIDASDATA/run03838sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3838wholecosm.log &
pids[11]=$!
agana.exe -O$DATADIR/CERN2018/pbar3836.root --mt $AGMIDASDATA/run03836sub*.mid.lz4 -- --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3836wholecosm.log &
pids[12]=$!

for pid in ${pids[*]}; do
    wait $pid
done

#=======================================================================================================================================================================================================================================
