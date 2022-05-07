// Vertex class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: May 2014

#ifndef __TFITVERTEX__
#define __TFITVERTEX__ 1

#include <TObject.h>
#include <TVector3.h>
#include <vector>

#include "TFitHelix.hh"

class TFitVertex : public TObject
{
private:
  int fID;
  std::vector<TFitHelix*> fHelixArray;
  int fNhelices;

  double fchi2;     // vertex chi^2
  TVector3 fVertex; // vertex position
  TVector3 fVertexError2;

  int fNumberOfUsedHelices;
  std::vector<TFitHelix*> fHelixStack;

  double fChi2Cut;

  TFitHelix* fInit0; // seed helix
  int fSeed0Index;
  double fSeed0Par; // arc length param.
  TFitHelix* fInit1;
  int fSeed1Index;
  double fSeed1Par; // arc length param.

  double fSeedchi2; // stage 1 chi2
  TVector3 fMeanVertex; // stage 1 position
  TVector3 fMeanVertexError2;

  double fNewChi2; // stage 2 chi2
  TVector3 fNewVertex; // stage 2 vertex
  TVector3 fNewVertexError2;  

  double fNewSeed0Par; // arc length param. helix new vertex
  double fNewSeed1Par;

  // stage 1
  int FirstPass();
  double FindSeed(  double trapradius2 = ALPHAg::_trapradius*ALPHAg::_trapradius );
  double FindMinDistance(double& s0, double& s1);
  double FindMinDistanceM2(double& s0, double& s1);
  TVector3 EvaluateMeanPoint();
  TVector3 EvaluateMeanPoint(TVector3 p0, TVector3 e0, 
			     TVector3 p1, TVector3 e1);
  TVector3 EvaluateMeanPointError2();
  
  // stage 2
  void SecondPass();
  double Recalculate();
  double RecalculateM2();

  // stage 3
  int ThirdPass();
  int Improve();
  double FindNewVertex(double* p, double* e);
  double FindNewVertexM2(double* p, double* e);
  

  void AssignHelixStatus();

  void CompressHelixStack()
  {
    std::vector<TFitHelix*> temp;
    temp.reserve(fHelixStack.size());
    for (TFitHelix* h: fHelixStack)
    {
      if (h)
        temp.push_back(h);
    }
    fHelixStack = std::move(temp);
  }

public:
  TFitVertex();
  TFitVertex(int id);
  ~TFitVertex();

  int AddHelix(TFitHelix*);
  inline const std::vector<TFitHelix*>* GetHelixArray()  {return &fHelixArray;}
  inline int GetNumberOfAddedHelix() const {return fNhelices;}

  void SetID(int id) { fID = id; }

  inline void SetChi2Cut(double cut) {fChi2Cut=cut;}
  inline double GetChi2Cut() {return fChi2Cut;}

  // main function to reconstruct the vertex
  int Calculate(const int thread_no = 1, const int thread_count = 1);

  inline TFitHelix* GetInit0() const {return fInit0;}
  inline TFitHelix* GetInit1() const {return fInit1;}

  inline const std::vector<TFitHelix*>* GetHelixStack() const {return &fHelixStack;}
  inline int GetNumberOfHelices() const         {return fNumberOfUsedHelices;}

  // inline double GetSeedHel0PDG() const {return ((TFitHelix*)(fHelixArray.At(fSeed0Index)))->GetParticleType();}
  // inline double GetSeedHel1PDG() const {return ((TFitHelix*)(fHelixArray.At(fSeed1Index)))->GetParticleType();}

  int FindDCA();

  inline double GetRadius()    const {return fVertex.Perp();}
  inline double GetAzimuth()   const {return fVertex.Phi();}
  inline double GetElevation() const {return fVertex.Z();}

  inline double GetChi2()     const {return fchi2;}
  inline double GetSeedChi2() const {return fSeedchi2;}
  inline double GetNewChi2()  const {return fNewChi2;}

  inline const TVector3* GetVertex()           const {return &fVertex;}
  inline const TVector3* GetVertexError2()     const {return &fVertexError2;}    
  inline const TVector3* GetNewVertex()        const {return &fNewVertex;}
  inline const TVector3* GetNewVertexError2()  const {return &fNewVertexError2;}
  inline const TVector3* GetMeanVertex()       const {return &fMeanVertex;}
  inline const TVector3* GetMeanVertexError2() const {return &fMeanVertexError2;}

  static bool InRadiusRange(double r);

  virtual void Print(Option_t *option="rphi") const;
  //virtual void Draw(Option_t *option="");
  virtual void Clear(Option_t *option="");
  virtual void Reset();
  //inline TPolyMarker3D* GetVertexPoint() const {return fPoint;}

  ClassDef(TFitVertex,1)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
