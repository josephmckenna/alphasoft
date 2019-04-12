#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <TFile.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TClonesArray.h>

#include "TStoreEvent.hh"
#include "AnaSettings.h"

#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "argparse.hpp"

enum finderChoice { base, adaptive, neural };

class PatternRecognition
{
public:
  PatternRecognition(AnaSettings* set, double B):fMagneticField(B),
						 fPointsArray("TSpacePoint",1000),
                                                 fTracksArray("TTrack",50)
  {
    fNhitsCut = set->GetInt("RecoModule","NhitsCut");
    fNspacepointsCut = set->GetInt("RecoModule","NpointsCut");
    fPointsDistCut = set->GetDouble("RecoModule","PointsDistCut");
    fMaxIncreseAdapt = set->GetDouble("RecoModule","MaxIncreseAdapt");
    fSeedRadCut = set->GetDouble("RecoModule","SeedRadCut");
    fSmallRadCut = set->GetDouble("RecoModule","SmallRadCut");
    fLastPointRadCut = set->GetDouble("RecoModule","LastPointRadCut");
 
    fLambda = set->GetDouble("RecoModule","Lambda_NN");
    fAlpha = set->GetDouble("RecoModule","Alpha_NN");
    fB = set->GetDouble("RecoModule","B_NN");
    fTemp = set->GetDouble("RecoModule","T_NN");
    fC = set->GetDouble("RecoModule","C_NN");
    fMu = set->GetDouble("RecoModule","Mu_NN");
    fCosCut = set->GetDouble("RecoModule","CosCut_NN");
    fVThres = set->GetDouble("RecoModule","VThres_NN");

    fDNormXY = set->GetDouble("RecoModule","DNormXY_NN");
    fDNormZ = set->GetDouble("RecoModule","DNormZ_NN");

    fTscale = set->GetDouble("RecoModule","TScale_NN");
    fMaxIt = set->GetInt("RecoModule","MaxIt_NN");
    fItThres = set->GetDouble("RecoModule","ItThres_NN");
  }

  ~PatternRecognition()
  {
    Reset();
  }
  
  void SetPoints( const TObjArray* p )
  {
    for( int n=0; n<p->GetEntriesFast(); ++n )
      {
	new(fPointsArray[n]) TSpacePoint(*(TSpacePoint*)p->At(n));
      }
  }
  
