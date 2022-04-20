// SpacePoint class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __TSPACEPOINT__
#define __TSPACEPOINT__ 1

#include "TObject.h"
#include "TMath.h"

class TSpacePoint: public TObject
{
private:
   int fw;
   int fp;
   double ft;
   double fHw;
   double fHp;

   double fx;
   double fy;
   double fz;
   double fr;
   double fphi;

   double ferrx;
   double ferry;
   double ferrz;
   double ferrr;
   double ferrphi;

   unsigned short fID;           // Geant4 track ID
   short fPDG;                   // Geant4 particle type ID

public:
   TSpacePoint();
   TSpacePoint(const TSpacePoint &p);

   TSpacePoint(int anode, int pad_col, int pad_row,
               double t,
               double r, double lorentz, double z_from_pad,
               double er_str, double ep_str, double ez_from_pad,
               double WireAplitude, double PadAmplitude);
  void Setup(int anode, int pad_col, int pad_row, 
	     double t,
	     double r, double lorentz, double z_from_pad,
	     double er_str, double ep_str, double ez_from_pad,
	     double WireAplitude, double PadAmplitude);

  void Setup(int anode, int pad_col, int pad_row, 
	     double t, double phi_from_aw,
	     double r, double lorentz, double z_from_pad,
	     double ep_from_aw,
	     double er_str, double ep_str, double ez_from_pad,
	     double WireAplitude, double PadAmplitude);
  
  TSpacePoint(double x, double y, double z,
	      double ex, double ey, double ez);

   virtual ~TSpacePoint() {};

   inline void SetX(double x)     { fx=x; fphi = TMath::ATan2(fy,fx); fr = TMath::Sqrt(fx*fx+fy*fy); }
   inline void SetY(double y)     { fy=y; fphi = TMath::ATan2(fy,fx); fr = TMath::Sqrt(fx*fx+fy*fy); }

   inline void SetPad(int p)      { fp=p; }
   inline void SetZ(double z)     { fz=z; }
   inline void SetErrZ(double ez) { ferrz=ez; }

   inline void SetTrackID(unsigned short id){ fID = id; }
   inline void SetTrackPDG(short pdg){ fPDG = pdg; }

   inline int GetWire() const {return fw;}
   inline int GetPad() const  {return fp;}

   inline double GetTime() const {return ft;}

   inline double GetWireHeight() const { return fHw;}
   inline double GetPadHeight() const { return fHp;}

   inline double GetX() const {return fx;}
   inline double GetY() const {return fy;}
   inline double GetZ() const {return fz;}

   inline double GetR() const   {return fr;}
   inline double GetPhi() const {return fphi;}

   inline double GetErrX() const {return ferrx;}
   inline double GetErrY() const {return ferry;}
   inline double GetErrZ() const {return ferrz;}

   inline double GetErrR()   const {return ferrr;}
   inline double GetErrPhi() const {return ferrphi;}

   inline unsigned short GetTrackID() const { return fID; };
   inline short GetTrackPDG() const { return fPDG; };

   inline double Distance(const TSpacePoint* aPoint) const {
      double dx = fx-aPoint->fx,
         dy = fy-aPoint->fy,
         dz = fz-aPoint->fz;
      return TMath::Sqrt(dx*dx+dy*dy+dz*dz);
   }
   double MeasureRad(const TSpacePoint*) const;
   double MeasurePhi(const TSpacePoint*) const;
   double MeasureZed(const TSpacePoint*) const;
   double DistanceRphi(const TSpacePoint*) const;

   static inline bool Order(const TSpacePoint& LHS,const TSpacePoint& RHS )
   {
      bool greater = (LHS.fr > RHS.fr);
      if(greater || LHS.fr < RHS.fr){
         return greater;
      } else {                  // sorting only by R makes maps and sets think two points are equal if r is equal
         return ((LHS.fz > RHS.fz) || ((LHS.fz == RHS.fz) && (LHS.fphi > RHS.fphi)));
      }
      // Safer alternative?
      /*if (LHS.fr != RHS.fr)
         return LHS.fr > RHS.fr;
      if (LHS.fw != RHS.fw)
         return LHS.fw > RHS.fw;
      if ( LHS.fz != RHS.fz)
         return LHS.fz > RHS.fz
      return LHS.fphi > RHS.fphi;*/
   }

   // static inline bool Order( TSpacePoint LHS, TSpacePoint RHS )
   // {
   //   return LHS.ft < RHS.ft;
   // }

   inline bool IsSortable() const { return true; }
   int Compare(const TObject*) const;

   static bool RadiusOrder(const TSpacePoint*,const  TSpacePoint*);

   bool IsGood(const double&, const double&) const;
   int Check(const double&, const double&) const;

   virtual void Print(Option_t *opt="xy") const;

   ClassDef(TSpacePoint,3)
};

int SpacePointCompare(const void* a, const void* b);


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
