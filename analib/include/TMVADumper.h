
#include "generalizedspher.h"

#include "TTree.h"
#include "TMVA/Reader.h"

class TMVADumper
{
   public:
   TTree* fTree;
   TMVADumper(TTree* tree)
   {
      fTree = tree;
   }
   virtual void LoadVariablesToReader(TMVA::Reader* reader) = 0;

   void Fill()
   {
      fTree->Fill();
   }
};

