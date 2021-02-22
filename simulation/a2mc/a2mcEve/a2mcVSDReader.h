#ifndef a2mcVSDReader_h
#define a2mcVSDReader_h

class a2mcVSDReader {
public:
    // ----------------------------------------------------------
    // File / Event Data
    // ----------------------------------------------------------
    TFile      *fFile;
    TDirectory *fDirectory;
    TObjArray  *fEvDirKeys;
    TEveVSD    *fVSD;
    Int_t       fMaxEv, fCurEv;
    Int_t       fEventMC;
    // ----------------------------------------------------------
    // Event visualization structures
    // ----------------------------------------------------------
    TEveTrackList   *fTrackListMC;
    TEveTrackList   *fTrackListRec;
    TEvePointSet    *fSilHits       = nullptr;
    TEvePointSet    *fPrimOriginHit = nullptr;
    TEvePointSet    *fPrimDecayHit  = nullptr;

public:
a2mcVSDReader(const char* file_name) :
    fFile(0),     
    fDirectory(0), 
    fEvDirKeys(0),
    fVSD(0), 
    fMaxEv(-1), 
    fCurEv(-1),
    fTrackListMC(0),
    fTrackListRec(0),
    fSilHits(0)
{
  // Constructor
    fFile = TFile::Open(file_name);
    if (!fFile) {
        Error("VSD_Reader", "Can not open file '%s' ... terminating.", file_name);
        gSystem->Exit(1);
    }
    fEvDirKeys = new TObjArray;
    TPMERegexp name_re("Event\\d+");
    TObjLink* lnk = fFile->GetListOfKeys()->FirstLink();
    while (lnk) {
        if (name_re.Match(lnk->GetObject()->GetName())) {
            fEvDirKeys->Add(lnk->GetObject());
        }
        lnk = lnk->Next();
    }

    fMaxEv = fEvDirKeys->GetEntriesFast();
    if (fMaxEv == 0) {
        Error("VSD_Reader", "No events to show ... terminating.");
        gSystem->Exit(1);
    }
    fVSD = new TEveVSD;
}

virtual ~a2mcVSDReader() {
  // Destructor.

   DropEvent();

   delete fVSD;
   delete fEvDirKeys;

   fFile->Close();
   delete fFile;
}

///< Hits/tracks loaders
void LoadSilHits(TEvePointSet*&, const TString&);
void LoadPrimaryOriginHit(TEvePointSet*&, const TString&);
void LoadPrimaryDecayHit(TEvePointSet*&, const TString&);
void LoadMCTracks();
void LoadRecTracks();
//void LoadClusters(TEvePointSet*&, const TString&, Int_t);
void AttachEvent();
void DropEvent();
//---------------------------------------------------------------------------
// Event navigation (NextEvent, PrevEvent, GotoEvent)
//---------------------------------------------------------------------------
void NextEvent();
void PrevEvent();
void SetCurEv(Int_t i) { fCurEv = i;};
Bool_t GotoEvent(Int_t ev);
Bool_t GotoEvent();

enum ESDTrackFlags {
    kITSin=0x0001,kITSout=0x0002,kITSrefit=0x0004,kITSpid=0x0008,
    kTPCin=0x0010,kTPCout=0x0020,kTPCrefit=0x0040,kTPCpid=0x0080,
    kTRDin=0x0100,kTRDout=0x0200,kTRDrefit=0x0400,kTRDpid=0x0800,
    kTOFin=0x1000,kTOFout=0x2000,kTOFrefit=0x4000,kTOFpid=0x8000,
    kHMPIDpid=0x20000,
    kEMCALmatch=0x40000,
    kTRDbackup=0x80000,
    kTRDStop=0x20000000,
    kESDpid=0x40000000,
    kTIME=0x80000000
};

Bool_t trackIsOn(TEveTrack* t, Int_t mask) {
  // Check is track-flag specified by mask are set.
    return (t->GetStatus() & mask) > 0;
}

Int_t DirNameToEventN(string& s) {
    string sn = s.substr(5,5); ///< Dir name is, e.g., "Event0005"
    return atoi(sn.c_str());
}

///< Dump (on screen) utilities
void DumpEvent();
void DumpSilHits();
void DumpSilHit();
    
void DumpMCTracks();
void DumpMCTrack();
    
    ClassDef(a2mcVSDReader, 0);
};

#endif // #ifdef a2mcVSDReader_h
