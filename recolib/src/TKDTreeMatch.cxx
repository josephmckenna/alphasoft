

#include "TKDTreeMatch.hh"


void TKDTreeMatch::BuildTree(const std::vector<ALPHAg::TPadSignal> pads, KDTreeIDContainer2D* pad_tree, const double PhiFactor)
{
   for (const ALPHAg::TPadSignal &s : pads) {
      pad_tree->emplace_back( s.t, PhiFactor * s.phi);
   }
   pad_tree->Build();
   return;
}

void TKDTreeMatch::WiresFindPads(
    const std::vector<ALPHAg::TWireSignal> wires,
    const std::vector<ALPHAg::TPadSignal> pads,
    KDTreeIDContainer2D* pad_tree,
    const double PhiFactor,
    std::vector<std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>>& spacepoints)
{
   for (const ALPHAg::TWireSignal &s : wires) {
      if (s.t > 0) {
         std::array<double, 2> data = {s.t,PhiFactor * s.phi};
         int                   index = 0;
         double                distance = 9999999999999999999.;

        pad_tree->FindNearestNeighbors(data.begin(), 1, &index, &distance);
        if (index < 0)
           continue;
        const ALPHAg::TPadSignal& pad = pads.at(index);
        if (fabs(pad.phi - s.phi) < 3* pad.errphi &&  fabs(pad.t - s.t) < 20)
        {
           spacepoints.emplace_back(
              std::make_pair(
                 s,
                 pad
               )
            );
            //std::cout <<"Index:"<< index << "\t"<< distance <<std::endl;
         }
     }
   }
}

void TKDTreeMatch::BuildTree(const std::vector<ALPHAg::TWireSignal> wires, KDTreeIDContainer2D* pad_tree, const double PhiFactor)
{
   for (const ALPHAg::TWireSignal &s : wires) {
      pad_tree->emplace_back( s.t, PhiFactor * s.phi);
   }
   pad_tree->Build();
   return;
}

void TKDTreeMatch::PadsFindWires(
    const std::vector<ALPHAg::TPadSignal> pads,
    const std::vector<ALPHAg::TWireSignal> wires,
    KDTreeIDContainer2D* wire_tree,
    const double PhiFactor,
    std::vector<std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>>& spacepoints)
{
   for (const ALPHAg::TPadSignal &s : pads) {
      if (s.t > 0) {
         std::array<double, 2> data = {s.t,PhiFactor * s.phi};
         int                   index = 0;
         double                distance = 9999999999999999999.;

        wire_tree->FindNearestNeighbors(data.begin(), 1, &index, &distance);
        if (index < 0)
           continue;
        const ALPHAg::TWireSignal& wire = wires.at(index);
        if (fabs(wire.phi - s.phi) < 3* wire.errphi &&  fabs(wire.t - s.t) < 20)
        {
           spacepoints.emplace_back(
              std::make_pair(
                 wire,
                 s
               )
            );
            //std::cout <<"Index:"<< index << "\t"<< distance <<std::endl;
         }
     }
   }
}
