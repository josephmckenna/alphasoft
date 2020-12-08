// TPC Anode Wires class definition
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#ifndef __TANODE__ 
#define __TANODE__ 1

#include "TTPCelement.hh"
#include <cassert>

class TAnode: public TTPCelement
{
private:
  int fWire;
  double fPos;

  int fAnodeBins;

protected:
  std::vector<double> fZeds;

public:
  TAnode();
  TAnode(int);
  TAnode( const TAnode& );
  virtual ~TAnode();
  TAnode& operator=( const TAnode& );

  inline void SetWire(int w)          { 
    assert( w < int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires() ) ); 
    fWire = w; }
  inline int GetWire() const          { return fWire; }

  inline void SetPosition(double phi) { fPos = phi; }
  inline double GetPosition() const   { return fPos; }

  int Locate( const double phi );
  int Locate( const int w );

  inline void SetZed(double z)              { fZeds.push_back(z);}
  //  inline void SetZed(std::vector<double> z) { fZeds = z; }
  void SetZed(std::vector<double> z);
  inline double GetZed(int i) const         { return fZeds.at(i); }
  inline std::vector<double> GetZed() const { return fZeds; }

  virtual void SetSignal(double t, double w=1.);
  virtual void SetSignal(std::vector<double> signal);
  virtual void ResetSignal();

  virtual int GetNumberOfSamples() const { return fAnodeBins; }

  virtual void Reset(bool all=true);

  ClassDef(TAnode,3)
};

#endif

