// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "NeuralFinder.hh"

#include <iostream>

bool gVerbose = false;

NeuralFinder::Neuron::Neuron(const vector<TSpacePoint*> &pts, int start, int end, const vector<double> &pointWeights): in(nullptr), out(nullptr), startPt(pts[start]), endPt(pts[end]), startIdx(start), endIdx(end)
{
   assert(pointWeights.size() == pts.size());
   // startPt = pts[start];
   // endPt = pts[end];
   SetX(endPt->GetX()-startPt->GetX());
   SetY(endPt->GetY()-startPt->GetY());
   SetZ(endPt->GetZ()-startPt->GetZ());
   SetPoint(0, startPt->GetX(), startPt->GetY(), startPt->GetZ());
   SetPoint(1, endPt->GetX(), endPt->GetY(), endPt->GetZ());
   assert(startPt->Distance(endPt) == Mag());
   assert(Mag() > 0.);
   // startIdx = start;
   // endIdx = end;

   weight = pointWeights[start]*pointWeights[end];
}

//==============================================================================================
NeuralFinder::NeuralFinder(TClonesArray* points):
   TracksFinder(points)
{
   // // No inherent reason why these parameters should be the same as in base class
   // fSeedRadCut = 150.;
   fPointsDistCut = 8.1;
   // fSmallRad = _cathradius;
   // fNpointsCut = 7;

   for(auto p: fPointsArray){
      pointWeights.push_back(pWeightScale/(p->GetErrX()*p->GetErrX()*p->GetErrY()*p->GetErrY()*p->GetErrZ()*p->GetErrZ()));
   }
   assert(pointWeights.size() == fPointsArray.size());
   // //  std::cout<<"NeuralFinder::NeuralFinder"<<std::endl;
   MakeNeurons();
}

//==============================================================================================
int NeuralFinder::MakeNeurons()
{
   nneurons = 0;
   for(unsigned int i = 0; i < fPointsArray.size(); i++){
      for(unsigned int j = i+1; j < fPointsArray.size(); j++){
         int startIdx(i), endIdx(j);
         if(fPointsArray[j]->Order(*fPointsArray[j],*fPointsArray[i])){
            startIdx = j;
            endIdx = i;
         }

         if(fPointsArray[startIdx]->Distance(fPointsArray[endIdx]) > fPointsDistCut){
            neurons.emplace_back(fPointsArray,startIdx,endIdx,pointWeights);
            outNeurons[fPointsArray[startIdx]].push_back(neurons.size()-1);
            inNeurons[fPointsArray[endIdx]].push_back(neurons.size()-1);
            nneurons++;
         }
      }
   }

   cout << "NeuralFinder::MakeNeurons: Number of Neurons: " << neurons.size() << endl;

   double maxLength = 0.;
   double maxTout = 0.;
   double meanTout = 0.;
   double maxTin = 0.;
   double meanTin = 0.;
   for(auto &n: neurons){
      if(n.Mag() > maxLength) maxLength = n.Mag();
      CalcMatrixT(n);
      if(n.GetTMat_out() > maxTout) maxTout = n.GetTMat_out();
      if(n.GetTMat_in() > maxTin) maxTin = n.GetTMat_in();
      meanTout += n.GetTMat_out();
      meanTin += n.GetTMat_in();
   }
   meanTout /= (double)neurons.size();
   meanTin /= (double)neurons.size();
   cout << "NeuralFinder::MakeNeurons: longest neuron: " << maxLength << endl;
   cout << "NeuralFinder::MakeNeurons: maximum TMat values: " << maxTin << '\t' << maxTout << endl;
   cout << "NeuralFinder::MakeNeurons: mean TMat values: " << meanTin << '\t' << meanTout << endl;

   return neurons.size();
};

//==============================================================================================
double NeuralFinder::MatrixT(const NeuralFinder::Neuron &n1, const NeuralFinder::Neuron &n2)
{
   double ptot2 = n1.Mag2()*n2.Mag2();
   if(ptot2 <= 0) return 0.;

   double cosine = n1.Dot(n2)/TMath::Sqrt(ptot2);
   if(cosine >  1.0){
      cout << "NeuralFinder::MatrixT: maxed cosine: " << cosine << endl;
      cosine =  1.0;
   }
   if(cosine < -1.0) cosine = -1.0;

   double dNorm = 10.;
   double d1 = n1.Mag()/dNorm;
   double d2 = n2.Mag()/dNorm;
   if(cosine > cosCut){
      double T = pow(cosine, lambda)/(pow(d1,mu)+pow(d2,mu)) * n1.GetWeight() * n2.GetWeight();
      // assert(T > 0.);
      // cout << "NeuralFinder::MatrixT: good cosine: " << cosine << ", T = " << T << endl;
      return T;
   } else {
      // cout << "NeuralFinder::MatrixT: low cosine: " << cosine << endl;
      return 0.;
   }


};

//==============================================================================================
void NeuralFinder::CalcMatrixT(NeuralFinder::Neuron &n)
{
   vector<int> &inputs = inNeurons[n.GetStartPt()];
   vector<int> &outputs = outNeurons[n.GetEndPt()];
   n.SetTMat_in(0.);
   for(auto i: inputs){
      double T = MatrixT(n, neurons[i]);
      if(T > n.GetTMat_in()){
         n.SetTMat_in(T);
         n.SetInput(&neurons[i]);
      }
   }
   n.SetTMat_out(0.);
   for(auto i: outputs){
      Neuron &n2 = neurons[i];
      double T = MatrixT(n, n2);
      if(T > n.GetTMat_out()){
         n.SetTMat_out(T);
         n.SetOutput(&neurons[i]);
      }
   }
}

