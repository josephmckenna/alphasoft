///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcFieldFromMap.h"

ClassImp(a2mcFieldFromMap)

a2mcFieldFromMap* a2mcFieldFromMap::fgInstance = 0;
//______________________________________________________________________________
a2mcFieldFromMap::a2mcFieldFromMap(Double_t Bx, Double_t By, Double_t Bz)
  : TVirtualMagField("a2mc magnetic field")
{
///< Standard constructor
///<    Bx -> the x component of the field value (in kiloGauss)
///<    By -> the y component of the field value (in kiloGauss)
///<    Bz -> the z component of the field value (in kiloGauss)

    if (fgInstance) {
        Fatal("a2mcFieldFromMap", "Singleton instance already exists.");
        return;
    }
    fgInstance = this;
    fB[0] = Bx;
    fB[1] = By;
    fB[2] = Bz;
}

// -------------   Standard constructor   ---------------------------------
a2mcFieldFromMap::a2mcFieldFromMap(std::string& fName)
  : TVirtualMagField("a2mcBField") {
      fPosX  = fPosY  = fPosZ  = 0.;
      fXmin  = fYmin  = fZmin  = 0.;
      fXmax  = fYmax  = fZmax  = 0.;
      fXstep = fYstep = fZstep = 0.;
      fNx    = fNy    = fNz    = 0;
      fScale = 1.; ///< In case of need for a scaled-down magnetic field
      funit = 10.0;
      fBx    = fBy    = fBz    = NULL;
      fFileName = fName;
      fArray[0] = -999.;
      Init();
      if (fgInstance) {
          Fatal("a2mcFieldFromMap", "Singleton instance already exists.");
          return;
      }
      fgInstance = this;
}
// ------------------------------------------------------------------------
//______________________________________________________________________________
a2mcFieldFromMap::a2mcFieldFromMap()
  : TVirtualMagField()
{
/// Default constructor
   fB[0] = 0.;
   fB[1] = 0.;
   fB[2] = 0.;
/// Default constructor
    if (fgInstance) {
        Fatal("a2mcFieldFromMap", "Singleton instance already exists.");
        return;
    }
    fgInstance = this;
}

// ------------   Destructor   --------------------------------------------
a2mcFieldFromMap::~a2mcFieldFromMap() {
    if ( fBx ) delete fBx;
    if ( fBy ) delete fBy;
    if ( fBz ) delete fBz;
    fgInstance = 0;
}
// ------------------------------------------------------------------------

// -----------   Intialisation   ------------------------------------------
void a2mcFieldFromMap::Init() {
    ReadAsciiFile(fFileName);
}

// ------------------------------------------------------------------------
// -----   Read field map from ASCII file (private)   ---------------------
void a2mcFieldFromMap::ReadAsciiFile(std::string& fileName) {

    Double_t bx=0., by=0., bz=0.;

    // Open file
    cerr << "-I- a2mcFieldFromMap: Reading field map from ASCII file " << fileName.c_str() << endl;
    ifstream mapFile(fileName.c_str());
    if ( ! mapFile.is_open() ) {
        cerr << "-E- a2mcFieldFromMap:ReadAsciiFile: Could not open file! " << endl;
        Fatal("ReadAsciiFile","Could not open file");
    }

    // Read map type
    TString type;
    mapFile >> type;

    Int_t iType = 0;
    if ( type == "nosym" ) iType = 1;
    // Read Units
    TString unit;
    mapFile >> unit;
    if ( unit == "G" ) funit = 0.001;
    else if ( unit == "T"  ) funit = 10.0;
    else if ( unit == "kG"  ) funit=1.0;
    else {
        cout << "-E- FieldMap::ReadAsciiFile: No units!" << endl;
        Fatal("ReadAsciiFile","No units defined");
    }

    // Read grid parameters
    mapFile >> fXmin >> fXmax >> fNx;
    mapFile >> fYmin >> fYmax >> fNy;
    mapFile >> fZmin >> fZmax >> fNz;
    fXstep = ( fXmax - fXmin ) / Double_t( fNx - 1 );
    fYstep = ( fYmax - fYmin ) / Double_t( fNy - 1 );
    fZstep = ( fZmax - fZmin ) / Double_t( fNz - 1 );
    // Create field arrays
    fBx = new TArrayF(fNx * fNy * fNz);
    fBy = new TArrayF(fNx * fNy * fNz);
    fBz = new TArrayF(fNx * fNy * fNz);

    // Read the field values
    Double_t factor = fScale * funit;   // Factor 1/1000 for G -> kG
    cout << right;
    Int_t nTot = fNx * fNy * fNz;
    cout << "-I- a2mcFieldFromMap: " << nTot << " entries to read... " << setw(3) << 0 << " % ";
    Int_t index = 0;
    div_t modul;
    Int_t iDiv = TMath::Nint(nTot/100.);
    for (Int_t ix=0; ix<fNx; ix++) {
        for (Int_t iy = 0; iy<fNy; iy++) {
            for (Int_t iz = 0; iz<fNz; iz++) {
                if (! mapFile.good()) cerr << "-E- a2mcFieldFromMap::ReadAsciiFile: "
                    << "I/O Error at " << ix << " "
                    << iy << " " << iz << endl;
                index = ix*fNy*fNz + iy*fNz + iz;
                modul = div(index,iDiv);
                mapFile >>  bx >> by >> bz;
                fBx->AddAt(factor*bx, index);
                fBy->AddAt(factor*by, index);
                fBz->AddAt(factor*bz, index);
                if ( mapFile.eof() ) {
                    cerr << endl << "-E- a2mcFieldFromMap::ReadAsciiFile: EOF"
                         << " reached at " << ix << " " << iy << " " << iz << endl;
                    mapFile.close();
                    break;
                }
            }   // z-Loop
        }     // y-Loop
    }       // x-Loop

    cout << "   " << index+1 << " read" << endl;

    mapFile.close();
}
//------------------------------------------------------------------------
// -----------   Get x component of the field   ---------------------------
Double_t a2mcFieldFromMap::GetArray(Int_t i) {
    return fArray[i];
}
// ------------------------------------------------------------------------
// -----------   Get x component of the field   ---------------------------
Double_t a2mcFieldFromMap::GetBx(Double_t x, Double_t y, Double_t z) {

    Int_t ix    = 0;
    Int_t iy    = 0;
    Int_t iz    = 0;
    Double_t dx = 0.;
    Double_t dy = 0.;
    Double_t dz = 0.;

    if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

        // Get Bx field values at grid cell corners
        fHa[0][0][0] = fBx->At(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBx->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

        // Return interpolated field value
        return Interpolate(dx, dy, dz);
    }
    return 0.;
}
// ------------------------------------------------------------------------