  int Find(finderChoice finder)
  {
    TracksFinder *pattrec;
    switch(finder)
      {
      case adaptive:
	pattrec = new AdaptiveFinder( &fPointsArray );
	((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(fMaxIncreseAdapt);
	break;
      case neural:
	pattrec = new NeuralFinder( &fPointsArray );
	((NeuralFinder*)pattrec)->SetLambda(fLambda);
	((NeuralFinder*)pattrec)->SetAlpha(fAlpha);
	((NeuralFinder*)pattrec)->SetB(fB);
	((NeuralFinder*)pattrec)->SetTemp(fTemp);
	((NeuralFinder*)pattrec)->SetC(fC);
	((NeuralFinder*)pattrec)->SetMu(fMu);
	((NeuralFinder*)pattrec)->SetCosCut(fCosCut);
	((NeuralFinder*)pattrec)->SetVThres(fVThres);
	((NeuralFinder*)pattrec)->SetDNormXY(fDNormXY);
	((NeuralFinder*)pattrec)->SetDNormZ(fDNormZ);
	((NeuralFinder*)pattrec)->SetTscale(fTscale);
	((NeuralFinder*)pattrec)->SetMaxIt(fMaxIt);
	((NeuralFinder*)pattrec)->SetItThres(fItThres);
	break;
      case base:
	pattrec = new TracksFinder( &fPointsArray ); 
	break;
      }

    pattrec->SetPointsDistCut(fPointsDistCut);
    pattrec->SetNpointsCut(fNspacepointsCut);
    pattrec->SetSeedRadCut(fSeedRadCut);

    int stat = pattrec->RecTracks();
    int tk,npc,rc;
    pattrec->GetReasons(tk,npc,rc);
    track_not_advancing += tk;
    points_cut += npc;
    rad_cut += rc;

    AddTracks( pattrec->GetTrackVector() );
    delete pattrec;
    return stat;
  }

  void AddTracks( const std::vector<track_t>* track_vector )
  {
    int n=0;
    for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
      {
	TTrack* thetrack=( (TTrack*)fTracksArray.ConstructedAt(n) ) ;
	thetrack->Clear();
	thetrack->SetMagneticField(fMagneticField);
	for( auto ip=it->begin(); ip!=it->end(); ++ip)
	  {
	    TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(*ip);
	    thetrack->AddPoint( ap );
	  }
	++n;
      }
    fTracksArray.Compress();
    assert(n==int(track_vector->size()));
    assert(fTracksArray.GetEntriesFast()==int(track_vector->size()));
  }

  void Reset()
  {
    fTracksArray.Clear("C"); // Ok, I need a delete here to cure leaks... further work needed
    fPointsArray.Clear();
  }

  const TClonesArray* GetTracksArray() const { return &fTracksArray; }

private:
  double fMagneticField;

  TClonesArray fPointsArray;
  TClonesArray fTracksArray;

  // general TracksFinder parameters, also used by other finders
  unsigned fNhitsCut;
  unsigned fNspacepointsCut;
  double fPointsDistCut;
  double fSeedRadCut;
  double fSmallRadCut;

  // AdaptiveFinder
  double fMaxIncreseAdapt;
  double fLastPointRadCut;

  // NeuralFinder
  // V_kl = 0.5 * [1 + tanh(c/Temp \sum(T_kln*V_ln) - alpha/Temp{\sum(V_kn) + \sum(V_ml)} + B/Temp)]
  // NN parameters             // ALEPH values (see DOI 10.1016/0010-4655(91)90048-P)
  double fLambda;              // 5.
  double fAlpha;               // 5.
  double fB;                   // 0.2
  double fTemp;                // 1.
  double fC;                   // 10.
  double fMu;                  // 2.
  double fCosCut;              // 0.9     larger kinks between neurons set T value to zero
  double fVThres;              // 0.9     V value above which a neuron is considered active

  double fDNormXY;             // normalization for XY distance and
  double fDNormZ;              // Z distance, different to weight the influence of gaps differently
  // no good reason for these values
  double fTscale;              // fudge factor to bring T values into range [0,1],
  // probably has to be changed with other parameters...
  int fMaxIt;                  // number of iterations
  double fItThres;             // threshold defining convergence

  // Reasons for pattrec to fail:
  int track_not_advancing;
  int points_cut;
  int rad_cut;
};

int main(int argc, const char** argv)
{
  // make a new ArgumentParser
  ArgumentParser parser;
  // add some arguments
  parser.addArgument("-f","--rootfile",1,false);
  parser.addArgument("-a","--anasettings",1,true);
  parser.addArgument("-b","--Bfield",1,true);
  parser.addArgument("-e","--Nevents",1,true);
  parser.addArgument("--finder",1,true);
  // parse the command-line arguments - throws if invalid format
  parser.parse(argc, argv);

  string fname = parser.retrieve<string>("rootfile");
  TFile* fin = TFile::Open(fname.c_str(),"READ");
  if( !fin->IsOpen() ) return 2;

  TTree* tEvents = (TTree*) fin->Get("StoreEventTree");
  if( !tEvents ) return 3;
  int Nevents = tEvents->GetEntriesFast();
  cout<<tEvents->GetTitle()<<"\t"<<Nevents<<endl;
  
  TStoreEvent* anEvent = new TStoreEvent();
  tEvents->SetBranchAddress("StoredEvent", &anEvent);

  string settings="default";
  if( parser.count("anasettings") )
    settings = parser.retrieve<string>("anasettings");

  AnaSettings* ana_settings = new AnaSettings(settings.c_str());
  ana_settings->Print();

  double MagneticField=1.0;
  if( parser.count("Bfield") )
    {
      string Bfield = parser.retrieve<string>("Bfield");
      MagneticField = stod(Bfield);
    }
  cout<<"Magnetic Field: "<<MagneticField<<" T"<<endl;

  finderChoice choosen_finder = adaptive;
  if( parser.count("finder") )
    {
      string cf = parser.retrieve<string>("finder");
      if( cf == "base") choosen_finder = base;
      else if( cf == "neural") choosen_finder = neural;
      else if( cf == "adaptive") choosen_finder = adaptive;
      else cerr<<"Unknown track finder mode \""<<cf<<"\", using adaptive"<<endl;
    }
  cout<<"Using track finder: "<<choosen_finder<<endl;
  PatternRecognition ppr(ana_settings,MagneticField);

  if( parser.count("Nevents") )
    {
      string nev = parser.retrieve<string>("Nevents");
      Nevents = stoi(nev);
    }
  cout<<"Processing "<<Nevents<<" events"<<endl;
  
  for( int n=0; n<Nevents; ++n)
    {
      tEvents->GetEntry(n);
      const TObjArray* points = anEvent->GetSpacePoints();    
      cout<<n<<"\tEvent Number: "
	  <<anEvent->GetEventNumber()<<"\tTime of the Event: "<<anEvent->GetTimeOfEvent()<<"s"<<endl;
      
      ppr.SetPoints( points );
      int nt = ppr.Find( choosen_finder );
      cout<<"\t# of Points: "<<points->GetEntriesFast()<<"\t# of Tracks: "<<nt<<endl;

      TFitVertex Vertex(anEvent->GetEventNumber());
      Vertex.SetChi2Cut( ana_settings->GetDouble("RecoModule","VtxChi2Cut") );
      int ntracks=ppr.GetTracksArray()->GetEntriesFast();
      for(int it=0; it<ntracks; ++it )
	{
	  TTrack* at = (TTrack*) ppr.GetTracksArray()->At(it);
	  if( MagneticField > 0. )
	    {
	      TFitHelix* helix = new TFitHelix(*at);
	      helix->SetChi2RCut( ana_settings->GetDouble("RecoModule","HelChi2RCut") );
	      helix->SetChi2ZCut( ana_settings->GetDouble("RecoModule","HelChi2ZCut") );
	      helix->SetChi2RMin( ana_settings->GetDouble("RecoModule","HelChi2RMin") );
	      helix->SetChi2ZMin( ana_settings->GetDouble("RecoModule","HelChi2ZMin") );
	      helix->SetDCut( ana_settings->GetDouble("RecoModule","HelDcut") );
	      helix->Fit();

            if( helix-> GetStatR() > 0 &&
                helix-> GetStatZ() > 0 )
               helix->CalculateResiduals();

            if( helix->IsGood() )
               {
		 // calculate momumentum
		 helix->Momentum();
		 Vertex.AddHelix(helix);
		 helix->Print();
	       }
	    else
	      {
		delete helix;
	      }
	    }
	  else
	    {
	      TFitLine* l = new TFitLine(*at);
	      l->SetChi2Cut( ana_settings->GetDouble("RecoModule","LineChi2Cut") );
	      l->SetChi2Min( ana_settings->GetDouble("RecoModule","LineChi2Min") );
	      l->SetPointsCut( ana_settings->GetInt("RecoModule","NpointsCut") );
	      l->Fit();
	      if( l->GetStat() > 0 )
		l->CalculateResiduals();
	      if( l->IsGood() )
		l->Print();
	      delete l;
	    }
	}
      
      int sv = Vertex.Calculate();
      if( sv > 0 ) Vertex.Print();
      Vertex.Reset();
      ppr.Reset();
      anEvent->Reset();
    }
  
  cout<<"End of run"<<endl;
    
  return 0;
}
