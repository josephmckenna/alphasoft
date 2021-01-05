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
