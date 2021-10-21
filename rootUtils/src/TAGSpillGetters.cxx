
#ifdef BUILD_AG
#include "TAGSpillGetters.h"

std::vector<TAGSpill> Get_AG_Spills(int runNumber, std::vector<std::string> description, std::vector<int> repetition)
{

   // The TAGSpill Tree is small... 
   TTreeReader* reader=Get_AGSpillTree(runNumber);
   TTreeReaderValue<TAGSpill> spill(*reader, "TAGSpill");

   assert(description.size()==repetition.size());
   int match_counter[description.size()];
   for (size_t i=0; i<description.size(); i++)
      match_counter[i]=0;

   std::vector<TAGSpill> spills;

   while (reader->Next())
   {
      //std::cout<<"Name:"<<spill->Name.c_str()<<std::endl;
      for (size_t i=0; i<description.size(); i++)
      {
         if (spill->IsMatchForDumpName(description[i]))
         {
            //spill->Print();
            //This TTreeReader value is odd... the dereferencing 
            //overload * is doing something special... so I need to 
            //dereference then get the pointer... then type cast it...
            //b->Print();
            if (repetition.at(i)<0)
               //Copy spill into returned vector
               spills.push_back(*spill); //This cast is sketchy...
            else if (repetition.at(i)==match_counter[i]++)
               spills.push_back(*spill);
            else
               continue;
         } 
      }
   }
   return spills;
}

std::vector<TAGSpill> Get_All_AG_Spills(int runNumber)
{
   return Get_AG_Spills(runNumber,{"*"},{-1});
}

#endif
