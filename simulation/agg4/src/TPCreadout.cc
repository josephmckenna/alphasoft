// TPC Readout class implementation
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#include "TPCreadout.hh"
#include "ElectronDrift.hh"
#include <iostream>
#include <fstream>
#include <cassert>
#include "TPCBase.hh"

#include "TF1.h"

#ifndef MAX_ALPHA16
#define MAX_ALPHA16 32
#endif
#ifndef NUM_CHAN_ALPHA16
#define NUM_CHAN_ALPHA16 16
#endif

TPCreadout::TPCreadout():fEventNumber(0),fSignalThreshold(0.)
{
  if(TPCBase::TPCBaseInstance()->GetPrototype())
    std::cout<<"TPCreadout::TPCreadout * simulation * PROTO-rTPC"<<std::endl;
  else
    std::cout<<"TPCreadout::TPCreadout * simulation * ALPHA-g rTPC"<<std::endl;
  
  std::cout<<"* Number of Anode channels: "<<TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()<<std::endl;
  fAnodesArray = new TClonesArray("TAnode",
				  int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires())
				  );
  TClonesArray& wiresarray = *(fAnodesArray);
  for(int aw = 0; aw<int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()); ++aw)
    new( wiresarray[aw] ) TAnode(aw);

  std::cout<<"* Number of Pad channels: "<<TPCBase::TPCBaseInstance()->GetNumberOfPads()<<std::endl;
  fPadsArray = new TClonesArray("TPads",TPCBase::TPCBaseInstance()->GetNumberOfPads());
  TClonesArray& padsarray = *(fPadsArray);
  for(int ip = 0; ip<TPCBase::TPCBaseInstance()->GetNumberOfPads(); ++ip)
    new( padsarray[ip] ) TPads(ip);

  fPadsChargeSigma = 2. * (TPCBase::TPCBaseInstance()->GetROradius()-TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()) / 2.34;
}

TPCreadout::TPCreadout(int event):fEventNumber(event),fSignalThreshold(0.)
{
  fAnodesArray = new TClonesArray("TAnode",
				  int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires())
				  );
  TClonesArray& wiresarray = *(fAnodesArray);
  for(int aw = 0; aw<int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()); ++aw)
    new( wiresarray[aw] ) TAnode(aw);

  fPadsArray = new TClonesArray("TPads",TPCBase::TPCBaseInstance()->GetNumberOfPads());
  TClonesArray& padsarray = *(fPadsArray);
  for(int ip = 0; ip<TPCBase::TPCBaseInstance()->GetNumberOfPads(); ++ip)
    new( padsarray[ip] ) TPads(ip);

  fPadsChargeSigma = 2. * (TPCBase::TPCBaseInstance()->GetROradius()-TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()) / 2.34;
}

TPCreadout::TPCreadout(double sigthres):fEventNumber(0),
					  fAnodesArray(0),fPadsArray(0),
					  fSignalThreshold(sigthres)
{
  if(TPCBase::TPCBaseInstance()->GetPrototype())
    std::cout<<"TPCreadout::TPCreadout * analysis * PROTO-rTPC\t";
  else
    std::cout<<"TPCreadout::TPCreadout * analysis * ALPHA-g rTPC\t";
  std::cout<<"Number of Pad channels: "<<TPCBase::TPCBaseInstance()->GetNumberOfPads()<<std::endl;

  fPadsChargeSigma = 2. * (TPCBase::TPCBaseInstance()->GetROradius()-TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()) / 2.34;
}

TPCreadout::TPCreadout(const char* inclusion_list):fEventNumber(0),
						     fAnodesArray(0),fPadsArray(0),
						     fSignalThreshold(0.)
{
  if(TPCBase::TPCBaseInstance()->GetPrototype())
    std::cout<<"TPCreadout::TPCreadout * analysis * PROTO-rTPC\t";
  else
    std::cout<<"TPCreadout::TPCreadout * analysis * ALPHA-g rTPC\t";
  std::cout<<"Number of Pad channels: "<<TPCBase::TPCBaseInstance()->GetNumberOfPads()<<std::endl;
 
  std::ifstream fin(inclusion_list);
  if( fin.is_open() )
    {
      int channel;
      while(1)
	{
	  fin>>channel;
	  if( !fin.good() ) break;
	  fAWch.push_back( channel );
	}
      fin.close();
      std::cerr<<"TPCreadout::TPCreadout(const char*) including "<<fAWch.size()<<" channels"<<std::endl;
    }
  else
    {
      std::cerr<<"TPCreadout::TPCreadout(const char*) No Exclusion"<<std::endl;
    }

  fPadsChargeSigma = 2. * (TPCBase::TPCBaseInstance()->GetROradius()-TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()) / 2.34;
}


