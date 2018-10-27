#include "TSpill.h"
#include "TCanvas.h"
#include "TText.h"
#include "TList.h"


ClassImp( TSpill )


void TSpill::FormatADInfo(TString* log){

  struct tm  *ts;
  char       time[80];
  char buf[800];

  ts = localtime(this->GetTime());
  strftime(time, sizeof(time), "%H:%M:%S", ts);

  // append transformer value                                                                                                              
  if(fTransformer != fTransformer) //NaN
    sprintf(buf,"Spill %4d vetoed [%s]--------------------------------------->||    ", fNum, time);
  else
    sprintf(buf,"Spill %4d %0.3lf [%s]-------------------------------------------------------------------------------------------------------------------------------------------------->", fNum, fTransformer, time);
  *log += buf;

}




void TSpill::FormatDumpInfo(TString* log, TSeq_Dump* d, Bool_t indent=kFALSE){

  char buf[800];

  if (indent){
    *log += "   "; // indentation     
  }
  sprintf(buf,"[%8.3lf-%8.3lf]=%8.3lfs |",d->GetStartonTime(),d->GetStoponTime(),(d->GetStoponTime()-d->GetStartonTime())); // timestamps 
  *log += buf;

  if (d->GetSeqNum()==0)
    sprintf(buf," %-65s|",d->GetDescription().Data()); // description 
  else if (d->GetSeqNum()==1)
    sprintf(buf," %-16s%-49s|","",d->GetDescription().Data()); // description 
  else if (d->GetSeqNum()==2)
    sprintf(buf," %-32s%-33s|","",d->GetDescription().Data()); // description 
  else if (d->GetSeqNum()==3)
    sprintf(buf," %-48s%-17s|","",d->GetDescription().Data()); // description 
  else
    sprintf(buf," UnknownSequencer-%-55s|",d->GetDescription().Data()); // description 
    
    
  *log += buf;



  for (int iDet = 0; iDet<fNDet; iDet++){
    sprintf(buf,"%9d ",d->GetDetIntegral(iDet));
    *log += buf;
  }
  *log += "";

}



TSpill::TSpill()
{
  fYStep = 0.05;
  fNDet = MAXDET;
}

TSpill::TSpill( Int_t runnumber, Int_t num, time_t time, int ndet)
{
  fYStep = 0.05;
  fNDet = ndet;
  fRunNumber = runnumber;
  fNum = num;
  fTransformer = 0;
  fIsMessage = kFALSE;
  fTime = time;
}

TSpill::TSpill( const char * message )
{
  fYStep = 0.05;
  fIsMessage = kTRUE;
  fMessage = message;
}


TSpill::~TSpill()
{
  //
  ls();
  fDumps.Delete();
  fFlaggedDumps.Delete();
}

void TSpill::PrintSpill( TCanvas *c, double x, double y, Int_t colour )
{
  if(!fIsMessage) {

    TString log;

    log="";
    FormatADInfo(&log);                  
    AddText(c,x,y,0.02,colour,log.Data(),fNum);
      
    for(Int_t i=0; i<GetNumDump(); i++ ) {  
      log="";
      FormatDumpInfo(&log,GetDump(i));                  
      AddText(c,x,y-(1+i)*fYStep,0.02,colour,"%s",GetDump(i)->GetDescription().Data());
    }

    printf("%s",log.Data());
  
  }
  else  {
    AddText(c,x,y,0.02,kBlue,"%s",fMessage.Data());
  }




}



void TSpill::AddText(TCanvas * c, double x, double y, double textsize, int colour, const char* format, ...)
{
  if(!c) return;

  va_list argptr;
  char text[256];
  
  va_start(argptr, format);
  vsprintf(text, (char *) format, argptr);
  va_end(argptr);
  
  c->cd();

  TText* t = NULL;
  //if (format[0] != '%')
  //  t = (TText*)c->GetListOfPrimitives()->FindObject(format);
  //printf("Looking for object %s, found %p, text %s\n", format, t, text);
  if (!t) 
    {
      t = new TText(x,y,text);
      t->SetName(format);
      c->GetListOfPrimitives()->Add(t);
    }
  t->SetText(x, y, text);
  t->SetTextSize(textsize);
  t->SetTextColor(colour);
  t->SetTextFont(102);
  //  c->Modified();
  //c->Update();
}
