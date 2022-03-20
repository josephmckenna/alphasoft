#include "TreeGetters.h"

TTree* Get_Tree_By_Name(const int runNumber,const char* name)
{
   TFile* f=Get_File(runNumber);
   TTree *tree = NULL;
   tree = (TTree *)f->Get(name);
   if (tree == NULL)
   {
      Error(name, "\033[31mTree for run number %d not found\033[00m", runNumber);
      tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
   }
   return tree;
}

TTreeReader* Get_TreeReader_By_Name(const int runNumber, const char* name)
{
   TFile* f=Get_File(runNumber);
   return new TTreeReader(name,f);
}


#ifdef BUILD_AG

TTreeReader* Get_AGSpillTree(Int_t runNumber)
{
   return Get_TreeReader_By_Name(runNumber, "AGSpillTree");
}

#endif
#ifdef BUILD_AG
TTree* Get_Chrono_Tree(const int runNumber, const std::string ChronoBoardChannel)
{
   std::string Name="ChronoBox_" + ChronoBoardChannel;
   std::cout<<"Loading "<<Name<<std::endl;

   if (ChronoBoardChannel == "cb01")
   {
      std::cout<<"AT TIME OF WRITING cb01 is not useful for FIFO analysis.... skipping"<<std::endl;
      return NULL;
   }

   TTree* t=Get_Tree_By_Name(runNumber,Name.c_str());
   return t;
}
#endif
#ifdef BUILD_AG
TTreeReader* Get_Chrono_TreeReader(const int runNumber, const std::string ChronoBoardChannel)
{
   std::string Name="ChronoBox_" + ChronoBoardChannel;
   std::cout<<"Loading "<<Name<<std::endl;

   if (ChronoBoardChannel == "cb01")
   {
      std::cout<<"AT TIME OF WRITING cb01 is not useful for FIFO analysis.... skipping"<<std::endl;
      return NULL;
   }

   return Get_TreeReader_By_Name(runNumber,Name.c_str());
}
#endif
#ifdef BUILD_AG
TTree* Get_Chrono_Name_Tree(const int runNumber)
{
   return Get_Tree_By_Name(runNumber,"ChronoBoxChannels");
}
#endif

TTree* Get_Seq_Event_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"SequencerEventTree");
}

TTree* Get_Seq_State_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"SequencerStateTree");
}

#ifdef BUILD_AG
TTree* Get_StoreEvent_Tree(const int runNumber)
{
   return Get_Tree_By_Name(runNumber,"StoreEventTree");
}

TTreeReader* Get_StoreEvent_TreeReader(const int runNumber)
{
   return Get_TreeReader_By_Name(runNumber,"StoreEventTree");
}

TTreeReader* Get_AGDetectorEvent_TreeReader(const int runNumber)
{
   return Get_TreeReader_By_Name(runNumber,"TADetectorEventTree");
}
#endif

#ifdef BUILD_A2
// ALPHA 2 Getters:
TTreeReader* A2_SIS_Tree_Reader(Int_t runNumber, Int_t Module_Number)
{
   TFile* f=Get_File(runNumber);
   std::string TreeName = "SIS" + std::to_string(Module_Number)+ std::string("Tree");
   TTreeReader* t=new TTreeReader(TreeName.c_str(), f);
   return t;
}

TTreeReader* Get_A2_SVD_Tree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTreeReader* t=new TTreeReader("SVDOfficialA2Time",f);
   return t;
}
TTreeReader* Get_A2SpillTree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTreeReader* t=new TTreeReader("A2SpillTree", f);
   return t;
}

TTreeReader* Get_TA2AnalysisReport_Tree(Int_t runNumber)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d=f->GetDirectory("AnalysisReport");
   TTreeReader* t = new TTreeReader("AnalysisReport", f->GetDirectory("/AnalysisReport"));
   return t;
}

#endif

#ifdef BUILD_AG

TTreeReader* Get_TAGAnalysisReport_Tree(Int_t runNumber)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d=f->GetDirectory("AnalysisReport");
   TTreeReader* t = new TTreeReader("AnalysisReport", f->GetDirectory("/AnalysisReport"));
   return t;
}

#endif

TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& Category, const std::string& Varname)
{
   std::string CombinedName = Category + "\\" + Varname;
   return Get_feGEM_Tree(runNumber,CombinedName);
}

TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& CombinedName)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d=f->GetDirectory("feGEM");
   TTreeReader* t = new TTreeReader(CombinedName.c_str(), f->GetDirectory("/feGEM"));
   return t;
}

TTreeReader* Get_feLV_Tree(Int_t runNumber, const std::string& BankName)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d=f->GetDirectory("felabview");
   TTreeReader* t = new TTreeReader(BankName.c_str(), f->GetDirectory("/felabview"));
   return t;
}

std::vector<TTreeReader*> Get_feGEM_File_Trees(Int_t runNumber, const std::string& CombinedName)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d = f->GetDirectory("feGEM");
   TList* aa = d->GetListOfKeys();
   std::vector<TTreeReader*> trees;
   for (int i=0; i<aa->GetEntries(); i++)
   {
      if (strncmp(CombinedName.c_str(),aa->At(i)->GetName(),CombinedName.size())==0)
      {
         TTreeReader* t = new TTreeReader(aa->At(i)->GetName(), f->GetDirectory("/feGEM"));
         trees.push_back(t);
      }
   }
   return trees;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
