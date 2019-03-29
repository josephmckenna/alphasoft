// Neural Network Tracks finder class definition
// for ALPHA-g TPC analysis
// based on ALEPH NN (see DOI 10.1016/0010-4655(91)90048-P)
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
   inline vector<double> GetPointWeights() const { return pointWeights; };
   inline vector<double> GetInNeuronWeights() const { return inNeuronWeights; };
   inline vector<double> GetOutNeuronWeights() const { return outNeuronWeights; };
   inline vector<double> GetNeuronV() const { return neuronV; };
   int ApplyThreshold(double thres);
   int MakeNeurons();

   class Neuron: public TVector3, public TPolyLine3D
   {
   public:
      using TPolyLine3D::DrawClone;

      Neuron(const vector<TSpacePoint*> &pts, int start, int end, const vector<double> &pointWeights);

      inline bool SetActive(bool act=true){ active = act; return active; };
      inline void SetV(double v){ V = v; };
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

      inline const TSpacePoint *GetStartPt() const { return startPt; };
      inline const TSpacePoint *GetEndPt() const { return endPt; };

      inline int GetStartIdx() const{ return startIdx; };
      inline int GetEndIdx() const{ return endIdx; };

      inline double GetWeight() const{ return weight; };

      inline void SetSubID(int id){ subTrackID = id; };
      inline int GetSubID() const{ return subTrackID; };

   private:
      Neuron *in;
      Neuron *out;

      const TSpacePoint *startPt;
      const TSpacePoint *endPt;
      const int startIdx, endIdx;
      bool active = false;
      double V = 0.5;

      double TMat_in = 0.;
      double TMat_out = 0.;

      double weight = 0.;
      int subTrackID = -1.;
   };

   const set<Neuron*> GetTrackNeurons(int trackID);

private:
   // int MakeNeurons();
   bool Run();

   double MatrixT(const Neuron &n1, const Neuron &n2);
   void  CalcMatrixT(Neuron &n);
   double CalcV(Neuron &n, double B, double T);

   int AssignTracks();
   set<int> FollowTrack(Neuron &n, int subID);

   // V_kl = 0.5 * [1 + tanh(c/Temp \sum(T_kln*V_ln) - alpha/Temp{\sum(V_kn) + \sum(V_ml)} + B/Temp)]
   // NN parameters             // ALEPH values (see DOI 10.1016/0010-4655(91)90048-P)
   double lambda = 5.;          // 5.
   double alpha = 5.;           // 5.
   double B = 0.2;              // 0.2
   double Temp = 10.;           // 1.
   double c = 10.;              // 10.
   double mu = 1.;              // 2.
   double cosCut = 0.9;         // 0.9    // larger kinks between neurons set T value to zero
   double VThres = 0.6;         // 0.9    // V value above which a neuron is considered active

   double dNormXY = 10.;        // normalization for XY distance and
   double dNormZ = 2.;          // Z distance, different to weight the influence of gaps differently
                                // no good reason for these values

   double Tscale = 0.4;         // fudge factor to bring T values into range [0,1], probably has to be changed with other parameters...

   int maxIt = 20;
   double itThres = 0.0005;     // threshold defining convergence
   double pWeightScale = 0.1;   // scale point weights to achieve something ~ 0..1

   vector<double> pointWeights, inNeuronWeights, outNeuronWeights, neuronV;

   int nneurons;

   vector<Neuron> neurons;

   struct cmp {
      bool operator() (const TSpacePoint *a, const TSpacePoint *b) const {
         return a->Order(*a, *b);
      }
   };

   map<const TSpacePoint*,vector<int>, cmp > outNeurons;
   map<const TSpacePoint*,vector<int>, cmp > inNeurons;

};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