//==============================================================================================
double NeuralFinder::CalcV(Neuron &n, double B, double T)
{

   double sumVOut(0.), sumVIn(0.), sumTV(0.);

   vector<int> &inputs = inNeurons[n.GetStartPt()];
   vector<int> &outputs = outNeurons[n.GetEndPt()];

   if(n.GetInput()) sumTV += n.GetTMat_in() * n.GetInput()->GetV();
   if(n.GetOutput()) sumTV += n.GetTMat_out() * n.GetOutput()->GetV();

   for(int i: outputs){
      Neuron &o = neurons[i];
      sumVOut += o.GetV();
   }
   for(int i: inputs) sumVIn += neurons[i].GetV();

   if(gVerbose){
      cout << "NeuralFinder::CalcV ++++++++++++++++++++++++++++++++++" << endl;
      cout << "NeuralFinder::CalcV: Neuron from " << n.GetStartPt()->GetX() << ", " << n.GetStartPt()->GetY() << ", " << n.GetStartPt()->GetZ() << endl;
      cout << "NeuralFinder::CalcV:          to " << n.GetEndPt()->GetX() << ", " << n.GetEndPt()->GetY() << ", " << n.GetEndPt()->GetZ() << endl;
      cout << "NeuralFinder::CalcV: sumTV = " << sumTV << endl;
      cout << "NeuralFinder::CalcV: sumVIn = " << sumVIn << endl;
      cout << "NeuralFinder::CalcV: sumVOut = " << sumVOut << endl;
   }
   double tanHArg = c/T*sumTV - alpha/T*(sumVIn + sumVOut) + B/T;

   n.SetV(0.5*(1 + tanh(tanHArg)));
   assert(n.GetV() >= 0 && n.GetV() <= 1);
   return n.GetV();
};

//==============================================================================================
int NeuralFinder::CountActive(){
   int na = 0;
   for(const auto &o: outNeurons){
      for(int i: o.second){
         na += neurons[i].GetActive();
      }
   }
   return na;
}

//==============================================================================================
bool NeuralFinder::Run()
{
   if(!GetNNeurons()) return false;
   vector<double> lastV;
   bool converged(false);
   for(int i = 0; i < maxIt; i++){
      vector<double> thisV;
      double bestV = -9999.;
      for(auto &con: outNeurons){
         for(int ii: con.second){
            thisV.push_back(CalcV(neurons[ii], B, Temp));
            if(thisV.back() > bestV) bestV = thisV.back();
         }
      }

      if(lastV.size() == thisV.size()){
         double change = 0.;
         for(unsigned int j = 0; j < lastV.size(); j++){
            change += fabs(lastV[j] - thisV[j]);
         }
         change /= double(thisV.size());
         cout << "NeuralFinder::CalcV: Iteration " << i << ", change " << change << ", active neurons: " << CountActive() << ", largest V = " << bestV << endl;
         if(change <= itThres){
            converged = true;
            break;
         }
      }
      lastV = thisV;
   }
   // for(auto &n: neurons){
   //    if(n.GetActive())
   //       n.GetEndPt()->SetTrackID(n.GetStartPt()->GetTrackID());
   // }
   return converged;
};

//==============================================================================================
int NeuralFinder::AssignTracks(){ // FIXME: This double-assigns points to multiple tracks
   fNtracks = 0;
   for(auto &con: outNeurons){
      for(int i: con.second){
         Neuron &n = neurons[i];

         if(n.GetActive() && n.GetSubID() < 0){
            set<int> trackPts = FollowTrack(n, fNtracks);
            fTrackVector.emplace_back(trackPts.begin(), trackPts.end());
            fNtracks++;
         }
      }
   }

   if( fNtracks != int(fTrackVector.size()) )
      std::cerr<<"NeuralFinder::RecTracks(): Number of found tracks "<<fNtracks
               <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;

   return fNtracks;
}

//==============================================================================================
set<int> NeuralFinder::FollowTrack(Neuron &n, int subID)
{                               // FIXME: somehow inNeurons[n.GetStartPt()] and outNeurons[n.GetEndPt()] are almost always empty
   set<int> pointset;
   if(n.GetActive()){
      if(n.GetSubID() < 0){
         n.SetSubID(subID);
         cout << "NeuralFinder::FollowTrack: subID " << subID << ", adding points " << n.GetStartIdx() << " and " << n.GetEndIdx() << endl;
         pointset.insert(n.GetStartIdx());
         pointset.insert(n.GetEndIdx());
         vector<int> &inn = inNeurons[n.GetStartPt()];
         for(int i: inn){
            Neuron &ni = neurons[i];
            set<int> inPts = FollowTrack(ni, subID);
            pointset.insert(inPts.begin(), inPts.end());
         }
         vector<int> &outn = outNeurons[n.GetEndPt()];
         for(int i: outn){
            Neuron &no = neurons[i];
            set<int> outPts = FollowTrack(no, subID);
            pointset.insert(outPts.begin(), outPts.end());
         }
      } else if(n.GetSubID() != subID){
         cout << "NeuralFinder::FollowTrack: Somehow hit a different subtrack, this shouldn't happen!" << endl;
      }
   }
   return pointset;
}

//==============================================================================================
int NeuralFinder::RecTracks()
{
   Run();
   AssignTracks();
   return fNtracks;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
