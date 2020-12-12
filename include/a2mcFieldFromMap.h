#ifndef a2mc_MAP_FIELD_H
#define a2mc_MAP_FIELD_H

#include <TVirtualMagField.h>
#include <TLorentzVector.h>
#include "TArrayF.h"
#include <iostream>
#include <fstream>

class a2mcFieldFromMap : public TVirtualMagField
{
public:
   a2mcFieldFromMap(Double_t Bx, Double_t By, Double_t Bz); 
   a2mcFieldFromMap();
  /** Standard constructor
   ** @param name       Name of field map
   ** @param fileType   R = ROOT file, A = ASCII
   **/
   a2mcFieldFromMap(std::string&);

   virtual ~a2mcFieldFromMap();
   

   /** Get the field components at a certain point 
    ** @param x,y,z     Point coordinates (global) [cm]
    ** @value Bx,By,Bz  Field components [kG]
    **/
   static a2mcFieldFromMap* Instance(); 
   virtual Double_t GetBx(Double_t x, Double_t y, Double_t z);
   virtual Double_t GetBy(Double_t x, Double_t y, Double_t z);
   virtual Double_t GetBz(Double_t x, Double_t y, Double_t z);
   Double_t GetArray(Int_t);
   /** Determine whether a point is inside the field map
    ** @param x,y,z              Point coordinates (global) [cm]
    ** @param ix,iy,iz (return)  Grid cell
    ** @param dx,dy,dz (return)  Distance from grid point [cm] if inside
    ** @value kTRUE if inside map, else kFALSE
    **/
   virtual Bool_t IsInside(Double_t x, Double_t y, Double_t z,
			  Int_t& ix, Int_t& iy, Int_t& iz,
			  Double_t& dx, Double_t& dy, Double_t& dz);



   virtual void Field(const Double_t* /*x*/, Double_t* B);

  /** Initialisation (read map from file) **/
  void Init();

  /** Read the field map from an ASCII file **/
  void ReadAsciiFile(std::string&);

  /** Get field values by interpolation of the grid.
   ** @param dx,dy,dz  Relative distance from grid point [cell units]
   **/
  Double_t Interpolate(Double_t dx, Double_t dy, Double_t dz); 

  //! calculated the gradient for B field at Point
  void CalculateGradient(TLorentzVector pos, Double_t *field);

private:
   static  a2mcFieldFromMap* fgInstance; ///< Singleton instance
   a2mcFieldFromMap(const a2mcFieldFromMap&);
   a2mcFieldFromMap& operator=(const a2mcFieldFromMap&);
   
   /** Variables **/
   /** Map file name **/
   std::string fFileName;


   /** Global scaling factor (w.r.t. map on file) **/
   Double_t fScale;             

   /** Units used in map file**/
   Double_t funit;             


   /** Field centre position in global coordinates  **/
   Double_t fPosX, fPosY, fPosZ; 


   /** Field limits in local coordinate system **/
   Double_t fXmin, fXmax, fXstep;
   Double_t fYmin, fYmax, fYstep;
   Double_t fZmin, fZmax, fZstep;


   /** Number of grid points  **/
   Int_t fNx, fNy, fNz;


   /** Arrays with the field values  **/
   TArrayF* fBx;
   TArrayF* fBy;
   TArrayF* fBz;


   /** Variables for temporary storage 
     ** Used in the very frequently called method GetFieldValue  **/
   Double_t fHa[2][2][2];            //! Field at corners of a grid cell
   Double_t fHb[2][2];               //! Interpolated field (2-dim)
   Double_t fHc[2];                  //! Interpolated field (1-dim)

   
   Double_t  fB[3]; ///< Magnetic field vector
   Double_t  fArray[20];
   ClassDef(a2mcFieldFromMap, 1)  // Mapped magnetic field        
};

#endif //a2mc_MAP_FIELD_H