// -----------   Get y component of the field   ---------------------------
Double_t a2mcFieldFromMap::GetBy(Double_t x, Double_t y, Double_t z) {

    Int_t ix    = 0;
    Int_t iy    = 0;
    Int_t iz    = 0;
    Double_t dx = 0.;
    Double_t dy = 0.;
    Double_t dz = 0.;

    if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

        // Get By field values at grid cell corners
        fHa[0][0][0] = fBy->At(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBy->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

        // Return interpolated field value
        return Interpolate(dx, dy, dz);
    }
    return 0.;
}
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------


// -----------   Get z component of the field   ---------------------------
Double_t a2mcFieldFromMap::GetBz(Double_t x, Double_t y, Double_t z) {

    Int_t ix    = 0;
    Int_t iy    = 0;
    Int_t iz    = 0;
    Double_t dx = 0.;
    Double_t dy = 0.;
    Double_t dz = 0.;

    if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

        // Get Bz field values at grid cell corners
        fHa[0][0][0] = fBz->At(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBz->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

        // Return interpolated field value
        return Interpolate(dx, dy, dz);
    }
    return 0.;
}

// -----------   Check whether a point is inside the map   ----------------
Bool_t a2mcFieldFromMap::IsInside(Double_t x, Double_t y, Double_t z,
			     Int_t& ix, Int_t& iy, Int_t& iz,
			     Double_t& dx, Double_t& dy, Double_t& dz) {

    // --- Transform into local coordinate system
    Double_t xl = x - fPosX;
    Double_t yl = y - fPosY;
    Double_t zl = z - fPosZ;

  // ---  Check for being outside the map range
    if ( ! ( xl >= fXmin && xl < fXmax && yl >= fYmin && yl < fYmax && zl >= fZmin && zl < fZmax ) ) {
        ix = iy = iz = 0;
        dx = dy = dz = 0.;
        return kFALSE;
    }
 
//   // --- Determine grid cell
//   ix = Int_t( xl / fXstep );
//   iy = Int_t( yl / fYstep );
//   iz = Int_t( zl / fZstep );
// 
// 
//   // Relative distance from grid point (in units of cell size)
//   dx = xl / fXstep - Double_t(ix);
//   dy = yl / fYstep - Double_t(iy);
//   dz = zl / fZstep - Double_t(iz);

    // --- Determine grid cell
    ix = Int_t( (xl-fXmin) / fXstep );
    iy = Int_t( (yl-fYmin) / fYstep );
    iz = Int_t( (zl-fZmin) / fZstep );


    // Relative distance from grid point (in units of cell size)
    // G. Bonomi: introducing cast to (float) to avoid problems of double precision rounding
    // This was the problem: 
    // xl = -4.8, fXmin = -5.0, fXstep = 0.2, ix = 1, nevertheless dx was something like 10.e-15
    // and this because in double precision xl was something like -4-7999999999999999982 (the same for fXmin and fXstep)
    dx = (float)((xl-fXmin) / fXstep - Double_t(ix));
    dy = (float)((yl-fYmin) / fYstep - Double_t(iy));
    dz = (float)((zl-fZmin) / fZstep - Double_t(iz));
    
    return kTRUE;
}
// ------------------------------------------------------------------------
// ------------   Interpolation in a grid cell (private)  -----------------
Double_t a2mcFieldFromMap::Interpolate(Double_t dx, Double_t dy, Double_t dz) {

    // Interpolate in x coordinate
    fHb[0][0] = fHa[0][0][0] + ( fHa[1][0][0]-fHa[0][0][0] ) * dx;
    fHb[1][0] = fHa[0][1][0] + ( fHa[1][1][0]-fHa[0][1][0] ) * dx;
    fHb[0][1] = fHa[0][0][1] + ( fHa[1][0][1]-fHa[0][0][1] ) * dx;
    fHb[1][1] = fHa[0][1][1] + ( fHa[1][1][1]-fHa[0][1][1] ) * dx;

    // Interpolate in y coordinate
    fHc[0] = fHb[0][0] + ( fHb[1][0] - fHb[0][0] ) * dy;
    fHc[1] = fHb[0][1] + ( fHb[1][1] - fHb[0][1] ) * dy;

    // Interpolate in z coordinate
    return fHc[0] + ( fHc[1] - fHc[0] ) * dz;
}
//______________________________________________________________________________
void a2mcFieldFromMap::Field(const Double_t* x, Double_t* B) 
{
///< Fill in the field value B in the given position at x.
///< (In case of a uniform magnetic field the value B does not depend on 
///< the position x ) 
///< x   The position
///< B   the field value (in kiloGauss) 
    B[0] = GetBx(x[0],x[1],x[2]); 
    B[1] = GetBy(x[0],x[1],x[2]); 
    B[2] = GetBz(x[0],x[1],x[2]);
}
//_____________________________________________________________________________
a2mcFieldFromMap* a2mcFieldFromMap::Instance()
{
    return fgInstance;
} 

