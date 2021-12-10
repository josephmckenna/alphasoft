Int_t FindBestChannel(Int_t runNumber, const char* description, Int_t repetition=0) 
{
  // AUTOMATICALLY FINDS THE GOOD CHANNEL, i.e. channel with most counts 
  
  //Get time range values
  std::vector<TA2Spill> spills = Get_A2_Spills(runNumber, {description}, {repetition});

  Double_t tmin = spills.front().GetStartTime();
  Double_t tmax = spills.front().GetStopTime();
  //Prepare reading data from root tree
  TSISChannels* sisch = new TSISChannels(runNumber);

  int possible_channels[5] = {
    42, 
    sisch->GetChannel("SIS_PMT_ATOM_OR"),
    sisch->GetChannel("SIS_PMT_ATOM_AND"),
    sisch->GetChannel("SIS_PMT_CATCH_OR"), 
    sisch->GetChannel("SIS_PMT_CATCH_AND")
  } ; 
  
  int highest_channel = -1;
  int highest_counts = 0;
  
  for (int i = 0; i < 5; i++) {
    int ch = possible_channels[i];
    int ch_counts = Count_SIS_Triggers(runNumber, ch, {tmin}, {tmax});
    
    printf("channel %d has %d counts\n", ch, ch_counts);
    
    if (ch_counts > highest_counts) {
      highest_counts = ch_counts;
      highest_channel = ch;
    }
  }
    
  if (highest_counts == 0) {
    printf("none of the channels seem to work!");
    return -1; 
  }
  return highest_channel;   
}

double Lifetime(int runNumber)
{
    std::vector<TA2Spill> lifetimeSpills =  Get_A2_Spills(runNumber,{"Lifetime"},{-1});
    if(lifetimeSpills.size()>0)
    {
        std::cout << "Lifetime dump detected..." << std::endl;
        std::vector<TA2Spill> coldDumpSpills =  Get_A2_Spills(runNumber,{"Cold Dump"},{-1});
        std::vector<TA2Spill> fifthDumpSpills =  Get_A2_Spills(runNumber,{"Fifth Dump"},{-1});
        if(coldDumpSpills.size()>=2 && fifthDumpSpills.size()>=2)
        {
            std::cout << "Cold dumps detected. Calculating lifetime..." << std::endl;
            double lifetimeDumpLength = lifetimeSpills.at(0).GetStopTime() - lifetimeSpills.at(0).GetStartTime();
            //GetSISTimeAndCounts(runNumber, "SIS_PMT_CATCH_OR", lifetimeSpills);

            //Int_t channel = GetSISChannel(runNumber, "SIS_PMT_CATCH_OR");
            //Int_t channel = GetSISChannel(runNumber, "CT_SiPM_OR");
            Int_t channel = FindBestChannel(runNumber, "Cold Dump");
            double coldDump0 = Count_SIS_Triggers(runNumber, channel, {coldDumpSpills.at(0).GetStartTime()}, {coldDumpSpills.at(0).GetStopTime()});
            double FifthDump0 = Count_SIS_Triggers(runNumber, channel, {fifthDumpSpills.at(0).GetStartTime()}, {fifthDumpSpills.at(0).GetStopTime()});
            double coldDump1 = Count_SIS_Triggers(runNumber, channel, {coldDumpSpills.at(1).GetStartTime()}, {coldDumpSpills.at(1).GetStopTime()});
            double FifthDump1 = Count_SIS_Triggers(runNumber, channel, {fifthDumpSpills.at(1).GetStartTime()}, {fifthDumpSpills.at(1).GetStopTime()});

            std::cout << "Cold Dump 0 counts = " << coldDump0 << ". Fifth Dump 0 counts = " << FifthDump0 << std::endl;
            std::cout << "Cold Dump 1 counts = " << coldDump1 << ". Fifth Dump 1 counts = " << FifthDump1 << std::endl;
            double normalised0 = coldDump0/FifthDump0;
            double normalised1 = coldDump1/FifthDump1;
            std::cout << "Normalised 0 = " << normalised0 << " and normalised 1 = " << normalised1<< std::endl;
            double logfactor = ( normalised0 / normalised1 );
            std::cout << "Log factor = " << logfactor << std::endl;
            std::cout << "Log factor after log = " << TMath::Log( logfactor )  << std::endl;
            std::cout << "2000/logfactor = " << 2000/TMath::Log( (coldDump0/FifthDump0) / (coldDump1/FifthDump0) ) << std::endl;

            double lifetime = lifetimeDumpLength / TMath::Log( logfactor );
            double lifetimeInMins = lifetime/60;

            std::cout << "Lifetime = " << lifetimeInMins << "m" << std::endl;

            return lifetimeInMins;


        }
        else
        {
            std::cout << "Lifetime dump found but not enough matching cold dumps and/or fifth dumps for the lifetime calculation." << std::endl;
        }
    }
    return -1;
}

