
#ifdef BUILD_A2
#include "TA2SpillGetters.h"


//Welp, this is broken in jupyter... why?
//----> 4 ROOT.Get_A2_Spillsa(45000,["Mixing"],[0])
//
//SystemError: vector<TA2Spill> ::Get_A2_Spillsa(int runNumber, _object* description, _object* repetition) =>
//    problem in C++; program state has been reset

#ifdef HAVE_PYTHON
std::vector<TA2Spill> Get_A2_Spills(int runNumber, PyObject* description, PyObject* repetition)
{
    std::vector<std::string> desc = listTupleToVector_String(description);
    std::vector<int> reps= listTupleToVector_Int( repetition);
    return Get_A2_Spills(runNumber,desc, reps);
}
#endif

std::vector<TA2Spill> Get_A2_Spills(int runNumber, std::vector<std::string> description, std::vector<int> repetition)
{

   // The TA2Spill Tree is small... 
   TTreeReader* reader=Get_A2SpillTree(runNumber);
   TTreeReaderValue<TA2Spill> spill(*reader, "TA2Spill");

   assert(description.size()==repetition.size());
   int match_counter[description.size()];

   std::vector<TA2Spill> spills;

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

#endif