TPCreadout::TPCreadout( const TPCreadout& right ):fEventNumber(right.fEventNumber)
{
  fAnodesArray = new TClonesArray(*(right.fAnodesArray));
  fPadsArray   = new TClonesArray(*(right.fPadsArray));
}

TPCreadout& TPCreadout::operator=( const TPCreadout& right )
{
  fEventNumber = right.fEventNumber;
  fAnodesArray = new TClonesArray(*(right.fAnodesArray));
  fPadsArray   = new TClonesArray(*(right.fPadsArray));
  fPadsChargeSigma = right.fPadsChargeSigma;
  return *this;
}

TPCreadout::~TPCreadout()
{
  if(fAnodesArray) delete fAnodesArray;
  if(fPadsArray) delete fPadsArray;
}


void TPCreadout::AddHit(double t, double z, double phi)  // correct pad signal assignment
{
  //  std::cout<<"TPCreadout::AddHit("<<t<<","<<z<<","<<phi<<")"<<std::endl;
  int Wire = FindAnode( phi );
  ( (TAnode*) fAnodesArray->At(Wire) )->Increment(t);
  ( (TAnode*) fAnodesArray->At(Wire) )->SetZed(z);
  // std::cout<<"Wire: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetWire()
  //  	   <<"\tcharge: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetCharge()
  //  	   <<"\ttime: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetLeadingEdgeDriftTime()
  //  	   <<" ns"<<std::endl;
  // std::cout<<"z pos: "<<z<<" mm\t";
  double r,awphi;
  TPCBase::TPCBaseInstance()->GetAnodePosition(Wire, r, awphi, true);
  //  std::cout<<"TPCreadout::AddHit  wire: "<<Wire<<" @ "<<awphi<<"    phi: "<<phi<<std::endl;
  AddInduction(t,Wire);

  int Pad = FindPad( z, phi );
  if(Pad >= 0)
    {
      double zcm = z*0.1;
      TF1* f = new TF1("fPadsChargeProfile","TMath::Gaus(x,[0],[1],0)",
		       zcm-6.*fPadsChargeSigma,zcm+6.*fPadsChargeSigma);
      f->SetParameters(zcm,fPadsChargeSigma);
      ( (TPads*) fPadsArray->At(Pad) )->Increment(t, f->Eval( ( (TPads*) fPadsArray->At(Pad) )->GetZ()*0.1 ) );
      // std::cout<<"Pad: "<<( (TPads*) fPadsArray->At(Pad) )->GetPad()
      // 	       <<"\tcharge: "<<( (TPads*) fPadsArray->At(Pad) )->GetCharge()
      // 	       <<"\ttime: "<<( (TPads*) fPadsArray->At(Pad) )->GetLeadingEdgeDriftTime()
      // 	       <<" ns"<<std::endl;
      double z,pdphi;
      TPCBase::TPCBaseInstance()->GetPadPosition(Pad,z,pdphi);
      //      std::cout<<"TPCreadout::AddHit  pad: "<<Pad<<" @ "<<pdphi<<"    phi: "<<phi<<std::endl;
      AddInduction(t,Pad,f); // <-- this one used
      delete f;
    }
}

void TPCreadout::AddHit(int id, int pdg, double t, double z, double phi)
{
  int Wire = FindAnode( phi );
  ( (TAnode*) fAnodesArray->At(Wire) )->Increment(id,pdg,t);
  ( (TAnode*) fAnodesArray->At(Wire) )->SetZed(z);
  // std::cout<<"Wire: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetWire()
  // 	   <<"\tcharge: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetCharge()
  // 	   <<"\ttime: "<<( (TAnode*) fAnodesArray->At(Wire) )->GetLeadingEdgeDriftTime()
  // 	   <<" ns"<<std::endl;
  //  std::cout<<"z pos: "<<z<<" mm\t";

  int Pad = FindPad( z, phi );
  if(Pad >= 0){
      ( (TPads*) fPadsArray->At(Pad) )->Increment(id,pdg,t);
      // std::cout<<"Pad: "<<( (TPads*) fPadsArray->At(Pad) )->GetPad()
      // 	     <<"\tcharge: "<<( (TPads*) fPadsArray->At(Pad) )->GetCharge()
      // 	     <<"\ttime: "<<( (TPads*) fPadsArray->At(Pad) )->GetLeadingEdgeDriftTime()
      // 	     <<" ns"<<std::endl;

      AddInduction(t,Pad,Wire);   // FIXME: Is this ok to skip if pad is out of bounds?
  }
}

