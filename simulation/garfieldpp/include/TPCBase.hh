#ifndef TPCBASE_H
#define TPCBASE_H

#include <iostream>
#include <vector>
#include <string>

#include <cmath>

static const double kUnknown = -9999999;

class TPCBase{
public:

   TPCBase(bool proto=false);

   void GetAnodePosition(int i, double &x_R, double &y_phi, bool mm=false, bool polar=false);
   void GetWirePosition(int i, double &x_R, double &y_phi, bool mm=false, bool polar=false);
   void GetPadPosition(int i, double &z); // no phi segmentation
   void GetPadPosition(int i, double &z, double &phi);
   void GetPadPosition(int i,int s, double &z, double &phi);
   std::pair<int,int> FindPad(const double z, const double phi = 0);
   unsigned int FindAnode(const double phi);

   int SectorAndPad2Index(const int padsec, const int pad);
   int SectorAndPad2Index(std::pair<int,int> p);
   std::pair<int,int> Index2SectorAndPad(const int padindex);

   struct electrode{
      electrode(short s, int ind, double g = 1.){ sec = s; i = ind; gain = g;};
      short sec;  // for anodes sec=0 for top, sec=1 for bottom
      int i;
      double gain;
      void print(){ printf("TPCBase::electrode %d sector: %d (gain: %1.0f)\n",i,sec,gain); };
   };

   struct electrode_cmp
   {
      bool operator() (const electrode& lhs, const electrode& rhs) const
      {
         return (lhs.sec < rhs.sec) || (lhs.sec == rhs.sec && lhs.i < rhs.i);
      }
   };


   int MapElectrodes(short run,
                     std::vector<electrode> &anodes,
                     std::vector<electrode> &pads);

   void SetPrototype(bool flag);
   bool GetPrototype() const { return fPrototype;};

   void SetPhi0(const double phi0_){ phi0 = phi0_; };
   inline double GetPhi0() const { return phi0; };

   inline double GetCathodeRadius(bool mm=false) const
   { if(mm) return CathodeRadius*10.;
      else return CathodeRadius; }
   inline double GetFieldWiresRadius(bool mm=false) const
   { if(mm) return FieldWiresR*10.;
      else return FieldWiresR; }
   inline double GetAnodeWiresRadius(bool mm=false) const
   { if(mm) return AnodeWiresR*10.;
      else return AnodeWiresR; }
   inline double GetROradius(bool mm=false) const
   { if(mm) return ROradius*10.;
      else return ROradius;}
   inline int GetNumberOfFieldWires() const { return NfieldWires; }
   inline int GetNumberOfAnodeWires() const { return NanodeWires; }
   inline double GetHalfLengthZ(bool mm=false) const
   { if(mm) return HalfLengthZ*10.;
      else return HalfLengthZ; }
   inline double GetFullLengthZ(bool mm=false) const
   { if(mm) return FullLengthZ*10.;
      else return FullLengthZ; }

   inline double GetAnodePitch() const { return AnodePitch; }

   inline double GetDiameterFieldWires() const { return diamFieldWires; }
   inline double GetTensionFieldWires() const { return tensionFieldWires; }
   inline double GetDiameterAnodeWires() const { return diamAnodeWires; }
   inline double GetTensionAnodeWires() const { return tensionAnodeWires; }
   inline double GetDensityAnodeWires() const { return AnodeWiresDensity; }

   inline int GetChargeTrapRadius() const { return trap_radius; } // garfield++ use

   inline double GetPadPitchZ(bool mm=false) const
   { if(mm) return PadSideZ*10.;
      else return PadSideZ; }
   inline int GetNumberPadsColumn() const { return npads; }
   inline int GetNumberPadsRow() const { return npadsec; }
   inline double GetPadPitchPhi() const { return PadWidthPhi; }
   inline int GetNumberOfPads() const { return totpads; }

   inline double GetTrapRadius(bool mm=false) const
   { if(mm) return TrapR*10.;
      else return TrapR; }

   static TPCBase* TPCBaseInstance();

protected:
   bool fPrototype;
   const double CathodeRadius; // cm
   const double FieldWiresR;
   const double AnodeWiresR;
   double ROradius;
   const int NanodeWires;
   const int NfieldWires;
   double AnodePitch;
   double HalfLengthZ;
   double FullLengthZ;
   double diamFieldWires;
   const double tensionFieldWires;
   const double diamAnodeWires;
   const double tensionAnodeWires;
   const double AnodeWiresDensity;
   int trap_radius;
   const double PadSideZ;
   int npads;
   const int npadsec;
   int totpads;
   double PadWidthPhi;
   const double TrapR;
   double phi0;

private:
   static TPCBase* fTPC;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
