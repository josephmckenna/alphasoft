// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "NeuralFinder.hh"

#include <iostream>

bool gVerbose = false;

NeuralFinder::Neuron::Neuron(const vector<TSpacePoint*> &pts, int start, int end, const vector<double> &pointWeights): in(nullptr), out(nullptr), startIdx(start), endIdx(end), startPt(pts[start]), endPt(pts[end])
{
   // assert(pointWeights.size() == pts.size());
   SetX(endPt->GetX()-startPt->GetX());
   SetY(endPt->GetY()-startPt->GetY());
   SetZ(endPt->GetZ()-startPt->GetZ());
   SetPoint(0, startPt->GetX(), startPt->GetY(), startPt->GetZ());
   SetPoint(1, endPt->GetX(), endPt->GetY(), endPt->GetZ());
   // assert(startPt->Distance(endPt) == Mag());
   // assert(Mag() > 0.);

   weight = 20.*pointWeights[start]*pointWeights[end];
}

//==============================================================================================
NeuralFinder::NeuralFinder(TClonesArray* points):
   TracksFinder(points)
{
   // // No inherent reason why these parameters should be the same as in base class
   // fSeedRadCut = 150.;
   // fPointsDistCut = 20.;
   // fSmallRad = _cathradius;
   // fNpointsCut = 7;

   for(auto p: fPointsArray){
      pointWeights.push_back(pWeightScale/(p->GetErrX()*p->GetErrX()*p->GetErrY()*p->GetErrY()*p->GetErrZ()*p->GetErrZ()));
   }
   // assert(pointWeights.size() == fPointsArray.size());
   // //  std::cout<<"NeuralFinder::NeuralFinder"<<std::endl;
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

         TSpacePoint *startp = fPointsArray[startIdx];
         TSpacePoint *endp = fPointsArray[endIdx];
         if(startp->Distance(endp) < fPointsDistCut){
            neurons.emplace_back(fPointsArray,startIdx,endIdx,pointWeights);
            outNeurons[startp].push_back(nneurons);
            inNeurons[endp].push_back(nneurons);
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
      inNeuronWeights.push_back(n.GetTMat_in());
      outNeuronWeights.push_back(n.GetTMat_out());
   }
   meanTout /= (double)neurons.size();
   meanTin /= (double)neurons.size();
   cout << "NeuralFinder::MakeNeurons: longest neuron: " << maxLength << endl;
   cout << "NeuralFinder::MakeNeurons: maximum TMat values: " << maxTin << '\t' << maxTout << endl;
   cout << "NeuralFinder::MakeNeurons: mean TMat values: " << meanTin << '\t' << meanTout << endl;

   // for(auto on: outNeurons){
   //    for(int i: on.second){
   //       assert(neurons[i].GetStartPt() == on.first);
   //    }
   // }
   // for(auto in: inNeurons){
   //    for(int i: in.second){
   //       assert(neurons[i].GetEndPt() == in.first);
   //    }
   // }
   return neurons.size();
};

//==============================================================================================
map<int,vector<int> > NeuralFinder::GetEndNeurons()
{
   map<int,vector<int> > en;
   for(auto p: inNeurons){
      bool atEnd = (outNeurons.count(p.first) == 0);
      if(!atEnd){
         atEnd = true;
         for(int i: outNeurons[p.first]){
            if(neurons[i].GetActive()){
               atEnd = false;
               break;
            }
         }
      }
      if(atEnd){
         for(int ni: p.second){
            if(neurons[ni].GetActive()){
               en[neurons[ni].GetSubID()].push_back(ni);
            }
         }
      }
   }
   return en;
}
//==============================================================================================
map<int,vector<int> > NeuralFinder::GetStartNeurons()
{
   map<int,vector<int> > sn;
   for(auto p: outNeurons){
      bool atStart = (inNeurons.count(p.first) == 0);
      if(!atStart){
         atStart = true;
         for(int i: inNeurons[p.first]){
            if(neurons[i].GetActive()){
               atStart = false;
               break;
            }
         }
      }
      if(atStart){
         for(int ni: p.second){
            if(neurons[ni].GetActive()){
               sn[neurons[ni].GetSubID()].push_back(ni);
            }
         }
      }
   }
   return sn;
}