int TPCreadout::FindPad(const double zed, const double phi)
{
  //  std::cout<<"TPCreadout::FindPad z: "<<zed<<"\tphi: "<<phi<<"\t";
  auto p = TPCBase::TPCBaseInstance()->FindPad(zed*0.1, phi);
  int Pad = TPCBase::TPCBaseInstance()->SectorAndPad2Index(p.first, p.second);
  //  std::cout<<"Pad: "<<Pad<<std::endl;
  return Pad;
}

int TPCreadout::FindAnode(const double phi)
{
  int Wire = (int) TPCBase::TPCBaseInstance()->FindAnode(phi);
  return Wire;
}

void TPCreadout::AddInduction(const double t, const int pad, const int anode)
{
  int up = pad + TPCBase::TPCBaseInstance()->GetNumberPadsRow();
  for(int ip=0; ip<6; ++ip)
    {
      if( up >= TPCBase::TPCBaseInstance()->GetNumberOfPads() ) break;
      ( (TPads*) fPadsArray->At(up) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetPadInduction(ip) );
      up+=TPCBase::TPCBaseInstance()->GetNumberPadsRow();
    }
  int dw = pad - TPCBase::TPCBaseInstance()->GetNumberPadsRow();
  for(int ip=0; ip<6; ++ip)
    {
      if( dw < 0 ) break;
      ( (TPads*) fPadsArray->At(dw) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetPadInduction(ip) );
      dw-=TPCBase::TPCBaseInstance()->GetNumberPadsRow();
    }

  //  std::cout<<"anode: "<<anode;
  int posve = anode + 1;
  //  std::cout<<" next: "<<posve;
  if( posve == int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()) ) posve = 0;
  //  std::cout<<" phifold: "<<posve<<"\n";
  for(int aw=0; aw<3; ++aw)
    {
      ( (TAnode*) fAnodesArray->At(posve) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetAnodeInduction(aw) );
      ++posve;
      if( posve == int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()) ) posve = 0;
      //      std::cout<<" next: "<<posve<<"\n";
    }

  int negve = anode - 1;
  //  std::cout<<" prev: "<<negve;
  if( negve == -1 ) negve = 255;
  //  std::cout<<" phifold: "<<negve<<"\n";
  for(int aw=0; aw<3; ++aw)
    {
      ( (TAnode*) fAnodesArray->At(negve) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetAnodeInduction(aw) );
      --negve;
      if( negve == -1 ) negve = 255;
      //      std::cout<<" prev: "<<negve<<"\n";
    }
}

void TPCreadout::AddInduction(const double t, const int anode)
{
  //  std::cout<<"anode: "<<anode;
  int posve = anode + 1;
  //  std::cout<<" next: "<<posve;
  if( posve == int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()) ) posve = 0;
  //  std::cout<<" phifold: "<<posve<<"\n";
  for(int aw=0; aw<3; ++aw)
    {
      ( (TAnode*) fAnodesArray->At(posve) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetAnodeInduction(aw) );
      ++posve;
      if( posve == int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()) ) posve = 0;
      //      std::cout<<" next: "<<posve<<"\n";
    }

  int negve = anode - 1;
  //  std::cout<<" prev: "<<negve;
  if( negve == -1 ) negve = 255;
  //  std::cout<<" phifold: "<<negve<<"\n";
  for(int aw=0; aw<3; ++aw)
    {
      ( (TAnode*) fAnodesArray->At(negve) )->SetSignal( t, ElectronDrift::ElectronDriftInstance()->GetAnodeInduction(aw) );
      --negve;
      if( negve == -1 ) negve = 255;
      //      std::cout<<" prev: "<<negve<<"\n";
    }
}

