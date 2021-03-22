#ifndef a2mc_Settings_H
#define a2mc_Settings_H

#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TObject.h>

class a2mcSettings
{
public:
    a2mcSettings();
    virtual ~a2mcSettings();
    void init(std::string);

    ///< Get functions
    Bool_t  isValid()           const {return status;       };
    std::string GetIniFile()    const {return ini_file;     };
    UInt_t  GetGenType()        const {return gen_type;     };
    UInt_t  GetGenMode()        const {return gen_mode;     };
    UInt_t  GetInnEnviro()      const {return inn_enviro;   };
    UInt_t  GetSilDet()         const {return sil_det;      };
    UInt_t  GetOutEnviro()      const {return out_enviro;   };
    UInt_t  GetMagField()       const {return mag_field;    };
    Bool_t  GetStoreTracks()    const {return store_tracks; };
    Int_t   GetTracksLim()      const {return tracks_lim;   };
    UInt_t  GetVerbose()        const {return verbose;      };
    void Print();
private:
    Bool_t status;
    std::string ini_file;
    UInt_t gen_type;
    UInt_t gen_mode;
    UInt_t inn_enviro;
    UInt_t sil_det;
    UInt_t out_enviro;
    UInt_t mag_field;
    Bool_t store_tracks;
    Int_t  tracks_lim;
    UInt_t verbose;
};

#endif //a2mc_Settings_H
