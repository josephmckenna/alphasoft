#ifndef __HISTO__
#define __HISTO__ 1

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

#include <string>
#include <map>
#include <set>

class Histo
{
private:
  TFile* fROOT;
  std::map<std::string,TH1D*> fH1;
  std::map<std::string,TH2D*> fH2;
  std::set<std::string> fHnames;

public:
  Histo( std::string fname="outsim.root" );
  ~Histo();

  void Book(std::string hname, std::string htitle, 
	    int xbin, double xmin, double xmax,
	    int ybin=0.0, double ymin=0.0, double ymax=0.0);

  TH1* GetHisto(std::string hname);
  
  inline TH1D* GetH1(std::string hname) { return fH1.at(hname); }
  inline TH2D* GetH2(std::string hname) { return fH2.at(hname); }

  int FillHisto(std::string hname, double x, double y=0., double w=0.);
};

#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