void TPCreadout::AddInduction(const double t, const int pad, const TF1* f) // <-- use this one
{
  int up = pad + TPCBase::TPCBaseInstance()->GetNumberPadsRow();
  for(int ip=0; ip<6; ++ip)
    {
      if( up >= TPCBase::TPCBaseInstance()->GetNumberOfPads() ) break;
      ( (TPads*) fPadsArray->At(up) )->SetSignal( t, f->Eval( ((TPads*) fPadsArray->At(up) )->GetZ()*0.1) );
      up+=TPCBase::TPCBaseInstance()->GetNumberPadsRow();
    }
  int dw = pad - TPCBase::TPCBaseInstance()->GetNumberPadsRow();
  for(int ip=0; ip<6; ++ip)
    {
      if( dw < 0 ) break;
      ( (TPads*) fPadsArray->At(dw) )->SetSignal( t, f->Eval( ((TPads*) fPadsArray->At(dw) )->GetZ()*0.1)  );
      dw-=TPCBase::TPCBaseInstance()->GetNumberPadsRow();
    }
}


void TPCreadout::Reset()
{
  for(int aw=0; aw<fAnodesArray->GetEntries(); ++aw)
    ( (TAnode*) fAnodesArray->At(aw) )->Reset(false);

  for(int ip=0; ip<fPadsArray->GetEntries(); ++ip)
    ( (TPads*) fPadsArray->At(ip) )->Reset(false);
}

int TPCreadout::GetNumberOfWiresHit()
{
  int N=0;
  for(int aw = 0; aw<int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()); ++aw)
    {
      TAnode* awire = GetWire(aw);
      double Q = awire->GetCharge();
      if( Q >= 1. ) 
	{
	  // std::cout<<"aw: "<<aw<<" (Q="<<Q<<") ";
	  // for(auto it = awire->GetSignal().begin() ; it != awire->GetSignal().end(); ++it)
	  //   std::cout<<(it - awire->GetSignal().begin())<<","<<*it<<" ";
	  // std::cout<<"\n";
	  ++N;
	}
    }
  return N;
}
int TPCreadout::GetNumberOfPadsHit()
{
  int N=0;
  for(int ip = 0; ip<TPCBase::TPCBaseInstance()->GetNumberOfPads(); ++ip)
    {
      TPads* apad = GetPad(ip);
      double Q = apad->GetCharge();
      if( Q >= 0. ) ++N;
    }
  return N;
}

Alpha16Event* TPCreadout::GetALPHA16Event(const TClonesArray* anodes)
{
  //  std::cout<<"TPCreadout::GetALPHA16Event "<<fEventNumber<<std::endl;
  // fEventBuilder.Reset();
  // Alpha16Event* anEvent = fEventBuilder.NewEvent();
  Alpha16Event* anEvent = new Alpha16Event();
  anEvent->counter = fEventNumber;
  int MaxChan(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires());

  double mV2ADC(8.);
  //  std::cout<<"TPCreadout::GetALPHA16Event   Number of Wires: "<<anodes->GetEntries()<<std::endl;

  for(int i=0; i<anodes->GetEntries(); ++i)
    {
      TAnode* elec = (TAnode*) anodes->At(i);
      if(elec)
	{
	  bool pres=true;
	  if( fAWch.size() > 0 )
	    pres=false;
	  for(auto aaa: fAWch)
	    {
	      if( aaa == elec->GetWire() )
		pres=true;
	    }
	  if( !pres ) continue;
	  TString ename( elec->GetName() );

	  // if( ename == "TAnode" )
	  //   std::cout<<"Anode # "<<elec->GetWire()<<"\tphi: "<<elec->GetPosition()<<"\t";
	  // else
	  //   std::cout<<"Unknown electrode";
	  // std::cout<<"Q: "<<elec->GetCharge()<<"\t";
	  // if( elec->GetDriftTimes().size() > 0 )
	  //   std::cout<<"t: "<<elec->GetLeadingEdgeDriftTime()<<" ns\n";
	  // else
	  //   std::cout<<"\n";

	  std::vector<double> signal( elec->GetSignal() );
	  std::vector<double>::iterator minit = std::min_element(signal.begin(),signal.end());
	  double amplitude,min,max;

	  if( minit != signal.end() )
	    min = TMath::Abs(*minit);
	  else
	    min = 0.;
	  std::vector<double>::iterator maxit = std::max_element(signal.begin(),signal.end());
	  if( maxit != signal.end() )
	    max = *maxit;
	  else
	    max = 0.;
	  amplitude = max>min?max:min;

	  if(amplitude >= fSignalThreshold)
	    {
	      int chan = elec->GetWire();

	      if( chan < MaxChan && chan >= 0. )
		{
		  // std::cout<<"ch: "<<chan<<"\tnsamples: "
		  // 	   <<anEvent->udpPacket[chan].nsamples<<"\n";

		  Alpha16Channel* aChannel = new Alpha16Channel;
		  aChannel->adc_module=0;
		  aChannel->adc_chan=48;
		  aChannel->tpc_wire=chan;
		  
		  // fill in the signal
		  for(unsigned int n = 0; n < signal.size(); ++n)
		    //anEvent->waveform[chan].push_back( int(signal.at(n)*mV2ADC) );
		    //  anEvent->hits[chan]->adc_samples.push_back( int(signal.at(n)*mV2ADC) );
		    aChannel->adc_samples.push_back( int(signal.at(n)*mV2ADC) );
		      
		  anEvent->hits.push_back(aChannel);
		}
	      else
		{
		  std::cerr<<"Electrode "<<chan
			   <<" is outside ALPHA16 module range"<<std::endl;
		}
	    }// electrodes signal > threshold
	}// electrode exists
    }// electrodes loop

  // std::cout<<"TPCreadout::GetALPHA16Event: "<<n<<" anode signals in range"<<std::endl;
  // anEvent->Print();

  return anEvent;
}

