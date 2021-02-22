#ifndef a2mc_MC_H
#define a2mc_MC_H

#include <TROOT.h>
#include <TSystem.h>
#include "TMCVerbose.h"

#include "a2mcSilSD.h"

#include "a2mcApparatus.h"
#include "a2mcFieldConstant.h"
#include "a2mcFieldFromMap.h"
#include "a2mcStack.h"
#include "a2mcPrimary.h"
#include "a2mcGenerator.h"
#include "a2mcRootManager.h"
#include "a2mcMessenger.h"

class a2mcStack;
class a2mcApparatus;
class a2mcGenerator;

using namespace std;

class a2mcVirtualMC : public TVirtualMCApplication
{
	public:
		a2mcVirtualMC(const Char_t* name,  const Char_t *title, Int_t, string&, Int_t);
		a2mcVirtualMC();
		virtual ~a2mcVirtualMC();

		// static access method
		static a2mcVirtualMC* Instance() {
            return (a2mcVirtualMC*)(TVirtualMCApplication::Instance()); 
        }

		// methods
		void InitMC(const char *setup);
		void RunMC(Int_t nofEvents);
		int  GetnEvents() {return nEvents;}
		void FinishRun();	

		virtual void ConstructGeometry();
		virtual void InitGeometry();
        virtual void AddParticles();
		virtual void GeneratePrimaries();
		virtual void BeginEvent();
		virtual void BeginPrimary();
		virtual void PreTrack();
		virtual void Stepping();
		virtual void PostTrack();
		virtual void FinishPrimary();
		virtual void FinishEvent();

		// virtual  void  ResetHits();

		// set methods
		void  SetVerboseLevel(Int_t verboseLevel);    

		// get methods
		a2mcGenerator*  GetPrimaryGenerator() const;

	private:

		void RegisterStack(); 
        void WriteLog();

		// data members

		Bool_t verbose; // = 0 no verbose, = 1 verbose
        Int_t runNumber, runSeed;
        string runTime;
        a2mcSettings a2mcConf{}; ///< Reading configuration file a2MC.ini
        a2mcMessenger mess;
        Int_t                 nEvents;            ///< Event counter
		a2mcStack*            fStack;             ///< VMC stack
		a2mcApparatus*        fDetConstruction;   ///< a2mc Detector construction    
        a2mcPrimary*          fPrimary;           ///< Primary  Object
		a2mcSilSD             fSilSD;             ///< Frame SD
		TMCVerbose            fVerbose;           ///< VMC verbose helper
		a2mcGenerator*        fPrimaryGenerator;  ///< Primary generator
		TVirtualMagField*     fMagField;          ///< Magnetic field
		a2mcRootManager*      fRootManager;       ///< Root manager
		ClassDef(a2mcVirtualMC,1)  //Interface to MonteCarlo application
};

// inline functions

/// \return The singleton instance 
//inline a2mcVirtualMC* a2mcVirtualMC::Instance()
//{ return (a2mcVirtualMC*)(TVirtualMCApplication::Instance()); }

/// Set verbosity 
/// \param verboseLevel  The new verbose level value
inline void  a2mcVirtualMC::SetVerboseLevel(Int_t verboseLevel)
{ fVerbose.SetLevel(verboseLevel); }

/// \return The primary generator
inline a2mcGenerator*  a2mcVirtualMC::GetPrimaryGenerator() const
{ return fPrimaryGenerator; }

#endif //a2mc_MC_H

