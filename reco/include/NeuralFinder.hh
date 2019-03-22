// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: L. Martin
// Date: Jan. 2019

#ifndef __NFINDER__
#define __NFINDER__ 1
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "TClonesArray.h"

#include "TVector3.h"
#include "TPolyLine3D.h"

using std::vector;
using std::map;
using std::set;
using std::cout;
using std::endl;

class NeuralFinder: public TracksFinder
{
public:
   NeuralFinder(TClonesArray*);
   ~NeuralFinder(){};

   virtual int RecTracks();
   inline int GetNNeurons() const { return nneurons; };
   int CountActive();

private:
   class Neuron: public TVector3, public TPolyLine3D
   {
   public:
      using TPolyLine3D::DrawClone;
      Neuron(): startPt(NULL), endPt(NULL), active(true), V(0.5){};

      Neuron(const vector<TSpacePoint*> &pts, int start, int end);

      inline bool SetActive(bool act=true){ active = act; return active; };
      inline void SetV(double v){ V = v; SetActive(V >= 0.5); };
      inline bool GetActive(){ return active; };
      inline double GetV(){ return V; };

      inline void SetInput(Neuron *inp){ in = inp; };
      inline void SetOutput(Neuron *outp){ out = outp; };
      inline void SetTMat_in(const double T){ TMat_in = T; };
      inline void SetTMat_out(const double T){ TMat_out = T; };

      inline Neuron *GetInput() const { return in; };
      inline Neuron *GetOutput() const { return out; };
      inline double GetTMat_in() const { return TMat_in; };
      inline double GetTMat_out() const { return TMat_out; };

      inline TSpacePoint *GetStartPt(){ return startPt; };
      inline TSpacePoint *GetEndPt(){ return endPt; };

      inline int GetStartIdx(){ return startIdx; };
      inline int GetEndIdx(){ return endIdx; };

   private:
      TSpacePoint *startPt, *endPt;
      int startIdx, endIdx;
      bool active;
      double V;

      Neuron *in, *out;
      double TMat_in = 0.;
      double TMat_out = 0.;

   };

private:
   int MakeNeurons();
   bool Run();

   double MatrixT(const Neuron &n1, const Neuron &n2);
   void  CalcMatrixT(Neuron &n);
   double CalcV(Neuron &n, double B, double T);

   void FillTrack(track_t &track, const vector<int> &nidx);
   int AssignTracks();

   double lambda = 5.;
   double alpha = 5.;
   double B = 0.2;
   double Temp = 1.;
   double c = 10.;
   double mu = 2.;
   double cosCut = 0.9;
   double distCut = 100.;

   int maxIt = 10;
   double itThres = 0.00005;    // threshold defining convergence

   int nneurons;

   vector<Neuron> neurons;

   struct cmp {
      bool operator() (const TSpacePoint *a, const TSpacePoint *b) const {
         return a->Order(*a, *b);
      }
   };

   map<TSpacePoint*,vector<int>, cmp > outNeurons;
   map<TSpacePoint*,vector<int>, cmp > inNeurons;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