void a2mcFieldFromMap::CalculateGradient(TLorentzVector pos, Double_t *field){

    field[0] = this->GetBx(pos[0],pos[1],pos[2]);
    field[1] = this->GetBy(pos[0],pos[1],pos[2]);
    field[2] = this->GetBz(pos[0],pos[1],pos[2]);

    const Double_t delta = 0.1; //0.1mm Increment for derivation (gradient)

    // // mag gradient x direction
    Double_t Bx0 = this->GetBx(pos[0]+delta,pos[1],pos[2]);
    Double_t By0 = this->GetBy(pos[0]+delta,pos[1],pos[2]);
    Double_t Bz0 = this->GetBz(pos[0]+delta,pos[1],pos[2]);
    Double_t Bx1 = this->GetBx(pos[0]-delta,pos[1],pos[2]);
    Double_t By1 = this->GetBy(pos[0]-delta,pos[1],pos[2]);
    Double_t Bz1 = this->GetBz(pos[0]-delta,pos[1],pos[2]);
    field[6] =  ( sqrt(Bx0*Bx0+By0*By0+Bz0*Bz0) - sqrt(Bx1*Bx1+By1*By1+Bz1*Bz1))/(2.0*delta);

    // // mag gradient y direction
    Bx0 = this->GetBx(pos[0],pos[1]+delta,pos[2]);
    By0 = this->GetBy(pos[0],pos[1]+delta,pos[2]);
    Bz0 = this->GetBz(pos[0],pos[1]+delta,pos[2]);
    Bx1 = this->GetBx(pos[0],pos[1]-delta,pos[2]);
    By1 = this->GetBy(pos[0],pos[1]-delta,pos[2]);
    Bz1 = this->GetBz(pos[0],pos[1]-delta,pos[2]);
    field[7] =  ( sqrt(Bx0*Bx0+By0*By0+Bz0*Bz0) - sqrt(Bx1*Bx1+By1*By1+Bz1*Bz1))/(2.0*delta);

    // // mag gradient z direction
    Bx0 = this->GetBx(pos[0],pos[1],pos[2]+delta);
    By0 = this->GetBy(pos[0],pos[1],pos[2]+delta);
    Bz0 = this->GetBz(pos[0],pos[1],pos[2]+delta);
    Bx1 = this->GetBx(pos[0],pos[1],pos[2]-delta);
    By1 = this->GetBy(pos[0],pos[1],pos[2]-delta);
    Bz1 = this->GetBz(pos[0],pos[1],pos[2]-delta);
    field[8] =  ( sqrt(Bx0*Bx0+By0*By0+Bz0*Bz0) - sqrt(Bx1*Bx1+By1*By1+Bz1*Bz1))/(2.0*delta);
}