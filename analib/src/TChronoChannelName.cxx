#include "TChronoChannelName.h"

ClassImp(TChronoChannelName)

#include "mvodb.h"

#ifdef INCLUDE_MVODB_H
//New manalyzer uses VirtualODB (after Jan 2020)
TChronoChannelName::TChronoChannelName(MVOdb* Odb, const std::string& board)
{
   SetBoard(board);
   std::string OdbPath="Equipment/" + board  + "/Settings/names";
   fName.reserve(60);
   Odb->RSA(OdbPath.c_str(),&fName,true,60,16);
   for (const auto& n: fName)
       std::cout<<"\t"<<n<<std::endl;
         //std::cout<<"MVODB "<<OdbPath<<" ch: "<<chan<<std::endl;
         //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
         //if (Odb->odbReadString(OdbPath.Data(),chan))
   
}
#endif
TChronoChannelName::TChronoChannelName(): fName(CHRONO_N_CHANNELS)
{
   fChronoBoardName = "";
}

TChronoChannelName::TChronoChannelName(TString json, const std::string& b): fName(CHRONO_N_CHANNELS)
{
   TString ChannelJSON;
   int last;
   if (json.CountChar('.')==2)
      for (int i=0; i<2; i++) //Strip 2 sets of '.' (ie .boarno.json)
         {
            last=json.Last('.');
            json=json(0,last);
         }
   //Re-add extension for this module
   json+=".";
   json+=b;
   json+=".json";
   
   std::cout <<"Loading Channel name list from file: "<<json<<std::endl;
   std::ifstream file;
   file.open(json);
   if (!file)
   {
      std::cerr<<"TChronoChannelName cannot find json file: "<<json<<std::endl;
      exit(12);
   }
   ChannelJSON.ReadFile(file);
   file.close();
   std::cout <<ChannelJSON<<std::endl;
   TChronoChannelName* me=NULL;//new TChronoChannelName();
   
   #if ROOT_VERSION_CODE > ROOT_VERSION(6,14,0)
   TBufferJSON::FromJSON(me,ChannelJSON);
   #else
   std::cout <<"Please update to atleast root 6.14 so that TBufferJSON is supported"<<std::endl;
   exit(1);
   #endif
   for (int i=0; i<CHRONO_N_CHANNELS; i++)
      fName[i]=me->GetChannelName(i);
   fChronoBoardName = me->GetBoardName();
   //me->Print();
   delete me;
   //Print();
}

TChronoChannelName::~TChronoChannelName()
{
   fName.clear();
}

void TChronoChannelName::DumpToJson(int runno)
{
   #if ROOT_VERSION_CODE > ROOT_VERSION(6,14,0)
   TString json = TBufferJSON::ToJSON(this);
   std::ofstream stream;
   TString fname="chrono/R";
   fname+=runno;
   fname+=".";
   fname+=fChronoBoardName;
   fname+=".json";
   stream.open(fname);
   if( !stream )
      std::cout << "Opening file failed" << std::endl;
   // use operator<< for clarity
   stream << json<<std::endl;
   stream.close();
   #else
   std::cout <<"Please update to atleast root 6.14 so that TBufferJSON is supported"<<std::endl;
   exit(1);
   #endif
   
}

void TChronoChannelName::Print()
{
   std::cout<<"Board Index:\t"<<fChronoBoardName<<std::endl;
   for (int i = 0; i < CHRONO_N_CHANNELS; i++)
      std::cout<<i<<": "<<fName[i] <<std::endl;
}

Int_t TChronoChannelName::GetChannel(std::string ChannelName, const bool exact_match) const
{
   if (!exact_match)
   {
      for (size_t i=0; i<CHRONO_N_CHANNELS; i++)
      {
         //std::cout <<fName[i]<<std::endl;
         //std::string doesn't have this functionality until C++20 :(
         if (TString(fName[i]).BeginsWith(ChannelName)) return i;
      }
   }
   else
   {
      for (size_t i=0; i < CHRONO_N_CHANNELS; i++)
      {
         //std::cout <<fName[i]<<std::endl;
         if ( i >= fName.size() )
            continue;
         if (fName[i] == ChannelName) return i;
      }
   }
   return -1;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
