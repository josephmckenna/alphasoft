// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "NeuralFinder.hh"

#include <iostream>

bool gVerbose = false;

NeuralFinder::NeuralFinder(TClonesArray* points):
   TracksFinder(points),
   lambda(5.),
   alpha(0.3),
   B(0.5),
   Temp(1.),
   c(10.),
   mu(2.),
   cosCut(0.9)
{
   // // No inherent reason why these parameters should be the same as in base class
   // fSeedRadCut = 150.;
   fPointsDistCut = 8.1;
   // fSmallRad = _cathradius;
   // fNpointsCut = 7;

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
            neurons.emplace_back(fPointsArray[startIdx],fPointsArray[endIdx]);
            outNeurons[fPointsArray[startIdx]].push_back(neurons.size()-1);
            inNeurons[fPointsArray[endIdx]].push_back(neurons.size()-1);
            nneurons++;
         }
      }
   }

   for(auto &n: neurons){
      CalcMatrixT(n);
   }

   return neurons.size();
};

//==============================================================================================
double NeuralFinder::MatrixT(const NeuralFinder::Neuron &n1, const NeuralFinder::Neuron &n2)
{
   double ptot2 = n1.Mag2()*n2.Mag2();
   if(ptot2 <= 0) return 0.;

   double cosine = n1.Dot(n2)/TMath::Sqrt(ptot2);
   if(cosine >  1.0){
      cout << "maxed cosine: " << cosine << endl;
      cosine =  1.0;
   }
   if(cosine < -1.0) cosine = -1.0;

   double dNorm = 30.;
   double d1 = n1.Mag()/dNorm;
   double d2 = n2.Mag()/dNorm;
   if(cosine > cosCut) return pow(cosine, lambda)/(pow(d1,mu)+pow(d2,mu));
   else return 0.;
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
      double T = MatrixT(n, neurons[i]);
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
      cout << "++++++++++++++++++++++++++++++++++" << endl;
      cout << "Neuron from " << n.GetStartPt()->GetX() << ", " << n.GetStartPt()->GetY() << ", " << n.GetStartPt()->GetZ() << endl;
      cout << "         to " << n.GetEndPt()->GetX() << ", " << n.GetEndPt()->GetY() << ", " << n.GetEndPt()->GetZ() << endl;
      cout << "sumTV = " << sumTV << endl;
      cout << "sumVIn = " << sumVIn << endl;
      cout << "sumVOut = " << sumVOut << endl;
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
      for(auto &con: outNeurons){
         double bestV = -9999.;
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
         cout << "Iteration " << i << ", change " << change << ", active neurons: " << CountActive() << endl;
         if(change <= itThres){
            converged = true;
            break;
         }
      }
      lastV = thisV;
   }
   trackID = 0;
   for(auto &con: outNeurons){
      bool oldTrack = inNeurons[con.first].size();
      if(oldTrack){
         oldTrack = false;
         for(int i: inNeurons[con.first]){
            if(neurons[i].GetActive()){
               oldTrack = true;
               break;
            }
         }
      }
      // if(!oldTrack){
      //    neurons[con.second[0]].GetStartPt()->SetTrackID(++trackID);
      //    cout << "New Track ID " << trackID << endl;
      //    con.first.Print();
      // }
   }
   // for(auto &n: neurons){
   //    if(n.GetActive())
   //       n.GetEndPt()->SetTrackID(n.GetStartPt()->GetTrackID());
   // }
   return converged;
};

//==============================================================================================
int NeuralFinder::RecTracks()
{
   int Npoints = fPointsArray.size();
   return fNtracks;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
