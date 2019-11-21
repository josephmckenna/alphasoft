#include "TAlphaEventMap.h"

ClassImp( TAlphaEventMap )

TAlphaEventMap::TAlphaEventMap()
{
   SetValues();
}

//______________________________________________________________________________
void TAlphaEventMap::SetValues( )
{
   assert( gGeoManager );
   for (int j=0; j<72;j++)
   {
      char* SilName=ReturnSilName(j);
      fCos[j] = 0.;
      fSin[j] = 0.;
      fXCenter[j] = 0.;
      fYCenter[j] = 0.;
      fZCenter[j] = 0.;

      // Grab the module parameters from the GeoManager
      Int_t n = (Int_t) gGeoManager->GetTopVolume()->GetNodes()->GetEntries();
      TGeoNode* node = NULL;
      for( Int_t i = 0; i < n; i++ )
      {
         node = gGeoManager->GetTopVolume()->GetNode( i );
         if( strncmp(node->GetName(),SilName,4) == 0 ) break;
         else node = NULL;
      }
      if( node )
      {
         fXCenter[j] = node->GetMatrix()->GetTranslation()[0];
         fYCenter[j] = node->GetMatrix()->GetTranslation()[1];
         fZCenter[j] = node->GetMatrix()->GetTranslation()[2];

         Double_t radius = TMath::Sqrt( fXCenter[j]*fXCenter[j] + fYCenter[j]*fYCenter[j] );
         fCos[j] = fXCenter[j]/radius;
         fSin[j] = fYCenter[j]/radius;
         fLayer[j]=ReturnLayer(j);
      }
      else
      {
         fCos[j] = 0;
         fSin[j] = 0;
         fXCenter[j] = 0;
         fYCenter[j] = 0;
         fZCenter[j] = 0;
      }
      delete SilName;
   }
}


//______________________________________________________________________________
Int_t TAlphaEventMap::ReturnLayer(Int_t SilNum)
{
  if(nSil == 72){
  // Take the module number 0..71
  // return the detector layer
  
  if(SilNum < 10) return 0;
  else if(SilNum >= 10 && SilNum < 22) return 1;
  else if(SilNum >= 22 && SilNum < 36) return 2;
  else if(SilNum >= 36 && SilNum < 46) return 0;
  else if(SilNum >= 46 && SilNum < 58) return 1;
  else if(SilNum >= 58 && SilNum < 72) return 2;

  return -1;
  }
  if(nSil == 60) {

  // Take the module number 0..59
  // return the detector layer

  if(SilNum < 8) return 0;
  else if(SilNum >= 8  && SilNum < 18) return 1;
  else if(SilNum >= 18 && SilNum < 30) return 2;
  else if(SilNum >= 30 && SilNum < 38) return 0;
  else if(SilNum >= 38 && SilNum < 48) return 1;
  else if(SilNum >= 48 && SilNum < 60) return 2;

  return -1;
  }
}


//______________________________________________________________________________
char * TAlphaEventMap::ReturnSilName(Int_t SilNum)
{
  assert( SilNum >= 0 );
  assert( SilNum < nSil );
  
  if(nSil == 72){
  // Return the silname e.g. 4si5
  // from the silicon module number 0..71

  char * name = new char[7];
  
  // AD end
  if ( SilNum < 10 ) sprintf(name,"0si%1d",SilNum);
  if ( SilNum >= 10 && SilNum < 22 )
  {
      if(SilNum == 20) sprintf(name, "1siA" );
      else if ( SilNum == 21 ) sprintf(name, "1siB");
      else  sprintf(name,"1si%1d",SilNum - 10 );
  }
  if ( SilNum >= 22 && SilNum < 36 )
  {
      if( SilNum == 32 ) sprintf( name,"2siA" );
      else if( SilNum == 33 ) sprintf( name,"2siB" );
      else if( SilNum == 34 ) sprintf( name,"2siC" );
      else if( SilNum == 35 ) sprintf( name,"2siD" );
      else sprintf(name,"2si%d",SilNum - 10 - 12);
  }

  if ( SilNum >= 36 && SilNum < 46 ) sprintf(name,"3si%1d",SilNum - 10 - 12 - 14);
  if ( SilNum >= 46 && SilNum < 58 )
  {
      if (SilNum == 56) sprintf(name, "4siA");
      else if (SilNum == 57) sprintf(name, "4siB");
      else sprintf(name,"4si%1d",SilNum - 10 - 12 - 14 - 10);
  }
  if ( SilNum >= 58 && SilNum < 72 )
  {
      if( SilNum == 68 ) sprintf( name,"5siA" );
      else if( SilNum == 69 ) sprintf( name,"5siB" );
      else if( SilNum == 70 ) sprintf( name,"5siC" );
      else if( SilNum == 71 ) sprintf( name,"5siD" );
      else sprintf(name,"5si%1d",SilNum - 10 - 12 - 14 - 10 - 12);
  }
  return name;
}
  
  
  if(nSil ==60) {
    // Return the silname e.g. 4si5
    // from the silicon module number 0..59

    char * name = new char[7];
  // AD end
    if ( SilNum < 8 )
      {
        sprintf(name,"0si%1d",SilNum);
      }
    if ( SilNum >= 8 && SilNum < 18 )
      {
        sprintf(name,"1si%1d",SilNum - 8);
      }
    if ( SilNum >=18 && SilNum < 30 )
      {
        if( SilNum == 28 )
          sprintf( name,"2siA" );
        else if( SilNum == 29 )
          sprintf( name,"2siB" );

        else
          sprintf(name,"2si%d",SilNum - 18);
      }
      
    // e+ end
    if ( SilNum >= 30 && SilNum < 38 )
      {
        sprintf(name,"3si%1d",SilNum - 30);
      }
    if ( SilNum >= 38 && SilNum < 48 )
      {
        sprintf(name,"4si%1d",SilNum - 38);
      }
    if ( SilNum >= 48 && SilNum < 60 )
      {
        if( SilNum == 58)
          sprintf( name,"5siA" );
        else if( SilNum == 59 )
          sprintf( name,"5siB" );
        else
          sprintf(name,"5si%1d",SilNum - 48);
  
      }
  return name;
  }
}

void TAlphaEventMap::Print(Option_t* /*option*/) const
{
   for (int i=0; i<nSil; i++)
   {
      printf("cos: \t%f\n",fCos[i]);     //cos from vertical
      printf("sin: \t%f\n",fSin[i]);     //sin from vertical
      printf("X center:\t%f\n",fXCenter[i]); //x center of module
      printf("Y center:\t%f\n",fYCenter[i]); //y center of module
      printf("Z center:\t%f\n",fZCenter[i]); //z center of module
      printf("Layer: \t%d\n",fLayer[i]);
   }
}
