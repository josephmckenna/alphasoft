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
        Int_t   fmedLiqN, fmedSteel316LN, fmedCuNbTi, fmedAlT6082, fmedEpoxy, fmedFR4;
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
        // SILICON DETECTOR (see here below for -> [lay/mod] -> name/id)
        // ===================================================================
        static const UInt_t nLayers  = 6; ///< In reality there are 3 layers (each divided in two half)
        UInt_t nModules[nLayers] = {10,12,14,10,12,14};
        Double_t silBox_rMin, silBox_rMax, silBox_halfZ, silBox_posZ;
        Double_t silPCB_halfX, silPCB_halfY, silPCB_halfZ;
        Double_t silMod_halfX, silMod_halfY, silMod_halfZ;
        Int_t SilModPos(UInt_t, UInt_t, Double_t&, Double_t&, Double_t&, Double_t&, Double_t&);
        std::map<int, std::string> silNameIDMap;

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

        Double_t GetSilDet_Z()   {return silBox_posZ;};
        Double_t GetSilDet_R()   {return silBox_rMax;};
        Double_t GetSilDet_L()   {return silBox_halfZ*2.;};
        
        std::map<int, std::string> GetSilNameIDMap() {return silNameIDMap;};
        ClassDef(a2mcApparatus,1) //a2mcApparatus
};

#endif //a2mc_APPARATUS_H
///< LEGEND/MAP OF THE SILICON DETECTOR
///< L = layer, M = module
///< ID = overall ID 
//    L   M   ID    NAME
//    0   0    0    0si0
//    0   1    1    0si1
//    0   2    2    0si2
//    0   3    3    0si3
//    0   4    4    0si4
//    0   5    5    0si5
//    0   6    6    0si6
//    0   7    7    0si7
//    0   8    8    0si8
//    0   9    9    0si9
//    1   0    10   1si0
//    1   1    11   1si1
//    1   2    12   1si2
//    1   3    13   1si3
//    1   4    14   1si4
//    1   5    15   1si5
//    1   6    16   1si6
//    1   7    17   1si7
//    1   8    18   1si8
//    1   9    19   1si9
//    1   10   20   1siA
//    1   11   21   1siB
//    2   0    22   2si0
//    2   1    23   2si1
//    2   2    24   2si2
//    2   3    25   2si3
//    2   4    26   2si4
//    2   5    27   2si5
//    2   6    28   2si6
//    2   7    29   2si7
//    2   8    30   2si8
//    2   9    31   2si9
//    2   10   32   2siA
//    2   11   33   2siB
//    2   12   34   2siC
//    2   13   35   2siD
//    3   0    36   3si0
//    3   1    37   3si1
//    3   2    38   3si2
//    3   3    39   3si3
//    3   4    40   3si4
//    3   5    41   3si5
//    3   6    42   3si6
//    3   7    43   3si7
//    3   8    44   3si8
//    3   9    45   3si9
//    4   0    46   4si0
//    4   1    47   4si1
//    4   2    48   4si2
//    4   3    49   4si3
//    4   4    50   4si4
//    4   5    51   4si5
//    4   6    52   4si6
//    4   7    53   4si7
//    4   8    54   4si8
//    4   9    55   4si9
//    4   10   56   4siA
//    4   11   57   4siB
//    5   0    58   5si0
//    5   1    59   5si1
//    5   2    60   5si2
//    5   3    61   5si3
//    5   4    62   5si4
//    5   5    63   5si5
//    5   6    64   5si6
//    5   7    65   5si7
//    5   8    66   5si8
//    5   9    67   5si9
//    5   10   68   5siA
//    5   11   69   5siB
//    5   12   70   5siC
//    5   13   71   5siD