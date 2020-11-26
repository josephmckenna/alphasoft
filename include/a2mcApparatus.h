#ifndef a2mc_APPARATUS_H
#define a2mc_APPARATUS_H

#include <TVirtualMC.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>

#include "a2mcMessenger.h"
#include "a2mcSettings.h"

class a2mcApparatus : public TObject
{
    public:  
        a2mcApparatus(Int_t);
        virtual ~a2mcApparatus();
        // static access method
        static a2mcApparatus* Instance();

    private:
        static  a2mcApparatus* fgInstance; ///< Singleton instance
        a2mcSettings a2mcConf{};
        Int_t runNumber;

        // Setting booleans to choose what to insert into the a2mcApparatus

        // Geometry stuff
        TGeoVolume *top, *Frame, *Roof;
        TGeoRotation *nullRot;
        TGeoRotation *fiberRot[2];
        // Medium Id for various materials
        Int_t   fmedAir, fmedVacuum;
        Int_t   fmedAl, fmedCu, fmedFe, fmedSi, fmedNb, fmedCsI, fmedLiqHe;
        Int_t   fmedLiqN, fmedSteel316LN, fmedCuNbTi, fmedAlT6082, fmedEpoxy;
        Int_t   kcolAl, kcolCu, kcolFe, kcolSi, kcolNb, kcolCsI, kcolLiqHe;
        Int_t   kcolLiqN, kcolSteel316LN, kcolCuNbTi, kcolAlT6082, kcolEpoxy;
        // Geometry objects size
        // Geometrical constants (values are in cm)
        // Y coordinate is the vertical one
        //  (y)| 
        //     |  / (x)
        //     | /
        //     |/_________ (z)
        // 

        // ===================================================================
        // WORLD 
        // ===================================================================
        Double_t  fWorld_Dx;   // X size of the world volume
        Double_t  fWorld_Dy;   // Y size of the world volume
        Double_t  fWorld_Dz;   // Z size of the world volume

        // ===================================================================
        // ENVIRONMENT (inner and outer with respect the silicon detector) 
        // ===================================================================
        Double_t  oxfordMag_rMin, oxfordMag_rMax, oxfordMag_halfZ; 
        Double_t  vacuumChamber_rMin, vacuumChamber_rMax, vacuumChamber_halfZ;
        Double_t  rInnMax, rOutMin;
        // ===================================================================
        // SILICON DETECTOR 
        // ===================================================================
        static const UInt_t nHalves  = 2;
        static const UInt_t nLayers  = 3;
        UInt_t nModules[nLayers] = {10,12,14};
        Double_t silDet_rMin, silDet_rMax, silDet_halfZ, silDet_posZ;
        Int_t SilModPos(UInt_t, UInt_t, UInt_t, Double_t&, Double_t&, Double_t&, Double_t&, Double_t&);

    public:
        void Init();
        void ConstructMaterials();
        void ConstructGeometry();
        void PrintNode(TGeoNode*, Int_t, Double_t[3]);

        void SetCuts();

        void InsertWorld();
        void InsertInnEnviro();
        void InsertSilDet();
        void InsertOutEnviro();

        inline Int_t GetRunNumber() { return runNumber;};
        Bool_t GetStoreTracks()  {return a2mcConf.GetStoreTracks();};

        Double_t GetWorldDx()    {return fWorld_Dx;}; 
        Double_t GetWorldDy()    {return fWorld_Dy;}; 
        Double_t GetWorldDz()    {return fWorld_Dz;}; 

        Double_t GetOxfMag_R()   {return oxfordMag_rMax;};
        Double_t GetOxfMag_L()   {return oxfordMag_halfZ*2.;};

        Double_t GetSilDet_Z()   {return silDet_posZ;};
        Double_t GetSilDet_R()   {return silDet_rMax;};
        Double_t GetSilDet_L()   {return silDet_halfZ*2.;};
        
        ClassDef(a2mcApparatus,1) //a2mcApparatus
};

#endif //a2mc_APPARATUS_H
