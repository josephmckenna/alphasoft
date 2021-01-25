import os


#This file is in AGRELEASE path... so set AGRELEASE accordingly
thisFilePath = os.path.dirname(os.path.abspath(__file__))
print("Setting AGRELEASE = " + thisFilePath )
os.environ['AGRELEASE'] = thisFilePath

os.environ['AGMIDASDATA'] = thisFilePath
os.environ['A2DATAPATH'] = thisFilePath + '/alpha2'

os.environ['AG_CFM'] = thisFilePath + '/ana'

# It can be used to tell the ROOTUTILS to fetch an output
# rootfile somewhere different from the default location
os.environ['AGOUTPUT'] = thisFilePath # this is the default location


os.environ['EOS_MGM_URL'] = 'root://eospublic.cern.ch'
os.environ['MCDATA'] = thisFilePath + '/simulation'

os.environ['PATH'] = os.environ['PATH']+":"+thisFilePath + "/bin"
os.environ['LD_LIBRARY_PATH'] = os.environ['LD_LIBRARY_PATH'] + ":" + thisFilePath + "/bin/lib"


#Set up run time environment (after project is compiled)
print("import ROOT")
import ROOT

# We can help the auto complete by including headers... not used for now
#ROOT.gInterpreter.ProcessLine('#include "my_header.h"')`

# Loading our libraries means we can interact with objects that we've stored in out root file
# Note, order matters a bit... librootUtils.so should be last as it depends on those before
for library in [ 'libaged.so', 'libagtpc.so', 'libanalib.so', 'libalpha2.so', 'librootUtils.so' ]:
	print("Loading lib: " + library)
	ROOT.gSystem.Load(thisFilePath + '/bin/lib/' + library)


ROOT.gInterpreter.ProcessLine('#define BUILD_A2 1')
ROOT.gInterpreter.ProcessLine('#define BUILD_AG 1')

#ROOT.gInterpreter.AddIncludePath(thisFilePath + '/bin/include/')
headers = os.listdir(thisFilePath + '/bin/include/')
print( 'Loading ' + str(len(headers)) + ' headers ' )
for header in headers:
	ROOT.gInterpreter.ProcessLine('#include "'  + header + '"' )