//==============================================================================================
int NeuralFinder::MakeMetaNeurons()
{
   // map<const TSpacePoint*,vector<int>, cmp > &inMeta = inNeurons;
   // map<const TSpacePoint*,vector<int>, cmp > &outMeta = outNeurons;
   // vector<Neuron> &metaNeurons = neurons;
   inMeta.clear();
   outMeta.clear();
   metaNeurons.clear();
   metaPoints.clear();
   map<int,vector<int> > inMetaIdx, outMetaIdx;
   for(auto sn: GetStartNeurons()){
      if(sn.second.size()){
         metaPoints.emplace_back(0.,0.,0.,kUnknown,kUnknown,kUnknown);
         TSpacePoint &pavg = metaPoints.back();
         metaNeurons.emplace_back(Neuron());
         // outMeta[&pavg].push_back(metaNeurons.size()-1);
         outMetaIdx[metaPoints.size()-1].push_back(metaNeurons.size()-1);
         Neuron &navg = metaNeurons.back();
         for(int i: sn.second){
            Neuron &n = neurons[i];
            navg += n;
            pavg.SetX(pavg.GetX()+n.GetStartPt()->GetX());
            pavg.SetY(pavg.GetY()+n.GetStartPt()->GetY());
            pavg.SetZ(pavg.GetZ()+n.GetStartPt()->GetZ());
         }
         navg *= 1./double(sn.second.size());
         pavg.SetX(pavg.GetX()/double(sn.second.size()));
         pavg.SetY(pavg.GetY()/double(sn.second.size()));
         pavg.SetZ(pavg.GetZ()/double(sn.second.size()));

         navg.SetSubID(sn.first);
         navg.SetStartPt(&pavg);
         cout << "******** Neuron: " << navg.X() << '\t' << navg.Y() << '\t' << navg.Z() << endl;
      }
   }
   for(auto en: GetEndNeurons()){
      if(en.second.size()){
         metaPoints.emplace_back(0.,0.,0.,kUnknown,kUnknown,kUnknown);
         TSpacePoint &pavg = metaPoints.back();
         metaNeurons.emplace_back(Neuron());
         // inMeta[&pavg].push_back(metaNeurons.size()-1);
         inMetaIdx[metaPoints.size()-1].push_back(metaNeurons.size()-1);
         Neuron &navg = metaNeurons.back();
         for(int i: en.second){
            Neuron &n = neurons[i];
            navg += n;
            pavg.SetX(pavg.GetX()+n.GetEndPt()->GetX());
            pavg.SetY(pavg.GetY()+n.GetEndPt()->GetY());
            pavg.SetZ(pavg.GetZ()+n.GetEndPt()->GetZ());
         }
         navg *= 1./double(en.second.size());
         pavg.SetX(pavg.GetX()/double(en.second.size()));
         pavg.SetY(pavg.GetY()/double(en.second.size()));
         pavg.SetZ(pavg.GetZ()/double(en.second.size()));

         navg.SetSubID(en.first);
         navg.SetEndPt(&pavg);
         cout << "******** Neuron: " << navg.X() << '\t' << navg.Y() << '\t' << navg.Z() << endl;
      }
   }

   for(auto i: inMetaIdx) inMeta[&metaPoints[i.first]] = i.second;
   for(auto i: outMetaIdx) outMeta[&metaPoints[i.first]] = i.second;

   map<const TSpacePoint*,vector<int>, cmp > newOut, newIn;
   for(auto in: inMeta){
      const TSpacePoint *ps = in.first;
      cout << "********* Start: ************ " << ps->ClassName() << endl;
      ps->TSpacePoint::Print("rphi");
      if(in.second.size()){
         int startID = metaNeurons[in.second.front()].GetSubID();
         for(auto o: outMeta){
            const TSpacePoint *pe = o.first;
            cout << "******** End ***********" << pe->ClassName() << endl;
            if(o.second.size()){
               int endID = metaNeurons[o.second.front()].GetSubID();
               if(endID != startID){
                  if(ps->Distance(pe) < 50.){
                     metaNeurons.emplace_back(Neuron());
                     pe->TSpacePoint::Print("rphi");
                     metaNeurons.back().SetEndPt(pe);
                     metaNeurons.back().SetStartPt(ps);
                     metaNeurons.back().SetXYZ(pe->GetX()-ps->GetX(), pe->GetY()-ps->GetY(), pe->GetZ()-ps->GetZ());
                     metaNeurons.back().SetSubID(startID);
                     cout << "******** Vector: " << metaNeurons.back().X() << '\t' << metaNeurons.back().Y() << '\t' << metaNeurons.back().Z() << endl;
                     newIn[pe].push_back(metaNeurons.size()-1);
                     newOut[ps].push_back(metaNeurons.size()-1);
                  } else {
                     cout << "Points are too far apart: " << ps->Distance(pe) << endl;
                  }
               } else {
                  cout << "Points " << ps << " and " << pe << " already belong to the same track " << startID << endl;
               }
            }
         }
      }
   }
   outMeta.insert(newOut.begin(), newOut.end());
   inMeta.insert(newIn.begin(), newIn.end());

   for(Neuron &n: metaNeurons){
      CalcMatrixT_meta(n);
   }
   return metaNeurons.size();
}
//==============================================================================================
const set<NeuralFinder::Neuron*> NeuralFinder::GetTrackNeurons(int trackID)
{
   set<NeuralFinder::Neuron*> nset;
   if(trackID >= 0 && trackID < (int)fTrackVector.size()){
      const track_t &t = fTrackVector[trackID];
      for(int i: t){
         TSpacePoint *p = fPointsArray[i];
         for(int j: inNeurons[p]){
            nset.insert(&neurons[j]);
         }
         for(int j: outNeurons[p]){
            nset.insert(&neurons[j]);
         }
      }
   } else {
      for(auto &n: neurons) nset.insert(&n);
   }
   return nset;
};
//==============================================================================================
const set<NeuralFinder::Neuron*> NeuralFinder::GetMetaNeurons()
{
   set<NeuralFinder::Neuron*> nset;
   for(auto &n: metaNeurons) nset.insert(&n);
   return nset;
};
//==============================================================================================
double NeuralFinder::MatrixT(const NeuralFinder::Neuron &n1, const NeuralFinder::Neuron &n2)
{
   double ptot2 = n1.Mag2()*n2.Mag2();
   if(ptot2 <= 0){
      return 0.;
   }

   double cosine = n1.Dot(n2)/TMath::Sqrt(ptot2);

   // floating point math will sometimes produce slightly above 1, which is trouble once the inverse function is called
   if(cosine >  1.0) cosine =  1.0;
   if(cosine < -1.0) cosine = -1.0;

   // cout << "++++ MatrixT: cosine = " << cosine << endl;
   // cout << n1.X() << '\t' << n1.Y() << '\t' << n1.Z() << endl;
   // cout << n2.X() << '\t' << n2.Y() << '\t' << n2.Z() << endl;
   // double d1 = n1.Mag()/dNorm;
   // double d2 = n2.Mag()/dNorm;
   double dXY1 = sqrt(n1.X()*n1.X()+n1.Y()*n1.Y())/dNormXY;
   double dXY2 = sqrt(n2.X()*n2.X()+n2.Y()*n2.Y())/dNormXY;
   double d1 = sqrt(dXY1*dXY1 + n1.Z()/dNormZ*n1.Z()/dNormZ);
   double d2 = sqrt(dXY2*dXY2 + n2.Z()/dNormZ*n2.Z()/dNormZ);
   if(cosine > cosCut){
      // 0.2 factor is made up, no good reason, just puts T roughly in [0,1]
      double T = Tscale*pow(cosine, lambda)/(pow(d1,mu)+pow(d2,mu)) * n1.GetWeight() * n2.GetWeight();
      // cout << "d1 = " << d1 << ", d2 = " << d2 << ", T = " << T << endl;
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
void NeuralFinder::CalcMatrixT_meta(NeuralFinder::Neuron &n)
{
   vector<int> *inputs = nullptr;
   if(n.GetStartPt()) inputs = &inMeta[n.GetStartPt()];
   vector<int> *outputs = nullptr;
   if(n.GetEndPt()) outputs = &outMeta[n.GetEndPt()];
   n.SetTMat_in(0.);
   if(inputs){
      for(auto i: *inputs){
         double T = 10.*MatrixT(n, metaNeurons[i]); // FIXME: T values are too small to work without upscale
         if(T > n.GetTMat_in()){
            n.SetTMat_in(T);
            n.SetInput(&metaNeurons[i]);
         }
      }
   }
   n.SetTMat_out(0.);
   if(outputs){
      for(auto i: *outputs){
         Neuron &n2 = metaNeurons[i];
         double T = 10.*MatrixT(n, n2); // FIXME: T values are too small to work without upscale
         if(T > n.GetTMat_out()){
            n.SetTMat_out(T);
            n.SetOutput(&metaNeurons[i]);
         }
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
   // cout << "XXXXXXXXXX " << n.GetV() << '\t' << c/T*sumTV << '\t' << alpha/T*sumVIn << '\t' << alpha/T*sumVOut << '\t' << B/T << endl;
   n.SetActive(n.GetV() >= VThres);
   assert(n.GetV() >= 0 && n.GetV() <= 1);
   return n.GetV();
};

//==============================================================================================
double NeuralFinder::CalcV_meta(Neuron &n, double B, double T)
{

   double sumVOut(0.), sumVIn(0.), sumTV(0.);

   vector<int> *inputs = nullptr;
   if(n.GetStartPt()) inputs = &inMeta[n.GetStartPt()];
   vector<int> *outputs = nullptr;
   if(n.GetEndPt()) outputs = &outMeta[n.GetEndPt()];

   if(n.GetInput()) sumTV += n.GetTMat_in() * n.GetInput()->GetV();
   if(n.GetOutput()) sumTV += n.GetTMat_out() * n.GetOutput()->GetV();

   if(outputs){
      for(int i: *outputs){
         Neuron &o = neurons[i];
         sumVOut += o.GetV();
      }
   }
   if(inputs){
      for(int i: *inputs) sumVIn += neurons[i].GetV();
   }
   if(true){
      cout << "NeuralFinder::CalcV_meta ++++++++++++++++++++++++++++++++++" << endl;
      cout << "NeuralFinder::CalcV_meta: sumTV = " << sumTV << endl;
      cout << "NeuralFinder::CalcV_meta: sumVIn = " << sumVIn << endl;
      cout << "NeuralFinder::CalcV_meta: sumVOut = " << sumVOut << endl;
   }
   double tanHArg = c/T*sumTV - alpha/T*(sumVIn + sumVOut) + B/T;

   n.SetV(0.5*(1 + tanh(tanHArg)));
   // cout << "XXXXXXXXXX " << n.GetV() << '\t' << c/T*sumTV << '\t' << alpha/T*sumVIn << '\t' << alpha/T*sumVOut << '\t' << B/T << endl;
   n.SetActive(n.GetV() >= VThres);
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
      double T = i<maxIt/2 ? Temp : 0.5*Temp;
      double BB = i<maxIt/2 ? B : 0.5*B;
      for(auto &con: outNeurons){
         for(int ii: con.second){
            thisV.push_back(CalcV(neurons[ii], BB, T));
            if(thisV.back() > bestV) bestV = thisV.back();
         }
      }

      if(lastV.size() == thisV.size()){
         double change = 0.;
         for(unsigned int j = 0; j < lastV.size(); j++){
            change += fabs(lastV[j] - thisV[j]);
         }
         change /= double(thisV.size());
         cout << "NeuralFinder::CalcV: Iteration " << i << ", Temp " << T << ", change " << change << ", active neurons: " << CountActive() << ", largest V = " << bestV << endl;
         if(change <= itThres){
            converged = true;
            break;
         }
      }
      lastV = thisV;
   }

   neuronV.clear();
   for(auto &n: neurons) neuronV.push_back(n.GetV());
   assert(int(neuronV.size()) == nneurons);
   return converged;
};

//==============================================================================================
bool NeuralFinder::RunMeta()
{
   int n = MakeMetaNeurons();
   cout << "NeuralFinder: generated " << n << " \"MetaNeurons\"" << endl;
   if(!n) return false;
   vector<double> lastV;
   bool converged(false);
   for(int i = 0; i < maxIt; i++){
      vector<double> thisV;
      double bestV = -9999.;
      double T = i<maxIt/2 ? Temp : 0.5*Temp;
      double BB = i<maxIt/2 ? B : 0.5*B;
      for(auto &n: metaNeurons){
         thisV.push_back(CalcV_meta(n, BB, T));
         if(thisV.back() > bestV) bestV = thisV.back();
      }

      if(lastV.size() == thisV.size()){
         double change = 0.;
         for(unsigned int j = 0; j < lastV.size(); j++){
            change += fabs(lastV[j] - thisV[j]);
         }
         change /= double(thisV.size());
         cout << "NeuralFinder::CalcV_meta: Iteration " << i << ", Temp " << T << ", change " << change << ", largest V = " << bestV << endl;
         if(change <= itThres){
            converged = true;
            break;
         }
      }
      lastV = thisV;
   }

   return converged;
}

//==============================================================================================
int NeuralFinder::AssignTracks(){
   // reset tracking
   fNtracks = 0;
   fTrackVector.clear();
   for(auto &n: neurons){
      n.SetSubID(-1);
   }

   // assign new tracks
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

   // Paranoid checks
   // for(unsigned int i = 0; i < fTrackVector.size(); i++){
   //    for(int p: fTrackVector[i]){
   //       for(int ni: outNeurons[fPointsArray[p]]){
   //          Neuron &n = neurons[ni];
   //          if(n.GetActive()){
   //             if(n.GetSubID() != i){
   //                cout << "WTF! " << n.GetSubID() << " != " << i << endl;
   //             }
   //          }
   //       }
   //       for(int ni: inNeurons[fPointsArray[p]]){
   //          Neuron &n = neurons[ni];
   //          if(n.GetActive()){
   //             if(n.GetSubID() != i){
   //                cout << "WTF! " << n.GetSubID() << " != " << i << endl;
   //             }
   //          }
   //       }
   //    }
   // }

   // for(auto &n: neurons){
   //    if(n.GetActive()){
   //       track_t &t = fTrackVector[n.GetSubID()];
   //       if(std::find(t.begin(), t.end(), n.GetStartIdx()) == t.end() || std::find(t.begin(), t.end(), n.GetEndIdx()) == t.end()){
   //          cout << "WTF! One of points " << n.GetStartIdx() << ", " << n.GetEndIdx() << " missing in track " << n.GetSubID() << endl;
   //       }
   //    }
   // }

   if( fNtracks != int(fTrackVector.size()) )
      std::cerr<<"NeuralFinder::RecTracks(): Number of found tracks "<<fNtracks
               <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;

   return fNtracks;
}

//==============================================================================================
int NeuralFinder::MatchMetaTracks(){
   map<int, unsigned int> metaMap;
   for(auto i: inMeta){
      set<int> idset;
      for(int ni: i.second){
         if(metaNeurons[ni].GetActive())
            idset.insert(metaNeurons[ni].GetSubID());
      }
      for(int ni: outMeta[i.first]){
         if(metaNeurons[ni].GetActive())
            idset.insert(metaNeurons[ni].GetSubID());
      }
      if(idset.size() > 1){
         int masterID = *std::min_element(idset.begin(), idset.end());
         for(int id: idset){
            metaMap[id] = masterID;
         }
         for(int ni: outMeta[i.first]){
            if(metaNeurons[ni].GetActive() && metaNeurons[ni].GetSubID() != masterID){
               assert(metaMap.count(metaNeurons[ni].GetSubID()));
               assert((int)metaMap[metaNeurons[ni].GetSubID()] == masterID);
               metaNeurons[ni].SetSubID(masterID);
            }
         }
      }
   }
   for(auto &n: metaNeurons){
      if(n.GetActive())
         if(!metaMap.count(n.GetSubID()))
            cout << "SubID " << n.GetSubID() << " unknown" << endl;
   }
   for(auto &n: neurons){
      if(n.GetActive()){
         if(metaMap.count(n.GetSubID()))
            n.SetSubID(metaMap[n.GetSubID()]);
      }
   }
   vector<int> doneSubTracks;
   for(unsigned int i = 0; i < fTrackVector.size(); i++){
      if(metaMap[i] != i){
         track_t &dest = fTrackVector[metaMap[i]];
         track_t &source = fTrackVector[i];
         dest.insert(dest.end(), source.begin(), source.end());
         doneSubTracks.push_back(i);
      }
   }
   for(auto it = doneSubTracks.rbegin(); it != doneSubTracks.rend(); it++){
      fTrackVector.erase(fTrackVector.begin()+ *it);
      fNtracks--;
   }
   return doneSubTracks.size();
}

//==============================================================================================
set<int> NeuralFinder::FollowTrack(Neuron &n, int subID)
{
   set<int> pointset;
   if(n.GetActive()){
      if(n.GetSubID() < 0){
         n.SetSubID(subID);
         // cout << "NeuralFinder::FollowTrack: subID " << subID << ", adding points " << n.GetStartIdx() << " and " << n.GetEndIdx() << endl;
         pointset.insert(n.GetStartIdx());
         pointset.insert(n.GetEndIdx());
         vector<int> &inn = inNeurons[n.GetStartPt()];
         for(int i: inn){
            Neuron &ni = neurons[i];
            set<int> inPts = FollowTrack(ni, subID);
            pointset.insert(inPts.begin(), inPts.end());
         }
         vector<int> &outn = outNeurons[n.GetStartPt()];
         for(int i: outn){
            Neuron &no = neurons[i];
            set<int> outPts = FollowTrack(no, subID);
            pointset.insert(outPts.begin(), outPts.end());
         }
         vector<int> &inn2 = inNeurons[n.GetEndPt()];
         for(int i: inn2){
            Neuron &ni = neurons[i];
            set<int> inPts = FollowTrack(ni, subID);
            pointset.insert(inPts.begin(), inPts.end());
         }
         vector<int> &outn2 = outNeurons[n.GetEndPt()];
         for(int i: outn2){
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
int NeuralFinder::ApplyThreshold(double thres)
{
   if(thres > 1){
      cout << "NeuralFinder::ApplyThreshold: set threshold above 1, this will switch off all Neurons!" << endl;
   }
   VThres = thres;
   for(auto &n: neurons){
      // cout << "XXXXX " << n.GetV() << '\t' << int(n.GetV() >= VThres) << endl;
      n.SetActive(n.GetV() >= VThres);
   }
   cout << "NeuralFinder: Neurons : \t" << nneurons << "\tactive:\t" << CountActive() << endl;
   return AssignTracks();
}


//==============================================================================================
int NeuralFinder::RecTracks()
{
   // MakeNeurons();
   Run();
   AssignTracks();
   cout << "NeuralFinder: Neurons : \t" << nneurons << "\tactive:\t" << CountActive() << "\ttracks:\t" << fNtracks << endl;
   RunMeta();
   int n = MatchMetaTracks();
   cout << "NeuralFinder: sub-track matching: absorbed sub-tracks = " << n << "\ttracks:\t" << fNtracks << endl;
   return fNtracks;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
