#ifndef __TKDTREEMATCH__
#define __TKDTREEMATCH__
#include <vector>
#include <array>
#include "TKDTree.h"
#include "SignalsType.hh"

class KDTreeIDContainer2D
{
   private:
      std::vector<double> fX;
      std::vector<double> fY;
      TKDTreeID fKDTreeID;
      std::string fTreeName;
   public:
      KDTreeIDContainer2D(int entires, const char* treename):
         fKDTreeID(entires,2,1),
         fTreeName(treename)
      {
         fX.reserve(entires);
         fY.reserve(entires);
      }
      bool IsMatch(const std::string name) const
      {
         return name == fTreeName;
      }
      std::vector<double>& X() { return fX; }
      std::vector<double>& Y() { return fY; }
      void emplace_back(const double x, const double y)
      {
         fX.emplace_back(x);
         fY.emplace_back(y);
      }
      void Build()
      {
         fKDTreeID.SetData(0, fX.data());
         fKDTreeID.SetData(1, fY.data());
         fKDTreeID.Build();
      }
      void FindNearestNeighbors(const double *point, Int_t k, int *ind, double *dist)
      {
         return fKDTreeID.FindNearestNeighbors(point,k,ind,dist);
      }

};

class TKDTreeMatch
{
   public:
      TKDTreeMatch() {};
      ~TKDTreeMatch() {};
      //Pads find wires
      void BuildTree(const std::vector<ALPHAg::TPadSignal> pads, KDTreeIDContainer2D* pad_tree, const double PhiFactor);
      void WiresFindPads(
          const std::vector<ALPHAg::TWireSignal> wires,
          const std::vector<ALPHAg::TPadSignal> pads,
          KDTreeIDContainer2D* pad_tree,
          const double PhiFactor,
          std::vector<std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>>& spacepoints);
      //Wires find pads
      void BuildTree(const std::vector<ALPHAg::TWireSignal> wires, KDTreeIDContainer2D* pad_tree, const double PhiFactor);
      void PadsFindWires(
         const std::vector<ALPHAg::TPadSignal> pads,
         const std::vector<ALPHAg::TWireSignal> wires,
         KDTreeIDContainer2D* wire_tree,
         const double PhiFactor,
         std::vector<std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>>& spacepoints);
};


#endif