GrifCEvent_t* TPCreadout::GetGrifCEvent(const TClonesArray* pads)
{
  GrifCEvent_t* anEvent = new GrifCEvent_t;
  anEvent->eventNo = fEventNumber+1;

  int MaxChan = NUM_PADS_CHAN;
  double mV2ADC(8.);

  for(int i=0; i<pads->GetEntries(); ++i)
    {
      TPads* elec = (TPads*) pads->At(i);
      if(elec)
	{
	  TString ename( elec->GetName() );
	  // if( ename == "TPads" )
	  //   std::cout<<"Pad # "<<elec->GetPad()<<"\tz: "<<elec->GetZ()<<"\t";
	  // else
	  //   std::cout<<"Unknown electrode";
	  // std::cout<<"Q: "<<elec->GetCharge()<<"\t";
	  // if( elec->GetDriftTimes().size() > 0 )
	  //   std::cout<<"t: "<<elec->GetLeadingEdgeDriftTime()<<" ns\n";
	  // else
	  //   std::cout<<"\n";
	  int chan = elec->GetPad();
	  if( chan < MaxChan && chan >= 0. )
	    {
	      std::vector<double> signal = elec->GetSignal();
	      anEvent->waveform[chan].reserve( elec->GetNumberOfSamples() );
	      anEvent->waveform[chan].clear();

	      //std::cout<<"ch: "<<chan<<"\n";

	      // fill in the signal
	      for(unsigned int n = 0; n < signal.size(); ++n)
		anEvent->waveform[chan].push_back( int16_t(signal.at(n)*mV2ADC) );

	      // double max = double(*std::max_element( anEvent->waveform[chan].begin(),
	      // 					     anEvent->waveform[chan].end() ) );
	      //	      std::cout<<"\t\tchan: "<<chan<<"\tmax: "<<max<<std::endl;

	      anEvent->udpPresent[chan] = true;

	      anEvent->numChan++;
	      //std::cout<<"\tnumChan: "<<anEvent->numChan<<"\tpresent: "
	      //<<anEvent->udpPresent[chan]<<"\n";
	    }
	  else
	    {
	      std::cerr<<"Electrode "<<chan
		       <<" is outside GrifC module range"<<std::endl;
	    }

	}// electrode exists
    }// electrodes loop

  anEvent->complete = true;

  //  int n = anEvent->numChan;
  // std::cout<<"TPCreadout::GetGrifCEvent: "<<n<<" anode signals in range"<<std::endl;
  // anEvent->Print();

  return anEvent;
}

// int TPCreadout::SectorAndPad2Index(const int ps, const int pi){
//     return (ps+pi*TPCBase::TPCBaseInstance()->GetNumberPadsRow());
// }

// std::pair<int,int> TPCreadout::Index2SectorAndPad(const int padindex){
//     int pi = padindex/TPCBase::TPCBaseInstance()->GetNumberPadsRow();
//     int ps = padindex%TPCBase::TPCBaseInstance()->GetNumberPadsRow();
//     return std::pair<int,int>(ps,pi);
// }
