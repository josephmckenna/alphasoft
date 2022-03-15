TString tag("_R");
int RunNumber=0;

void PrintSeqEvent( TSeq_Event* anEvent)
{
  cout<<"Seq: "<<anEvent->GetSeq()<<" Num: "<<anEvent->GetSeqNum()<<" ID: "<<anEvent->GetID()<<" head: "<<anEvent->GetSeqHeader()<<endl;
  cout<<"\tEvent Name: "<<anEvent->GetEventName()<<" Event Description: "<<anEvent->GetDescription()<<endl;
  cout<<"\ton cnt: "<<anEvent->GetonCount()<<" on stt: "<<anEvent->getonState()<<endl;
}

void ProcessTree( TTree* tin )
{
  TSeq_Event* SeqEvent = new TSeq_Event;
  tin->SetBranchAddress("SequencerEvent", &SeqEvent);
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      SeqEvent->Reset();
      tin->GetEntry(e);
      PrintSeqEvent( SeqEvent );
    }
}


void ProcessData()
{
  TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();
  TString fname(fin->GetName());
  cout<<fname<<" FOUND"<<endl;
  RunNumber = TString(fname(6,5)).Atoi();
  tag+=RunNumber;

  TTree* tin = (TTree*) fin->Get("SequencerEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  ProcessTree( tin );
}


void PrintSeqTree()
{
  cout<<"This is a test"<<endl;
  ProcessData();
}
