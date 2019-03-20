#include <iostream>
#include <fstream>

#include <TMath.h>
#include <TString.h>

#include <GeometrySimple.hh>
#include <SolidTube.hh>
#include <Medium.hh>

#include "ComponentBmap.hh"
#include "TPC.hh"

TPC::TPC(double V_c, double V_a, double V_f): Garfield::ComponentBmap(true), 
					      TPCBase(false),
					      CathodeVoltage(V_c), 
					      AnodeVoltage(V_a), 
					      FieldVoltage(V_f), 
					      medium(0), chamber(0)
{}

void TPC::init()
{
  if(fPrototype)
    std::cout<<"TPC::init() Proto-rTPC\t";
  else
    std::cout<<"TPC::init() ALPHA-g rTPC\t";
  std::cout<<"TPC::init() length: "<<FullLengthZ<<" cm"<<std::endl;

  DisableDebugging();

  // outer wall of the TPC
  AddTube(ROradius,0.0,0,"ro");

  // // define cathodic pads
  // double PadPosZmax = ((double) npads) * 0.5 * PadSideZ;
  // double PadPosZmin = PadPosZmax - PadSideZ;
  // //  const double gapZ = ROradius - AnodeWiresR;
  // std::cout<<"TPC::init() === PADS POSITIONING ==="<<std::endl;
  // for(int ip=0; ip<npads; ++ip)
  //   {
  //     TString padname = TString::Format("p%d",ip);

  //     //std::cout<<ip<<"\t"<<PadPosZmin<<"\t"<<PadPosZmax<<"\t"<<padname<<std::endl;
  //     // AddStripOnTube(ROradius, PadPosZmin,PadPosZmax,
  //     // 		     padname.Data(), gapZ);

  //     pads.push_back(padname.Data());
  //     // Activate "Weighting Fields"
  //     //AddReadout(padname.Data());

  //     PadPosZmax  = PadPosZmin;
  //     PadPosZmin -= PadSideZ;
  //   }

  // inner wall of the TPC
  const double IV = CathodeVoltage;
  //  AddWire(0.,0.,2.*CathodeRadius,IV,"c",2.*HalfWidthZ,<tension>,<density>1.21,<trap radius>);
  AddWire(0.,0.,2.*CathodeRadius,IV,"c",FullLengthZ);

  // Field Wires

  const double VoltFieldWires = FieldVoltage;

  int nf=0;
  for(int i=0; i<NfieldWires; ++i)
    {
      double xw, yw;
      GetWirePosition(i,xw,yw);
      AddWire(xw,yw,diamFieldWires,VoltFieldWires,
	      "f",FullLengthZ,tensionFieldWires);
      ++nf;
    }
  std::cout<<"TPC::init() ---> Number of Field Wires: "<<nf<<std::endl;

  // Anode Wires

  const double VoltAnodeWires = AnodeVoltage;

  int na=0;
  std::cout<<"TPC::init() ---> Anode Wires: ";
  std::ofstream faw("./wiresmap.dat");
  for(int i=0; i<NanodeWires; ++i)
    {
      double xw, yw;
      GetAnodePosition(i,xw,yw);
      TString wname = TString::Format("a%d",i);
      AddWire(xw,yw,diamAnodeWires,VoltAnodeWires,
	      wname.Data(),FullLengthZ,
	      tensionAnodeWires,AnodeWiresDensity,trap_radius);
      std::cout<<wname<<" ";
      faw<<i<<"\t"<<xw<<"\t"<<yw<<std::endl;
      ++na;
    }
  faw.close();
  std::cout<<"TPC::init() ---> Number of Anode Wires: "<<na<<std::endl;

  // Activate "Weighting Fields"
  for(int a=0; a<NanodeWires; ++a)
    {
      TString wname = TString::Format("a%d",a);
      anodes.push_back(wname.Data());
      AddReadout(wname.Data());
    }
  AddReadout("ro");
  readouts.push_back("ro");

  if( medium )
    {
      double AngleOffsetAnodeWires = 0.5*GetAnodePitch();
      double x = (AnodeWiresR+0.5005*diamAnodeWires)*TMath::Cos(AngleOffsetAnodeWires);
      double y = (AnodeWiresR+0.5005*diamAnodeWires)*TMath::Sin(AngleOffsetAnodeWires);
      double ex, ey, ez;
      int st;
      ElectricField(x,y,0.0,ex,ey,ez,medium,st);
      double ex2;
      ElectricField(x-1.001*diamAnodeWires,y,0.0,ex2,ey,ez,medium,st);

      ex = abs(ex); ex2 = abs(ex2);
      std::cout << "TPC::init() Field 0.1% outside of anode wire: " << ex2 << " V/cm (inside), " << ex << " V/cm (outside)" << std::endl;
    }
  std::cout<<"TPC::init() ---> drift cell Type: "<<GetCellType()<<std::endl;
}

void TPC::SetGas(Medium *m)
{
  std::cout<<"TPC::SetGas(Medium*)"<<std::endl;
  medium = m;
  std::cout<<"TPC::SetGas Temperature: "<<medium->GetTemperature()
	   <<" K\tPressure: "<<medium->GetPressure()<<" torr"<<std::endl;
  geo.Clear();
  chamber = new SolidTube(0., 0., 0.,
			  CathodeRadius, ROradius,
			  FullLengthZ);
  geo.AddSolid(chamber,medium);
 
  vector<double> ef, bf, ang;
  double bmin, bmax;
  GetBRange(bmin, bmax);
  medium->GetFieldGrid(ef, bf, ang);
  if(!( bmin >= bf.front() && bmax <= bf.back())) 
    std::cerr << "TPC::init() B-Field " << bmin << ":" << bmax 
	      << " outside of Magboltz file scope: " << bf.front() 
	      << " to " << bf.back() << std::endl;
  //    assert( bmin >= bf.front() && bmax <= bf.back());

  SetGeometry(&geo);
}


void TPC::SetVoltage(double &vc, double& vaw, double& vfw)
{
  CathodeVoltage = vc;
  AnodeVoltage = vaw;
  FieldVoltage = vfw;
}
