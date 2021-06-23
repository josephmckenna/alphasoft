for run in `cat cosmicList`
do
    alphaStrips.exe  run${run}sub00000.mid.gz -- --EOS
    alphaAnalysis.exe run${run}sub00000.mid.gz -- --EOS --summarise Mixing
	macro_main_eventlist_generator.exe -- --runnumber --${run}
	a2dumper.exe run${run}sub00000.mid.gz -- --eventlist "eventlist${run}.list" --datalabel cosmic
done
macro_main_merger.exe -- --listfile cosmicList.list --cosmic


for run in `cat mixingList`
do
    alphaStrips.exe  run${run}sub00000.mid.gz -- --EOS
    alphaAnalysis.exe run${run}sub00000.mid.gz -- --EOS --summarise Mixing
	macro event selector ${run}eventlist
	a2dumper.exe run${run}sub00000.mid.gz -- --dumplist "${run}eventlist" --label mixing
done
macro_main_merger.exe -- --listfile mixingList --mixing