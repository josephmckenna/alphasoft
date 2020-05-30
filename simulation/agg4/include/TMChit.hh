#ifndef __TMChit__
#define __TMChit__

#include <TObject.h>

class TMChit: public TObject
{
private:
  int fID;
  int fPDG;

  double fx; // in mm
  double fy;
  double fz;
  double fr;
  double fp; // in rad
  double ft; // in ns
  double fEdep; // in eV
  
public:
  TMChit() {};
  TMChit(int, int, double, double, double);
  TMChit(int, int, double, double, double, double, double);
  TMChit(int, int, double, double, double, double);
  TMChit(int, int, double, double, double, double, double, double, double);

  inline int GetTrackID()  { return fID; }
  inline int GetTrackPDG() { return fPDG; }

  inline double GetX() const {return fx;}
  inline double GetY() const {return fy;}
  inline double GetZ() const {return fz;}

  inline double GetT()      const {return ft;}
  inline double GetTime()   const {return ft;}

  inline double GetR()      const {return fr;}
  inline double GetRadius() const {return fr;}
  inline double GetPhi()    const {return fp;}

  inline double GetDepositEnergy() const { return fEdep; }


  ClassDef(TMChit,2)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
