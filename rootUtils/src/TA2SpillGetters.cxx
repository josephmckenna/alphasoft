#include "TA2SpillGetters.h"

std::vector<TA2Spill*> Get_A2_Spills(int runNumber, std::vector<std::string> description, std::vector<int> repetition)
{

   // The TA2Spill Tree is small... 
   TTreeReader* reader=Get_A2SpillTree(runNumber);
   TTreeReaderValue<TA2Spill> spill(*reader, "TA2Spill");

   assert(description.size()==repetition.size());
   int match_counter[description.size()];
   for (size_t i=0; i<description.size(); i++)
   {
      match_counter[i]=0;
      //Dump markers are surrounded by quote marks...
      description[i].insert(0,"\"");
      description[i].push_back('"');
   }

   std::vector<TA2Spill*> spills;

   while (reader->Next())
   {
      //std::cout<<"Name:"<<spill->Name.c_str()<<std::endl;
      for (size_t i=0; i<description.size(); i++)
      {
         if (strcmp(description[i].c_str(),spill->Name.c_str())==0)
         {
            //std::cout<<"Match dump found:"<<description[i].c_str()<<"=="<<spill->Name.c_str()<<std::endl;
            //If we asked for all repetitions

            //spill->Print();

            //This TTreeReader value is odd... the dereferencing 
            //overload * is doing something special... so I need to 
            //dereference then get the pointer... then type cast it...

            TA2Spill* b=new TA2Spill((const TA2Spill*) &(*spill));
            //b->Print();
            if (repetition.at(i)<0)
               //Copy spill into returned vector
               spills.push_back(b); //This cast is sketchy...
            else if (repetition.at(i)==match_counter[i]++)
               spills.push_back(b);
            else
               continue;
         } 
      }
   }

   return spills;
}
