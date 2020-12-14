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
a2mcFieldFromMap::a2mcFieldFromMap(std::string fName)
  : TVirtualMagField("a2mcBField") {
      fFileName = fName;
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
//    if ( fBx ) delete fBx;
//    if ( fBy ) delete fBy;
//    if ( fBz ) delete fBz;
    fBx.clear();
    fBy.clear();
    fBz.clear();
    fgInstance = 0;
}
// ------------------------------------------------------------------------

// -----------   Intialisation   ------------------------------------------
void a2mcFieldFromMap::Init() {
    fXmin  = fYmin  = fZmin  = 0.;
    fXmax  = fYmax  = fZmax  = 0.;
    fXstep = fYstep = fZstep = 2.;
    fNx    = fNy    = fNz    = 0;
    fScale = 1.;        ///< In case of need for a scaled-down magnetic field
    fUnit = 10.0;       ///< VirtualMagField expect values in kG, while the map file is in Tesla -> * 10.
//    fBx    = fBy    = fBz    = NULL;
    fBx.clear();
    fBy.clear();
    fBz.clear();
    ReadAsciiFile(fFileName);
}

// ------------------------------------------------------------------------
// -----   Read field map from ASCII file (private)   ---------------------
void a2mcFieldFromMap::ReadAsciiFile(std::string& fileName) {

    // Open file
    std::cerr << "-I- a2mcFieldFromMap: Reading field map from ASCII file " << fileName.c_str() << std::endl;
    std::ifstream mapFile(fileName.c_str());
    if (!mapFile.is_open()) {
        std::cerr << "-E- a2mcFieldFromMap:ReadAsciiFile: Could not open file! " << std::endl;
        Fatal("ReadAsciiFile","Could not open file");
    }   

    std::string line, dummy;
    size_t found;
    ///< Read the limits
    ///< LIMITS ON X
    for(UInt_t i=0; i<3; i++) {
        getline(mapFile,line);
        if(line.find("LIMIT X")!=std::string::npos) {
            std::istringstream iss(line);
            iss >> dummy >> dummy >> fXmin >> fXmax;
        }
        if(line.find("LIMIT Y")!=std::string::npos) {
            std::istringstream iss(line);
            iss >> dummy >> dummy >> fYmin >> fYmax;
        }
        if(line.find("LIMIT Z")!=std::string::npos) {
            std::istringstream iss(line);
            iss >> dummy >> dummy >> fZmin >> fZmax;
        }
    }
    ///< Guessing the number of points, assuming a 2 cm step (2x2x2 cm^3 cubic voxel)
    fNx = (fXmax - fXmin)/(int)(fXstep) + 1;
    fNy = (fYmax - fYmin)/(int)(fYstep) + 1;
    fNz = (fZmax - fZmin)/(int)(fZstep) + 1;
    getline(mapFile,line); ///< Empty line
    getline(mapFile,line); ///< "X Y Z BX BY BZ" line
    found = line.find("X Y Z BX BY BZ");
    if(found==std::string::npos) { ///< Cross check that everything is fine
        std::cout << "a2mcFieldFromMap::ReadAsciiFile -> PLEASE CHECK INPUT MAGNETIC FIELD MAP FORMAT" << std::endl;
    }
    Float_t factor = fScale * fUnit;   // Factor 1/1000 for G -> kG
    Float_t bx=0., by=0., bz=0.;
    Float_t rx=0., ry=0., rz=0.;
    while (getline(mapFile,line)) {
        std::istringstream iss(line);
        iss >> rx >> ry >> rz >> bx >> by >> bz;
        fRx.push_back(rx);
        fRy.push_back(ry);
        fRz.push_back(rz);
        fBx.push_back(bx*factor);
        fBy.push_back(by*factor);
        fBz.push_back(bz*factor);
    }
    if((UInt_t)fBz.size()!=fNx*fNy*fNz) {
        std::cout << "a2mcFieldFromMap::ReadAsciiFile -> the number of points is not what expected (fNx*fNy*fNz)" << std::endl;
    }
    
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

    if (IsInside(x, y, z, ix, iy, iz, dx, dy, dz)) {

        // Get Bx field values at grid cell corners
        fHa[0][0][0] = fBx.at(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBx.at((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBx.at(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBx.at((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBx.at(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBx.at((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBx.at(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBx.at((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

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

    if (IsInside(x, y, z, ix, iy, iz, dx, dy, dz)) {

        // Get By field values at grid cell corners
        fHa[0][0][0] = fBy.at(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBy.at((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBy.at(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBy.at((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBy.at(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBy.at((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBy.at(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBy.at((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

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

    if (IsInside(x, y, z, ix, iy, iz, dx, dy, dz)) {

        // Get Bz field values at grid cell corners
        fHa[0][0][0] = fBz.at(ix    *fNy*fNz + iy    *fNz + iz);
        fHa[1][0][0] = fBz.at((ix+1)*fNy*fNz + iy    *fNz + iz);
        fHa[0][1][0] = fBz.at(ix    *fNy*fNz + (iy+1)*fNz + iz);
        fHa[1][1][0] = fBz.at((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
        fHa[0][0][1] = fBz.at(ix    *fNy*fNz + iy    *fNz + (iz+1));
        fHa[1][0][1] = fBz.at((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
        fHa[0][1][1] = fBz.at(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
        fHa[1][1][1] = fBz.at((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

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
    Double_t xl = x;
    Double_t yl = y;
    Double_t zl = z;

  // ---  Check for being outside the map range
    if (!(xl >= fXmin && xl < fXmax && yl >= fYmin && yl < fYmax && zl >= fZmin && zl < fZmax)) {
        ix = iy = iz = 0;
        dx = dy = dz = 0.;
        return kFALSE;
    }
 
    // --- Determine grid cell ("left" element)
    ix = Int_t((xl-fXmin)/fXstep);
    iy = Int_t((yl-fYmin)/fYstep);
    iz = Int_t((zl-fZmin)/fZstep);


    // Relative distance from grid point (in units of cell size)
    // G. Bonomi: introducing cast to (float) to avoid problems of double precision rounding
    // This was the problem: 
    // xl = -4.8, fXmin = -5.0, fXstep = 0.2, ix = 1, nevertheless dx was something like 10.e-15
    // and this because in double precision xl was something like -4-7999999999999999982 (the same for fXmin and fXstep)
    dx = (Float_t)((xl-fXmin)/fXstep - Double_t(ix));
    dy = (Float_t)((yl-fYmin)/fYstep - Double_t(iy));
    dz = (Float_t)((zl-fZmin)/fZstep - Double_t(iz));
    
//    ///< Cross check that the correspondance index -> correct cell is OK
//    Int_t index = ix*fNy*fNz + iy*fNz + iz;
//    if(fabs(x-fRx[index])>=fXstep) std::cout << "ERROR on X " << std::endl;
//    if(fabs(y-fRy[index])>=fYstep) std::cout << "ERROR on Y " << std::endl;
//    if(fabs(z-fRz[index])>=fZstep) std::cout << "ERROR on Z " << std::endl;

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
    Double_t lower_limit = 1.e-12; ///< To avoind double precision fluctuations around 0
    Double_t xx,yy,zz;
    xx = x[0]; yy = x[1]; zz = x[2];
    if(fabs(xx)<lower_limit) xx = 0.;
    if(fabs(yy)<lower_limit) yy = 0.;
    if(fabs(zz)<lower_limit) zz = 0.;
    B[0] = GetBx(xx,yy,zz);
    B[1] = GetBy(xx,yy,zz);
    B[2] = GetBz(xx,yy,zz);
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