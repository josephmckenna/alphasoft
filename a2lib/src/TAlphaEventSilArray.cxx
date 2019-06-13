// TAlphaEventSilArray.h
#include "TAlphaEventSilArray.h"

TAlphaEventSilArray::TAlphaEventSilArray( Int_t ASIC, TString SilName, Double_t * Array )
{
  fASIC = ASIC;
  fSilName = SilName;
  for( Int_t i = 0; i < 128;  i++ )
    fArray[i] = Array[i];
}

TAlphaEventSilArray::TAlphaEventSilArray()
{
  memset(fArray,0,sizeof(fArray));
  fASIC = 0;
}

TAlphaEventSilArray::~TAlphaEventSilArray()
{
  //dtor
}

void TAlphaEventSilArray::Print(Option_t * /* opt */) const
{
  printf("\n---------------TAlphaEventSilArray-----------\n");
  printf("SilName: %s ASIC: %d\n",fSilName.Data(),fASIC);
  for( Int_t i = 0; i < 128; i++ )
    printf("%lf ",fArray[i]);
  printf("\n---------------------------------------------\n");
}
