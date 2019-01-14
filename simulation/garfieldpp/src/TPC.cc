#include <iostream>
#include <fstream>

#include <TMath.h>
#include <TString.h>

#include <GeometrySimple.hh>
#include <SolidTube.hh>
#include <Medium.hh>

#include "ComponentBmap.hh"
#include "TPC.hh"

using namespace std;
using namespace Garfield;

// /// I don't know why, but these have to be declared here in order to compile, even though they're defined in the header.
// constexpr double TPCBase::CathodeRadius;
// constexpr double TPCBase::FieldWiresR;
// constexpr double TPCBase::AnodeWiresR;
// constexpr double TPCBase::ROradius;
// constexpr double TPCBase::HalfWidthZ;
// constexpr double TPCBase::diamFieldWires;
// constexpr double TPCBase::tensionFieldWires;
// constexpr double TPCBase::diamAnodeWires;
// constexpr double TPCBase::tensionAnodeWires;
// constexpr double TPCBase::AnodeWiresDensity;
// constexpr double TPCBase::PadSideZ;

void TPC::init(){
  
  if(fPrototype)
    cout<<"Proto-rTPC\t";
  else
    cout<<"ALPHA-g rTPC\t";
  cout<<"length: "<<FullLengthZ<<" cm"<<endl;
  
    const double BigHalfWidthZ = 1.25*HalfLengthZ;

    DisableDebugging();

    // outer wall of the TPC
    AddTube(ROradius,0.0,0,"ro");

    // define cathodic pads
  double PadPosZmax = ((double) npads) * 0.5 * PadSideZ;
  double PadPosZmin = PadPosZmax - PadSideZ;
  const double gapZ = ROradius - AnodeWiresR;
  cout<<"\n=== PADS POSITIONING ==="<<endl;
  for(int ip=0; ip<npads; ++ip)
    {
      TString padname = TString::Format("p%d",ip);

      cout<<ip<<"\t"<<PadPosZmin<<"\t"<<PadPosZmax<<"\t"<<padname<<endl;
      // AddStripOnTube(ROradius, PadPosZmin,PadPosZmax,
      // 		     padname.Data(), gapZ);

      pads.push_back(padname.Data());
      // Activate "Weighting Fields"
      //AddReadout(padname.Data());

      PadPosZmax  = PadPosZmin;
      PadPosZmin -= PadSideZ;
    }

    // inner wall of the TPC
    const double IV = CathodeVoltage;
    //  AddWire(0.,0.,2.*CathodeRadius,IV,"c",2.*HalfWidthZ,<tension>,<density>1.21,<trap radius>);
    AddWire(0.,0.,2.*CathodeRadius,IV,"c",2*BigHalfWidthZ);

    // Field Wires

    const double VoltFieldWires = FieldVoltage;

    double AngleFieldWires = TMath::TwoPi() / double(NfieldWires);
    double nf=0.0;
    for(int i=0; i<NfieldWires; ++i)
        {
            double phi = AngleFieldWires*nf;
            const double xw = FieldWiresR*TMath::Cos(phi);
            const double yw = FieldWiresR*TMath::Sin(phi);
            AddWire(xw,yw,diamFieldWires,VoltFieldWires,
                    "f",2*BigHalfWidthZ,tensionFieldWires);
            ++nf;
        }
    cout<<"---> Number of Field Wires: "<<nf<<endl;

    // Anode Wires

    const double VoltAnodeWires = AnodeVoltage;

    double AngleAnodeWires = GetAnodePitch();
    double AngleOffsetAnodeWires = 0.5*AngleAnodeWires;
    double na=0.0;
    cout<<"\nAnode Wires: ";
    ofstream faw("./wiresmap.dat");
    for(int i=0; i<NanodeWires; ++i)
        {
            double phi = AngleAnodeWires * na + AngleOffsetAnodeWires;
            const double xw = AnodeWiresR*TMath::Cos(phi);
            const double yw = AnodeWiresR*TMath::Sin(phi);
            TString wname = TString::Format("a%d",i);
            AddWire(xw,yw,diamAnodeWires,VoltAnodeWires,
                    wname.Data(),2*BigHalfWidthZ,
                    tensionAnodeWires,AnodeWiresDensity,trap_radius);
            cout<<wname<<" ";
            faw<<i<<"\t"<<xw<<"\t"<<yw<<endl;
            ++na;
        }
    faw.close();
    cout<<"\n---> Number of Anode Wires: "<<na<<endl;

    // Activate "Weighting Fields"
    for(int a=0; a<NanodeWires; ++a)
        {
            TString wname = TString::Format("a%d",a);
            anodes.push_back(wname.Data());
            AddReadout(wname.Data());
        }
    AddReadout("ro");
    readouts.push_back("ro");

    medium = new Medium;
    SetGas(medium);

    double x = (AnodeWiresR+0.5005*diamAnodeWires)*TMath::Cos(AngleOffsetAnodeWires);
    double y = (AnodeWiresR+0.5005*diamAnodeWires)*TMath::Sin(AngleOffsetAnodeWires);
    double ex, ey, ez;
    int st;
    ElectricField(x,y,0,ex,ey,ez,medium,st);
    double ex2;
    ElectricField(x-1.001*diamAnodeWires,y,0,ex2,ey,ez,medium,st);

    ex = abs(ex); ex2 = abs(ex2);
    cout << "Field 0.1% outside of anode wire: " << ex2 << " V/cm (inside), " << ex << " V/cm (outside)" << endl;

    cout<<" ---> drift cell Type: "<<GetCellType()<<endl;
}

void TPC::SetGas(Medium *m){
    geo.Clear();
    const double BigHalfWidthZ = 1.25*HalfLengthZ;
    chamber = new SolidTube(0., 0., 0.,
                                       0., ROradius+1.,
                                       BigHalfWidthZ);
    geo.AddSolid(chamber,m);
    if(medium && m != medium){
        delete medium;
        medium = 0;
    }
    vector<double> ef, bf, ang;
    double bmin, bmax;
    GetBRange(bmin, bmax);
    m->GetFieldGrid(ef, bf, ang);
    if(!( bmin >= bf.front() && bmax <= bf.back())) std::cerr << "B-Field " << bmin << ":" << bmax << " outside of Magboltz file scope: " << bf.front() << " to " << bf.back() << std::endl;
    //    assert( bmin >= bf.front() && bmax <= bf.back());
    SetGeometry(&geo);
}
