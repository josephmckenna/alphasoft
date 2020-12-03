#include "Histo.hh"
#include <iostream>

Histo::Histo( std::string fname )
{
  fROOT = new TFile(fname.c_str(),"RECREATE");
}

Histo::~Histo()
{
   Save();
   fROOT->Close();
   delete fROOT;
  
  fH1.clear();
  fH2.clear();
}

void Histo::Book(std::string hname, std::string htitle, 
		 int xbin, double xmin, double xmax,
		 int ybin, double ymin, double ymax)
{
  fROOT->cd();
  if( ybin > 0 )
    fH2[hname] = new TH2D(hname.c_str(),htitle.c_str(),
			  xbin, xmin, xmax,
			  ybin, ymin, ymax);
  else
    fH1[hname] = new TH1D(hname.c_str(),htitle.c_str(),
			  xbin, xmin, xmax);

  fHnames.insert(hname);
}

TH1* Histo::GetHisto(std::string hname)
{
  if( fH2.count( hname) )
    return GetH2(hname);
  else if( fH1.count( hname) )
    return GetH1(hname);
  else
    {
      std::cerr<<"Histo::GetHisto No Histogram named: "<<hname<<std::endl;
      return 0;
    }
}

int Histo::FillHisto(std::string hname, double x, double y, double w)
{
  if( fH2.count( hname) )
    {
      if( w > 0. )
	return GetH2(hname)->Fill(x,y,w);
      else
	return GetH2(hname)->Fill(x,y);
    }
  else if( fH1.count( hname) )
    {
      if( y > 0. )
	return GetH1(hname)->Fill(x,y);
      else
	return GetH1(hname)->Fill(x);
    }
  else
    {
      std::cerr<<"Histo::FillHisto No Histogram named: "<<hname<<std::endl;
      return 0;
    }
}

int Histo::WriteObject(TObject* obj, std::string oname)
{
   fROOT->cd();
   std::cout<<"Histo::WriteObject "<<oname;
   int bytes_written = gDirectory->WriteTObject(obj,oname.c_str());
   if( bytes_written > 0 )
      std::cout<<" DONE ("<<bytes_written<<")"<<std::endl;
   else
      std::cout<<" FAILED"<<std::endl;
   return bytes_written;
}

void Histo::Save()
{
   fROOT->cd();
  for( auto h : fH1 )
     h.second->Write();  
  for( auto h : fH2 )
     h.second->Write();  
  fROOT->Write();
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
