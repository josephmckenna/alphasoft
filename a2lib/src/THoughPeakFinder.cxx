#include <math.h>

#include "THoughPeakFinder.h"

ClassImp( THoughPeakFinder )

THoughPeakFinder::THoughPeakFinder()
{
  //
}

THoughPeakFinder::~THoughPeakFinder()
{
  //
}

THoughPeakFinder::THoughPeakFinder( TH2D * hist, Double_t thr )
{
  //
  Double_t max = hist->GetMaximum();

  printf("Histogram: %s, xbins: %d, ybins: %d, max: %lf\n",
	 hist->GetName(),hist->GetNbinsX(),hist->GetNbinsY(),max);
  for( Int_t y = 0; y < hist->GetNbinsY(); y++ )
    for( Int_t x = 0; x < hist->GetNbinsX(); x++ )
      if( hist->GetBinContent( x, y ) >= thr )
	if( Is_Peak(hist, x,y ) >= 7 )
	  {
	    THoughPeak * p = new THoughPeak( x,y, hist->GetBinContent(x,y));
	    AddLast(p);
	  }
}


Int_t THoughPeakFinder::Is_Peak( TH2D * hist, Int_t x, Int_t y )
{
  Int_t peak_like = 0;

  Double_t center = hist->GetBinContent(x,y);

  if( center > hist->GetBinContent( x-1,y-1 ) )
    peak_like++;
  if( center > hist->GetBinContent( x-1,y   ) )
    peak_like++;
  if( center > hist->GetBinContent( x-1,y+1 ) )
    peak_like++;
  if( center > hist->GetBinContent( x  ,y-1 ) )
    peak_like++;
  if( center > hist->GetBinContent( x  ,y+1 ) )
    peak_like++;
  if( center > hist->GetBinContent( x+1,y-1 ) )
    peak_like++;
  if( center > hist->GetBinContent( x+1,y   ) )
    peak_like++;
  if( center > hist->GetBinContent( x+1,y+1 ) )
    peak_like++;

  return peak_like;
}

void THoughPeakFinder::Reduce()
{
  Int_t n = GetEntries();
  if( n < 2 )
    return;

  THoughPeak *tmp_list[GetEntries()];
  for( Int_t i = 0; i < n; i++ )
    tmp_list[i] = At(i);

  //reduce peaks
  while(n>0)
    {
      bool again = false;
      for(int i = 0; i < n; i++)
	for(int j = i+1; j < n; j++)
	  {
	    THoughPeak * pi = tmp_list[i];
	    THoughPeak * pj = tmp_list[j]; 

	    if( fabs( (float)pi->GetX() - (float)pj->GetX() ) > fxlimit ||
		fabs( (float)pi->GetY() - (float)pj->GetY() ) > fylimit )
	      continue;
	    
	    
	    if( pi->GetVotes() > pj->GetVotes() )
		  tmp_list[i] = pi;
	    else if( pi->GetVotes() < pj->GetVotes() )
		  tmp_list[i] = pj;
	    else
		  {
		    THoughPeak * p = new THoughPeak( (Int_t)0.5*(pi->GetX() + pj->GetX() ),
                                             (Int_t)0.5*(pi->GetY() + pj->GetY() ),
						     pi->GetVotes());
		    tmp_list[i] = p;
		  }
	    	    
	    for (int k=j+1; k<n; k++)
	      tmp_list[k-1] = tmp_list[k];
	    n--;
	    again = true;
	    
	    j = n+1;
	    i = n+1;
	  }
	   
      if(!again)
	break;
    } 

  TObjArray new_peaks;
  for( Int_t i = 0; i < n; i++ )
    {
	new_peaks.AddLast( tmp_list[i] );
    }
  fpeaks = new_peaks;
}
