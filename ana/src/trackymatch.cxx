#include "AgFlow.h"
#include "RecoFlow.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.hh"
#include <set>
#include <iostream>
#include <numeric>

#include "AnaSettings.hh"
#include "Match.hh"

class TrackyMatchFlags {
public:
   bool         fRecOff      = false; // Turn reconstruction off
   AnaSettings *ana_settings = NULL;
   bool         fDiag        = false;
   bool         fTrace       = false;
   bool         fForceReco   = false;

   int TotalThreads = 0;
   TrackyMatchFlags() // ctor
   {
   }

   ~TrackyMatchFlags() // dtor
   {
   }
};

class TrackyMatchModule : public TARunObject {
public:
   TrackyMatchFlags *fFlags     = NULL;
   bool              fTrace     = false;
   int               fCounter   = 0;
   bool              diagnostic = false;

private:
   std::vector<std::pair<int, double>> fSortedT;
   std::vector<std::pair<int, double>> fSortedPhi;
   std::vector<std::pair<int, double>> fSortedZ;
   class TSignalNode
   {
       public:
          TSignalNode* fLeft;
          TSignalNode* fRight;
          bool fIsPad;
          int fIndex;
          int fMass; // Counter to track the number of signals merged into this?
          double fPoint[3];// t, phi, z;
        TSignalNode(bool IsPad, int index, const ALPHAg::signal& s )
        {
            fLeft = fRight = NULL;
            fIsPad = IsPad;
            fIndex = index;
            fMass = 1;
            fPoint[0] = s.t;
            fPoint[1] = s.phi;
            fPoint[2] = s.z;
        }
        TSignalNode(const TSignalNode& n)
        {
            // Maybe copying the children is bad and we want NULLs?
            fLeft = n.fLeft;
            fRight = n.fRight;
            fIsPad = n.fIsPad;
            fIndex = n.fIndex;
            fMass = n.fMass;
            for (int i = 0; i < 3; i++)
            {
                fPoint[i] = n.fPoint[i];
            }
        }
        // Weighted mean of two points
        TSignalNode(const TSignalNode& a, const TSignalNode& b)
        {
           fLeft = fRight = NULL;
           fMass = a.fMass + b.fMass;
           assert (a.fIsPad == b.fIsPad);
           fIsPad = a.fIsPad;
           for (int i = 0; i < 3; i++)
           {
               fPoint[i] = a.fPoint[i]*a.fMass + b.fPoint[i]*b.fMass;
               fPoint[i] /= fMass;
           }
        }
    };

        // A method to create a node of K D tree
struct TSignalNode* newNode(TSignalNode* s)
{
    s->fLeft = s->fRight = NULL;
    return s;
}
  
// Inserts a new node and returns root of modified tree
// The parameter depth is used to decide axis of comparison
TSignalNode *insertRec(TSignalNode *root, TSignalNode* node, unsigned depth)
{
    // Tree is empty?
    if (root == NULL)
       return newNode(node);
  
    // Calculate current dimension (cd) of comparison
    unsigned cd = depth % 3;
  
    // Compare the new point with root on current dimension 'cd'
    // and decide the left or right subtree
    if (node->fPoint[cd] < (root->fPoint[cd]))
        root->fLeft  = insertRec(root->fLeft, node, depth + 1);
    else
        root->fRight = insertRec(root->fRight, node, depth + 1);
  
    return root;
}
  
// Function to insert a new point with given point in
// KD Tree and return new root. It mainly uses above recursive
// function "insertRec()"
TSignalNode* insert(TSignalNode *root, TSignalNode* node)
{
    return insertRec(root, node, 0);
}
  
// A utility method to determine if two Points are same
// in K Dimensional space
bool arePointsSame(const TSignalNode& node1, const TSignalNode& node2)
{
    // Compare individual pointinate values
    for (int i = 0; i < 3; ++i)
        if (node1.fPoint[i] != node2.fPoint[i])
            return false;
    return true;
}
  
// Searches a Point represented by "point[]" in the K D tree.
// The parameter depth is used to determine current axis.
bool searchRec(TSignalNode* root, const TSignalNode& node, unsigned depth)
{
    // Base cases
    if (root == NULL)
        return false;
    if (arePointsSame(*root, node))
        return true;
  
    // Current dimension is computed using current depth and total
    // dimensions (k)
    unsigned cd = depth % 3;
  
    // Compare point with root with respect to cd (Current dimension)
    if (node.fPoint[cd] < root->fPoint[cd])
        return searchRec(root->fLeft, node, depth + 1);
  
    return searchRec(root->fRight, node, depth + 1);
}
  
// Searches a Point in the K D tree. It mainly uses
// searchRec()
bool search(TSignalNode* root, const TSignalNode& node)
{
    // Pass current depth as 0
    return searchRec(root, node, 0);
}
/*
// A utility function to find minimum of three integers
double min(double x,double y, double z)
{
    return std::min(x, std::min(y, z));
}
  
// Recursively finds minimum of d'th dimension in KD tree
// The parameter depth is used to determine current axis.
double findMinRec(TSignalNode* root, int d, unsigned depth)
{
    // Base cases
    if (root == NULL)
        return INT_MAX;
  
    // Current dimension is computed using current depth and total
    // dimensions (k)
    unsigned cd = depth % 3;
  
    // Compare point with root with respect to cd (Current dimension)
    if (cd == d) {
        if (root->fLeft == NULL)
            return root->fPoint[d];
        return std::min(root->fPoint[d], findMinRec(root->fLeft, d, depth + 1));
    }
  
    // If current dimension is different then minimum can be anywhere
    // in this subtree
    return min(root->fPoint[d],
               findMinRec(root->fLeft, d, depth + 1),
               findMinRec(root->fRight, d, depth + 1));
}
  
// A wrapper over findMinRec(). Returns minimum of d'th dimension
double findMin(TSignalNode* root, int d)
{
    // Pass current level or depth as 0
    return findMinRec(root, d, 0);
}
*/
  
// A utility function to find minimum of three integers
TSignalNode *minNode(TSignalNode *x, TSignalNode *y, TSignalNode *z, int d)
{
    if (y != NULL && y->fPoint[d] < x->fPoint[d])
       return y;
    if (z != NULL && z->fPoint[d] < x->fPoint[d])
       return z;
    return x;
}

TSignalNode *minNodeType(TSignalNode *x, TSignalNode *y, TSignalNode *z, int d, bool IsPad)
{
    // Potential future optimisation: Fiddle with this ordering
    if (y != NULL && y->fPoint[d] < x->fPoint[d])
       if (y->fIsPad == IsPad)
          return y;
    if (z != NULL && z->fPoint[d] < x->fPoint[d])
       if (z->fIsPad == IsPad)
         return z;
    return x;
}

// Recursively finds minimum of d'th dimension in KD tree
// The parameter depth is used to determine current axis.
TSignalNode *findMinRec(TSignalNode* root, int d, unsigned depth)
{
    // Base cases
    if (root == NULL)
        return NULL;
  
    // Current dimension is computed using current depth and total
    // dimensions (k)
    unsigned cd = depth % 3;
  
    // Compare point with root with respect to cd (Current dimension)
    if (cd == d)
    {
        if (root->fLeft == NULL)
            return root;
        return findMinRec(root->fLeft, d, depth+1);
    }
  
    // If current dimension is different then minimum can be anywhere
    // in this subtree
    return minNode(root,
               findMinRec(root->fLeft, d, depth+1),
               findMinRec(root->fRight, d, depth+1), d);
}
TSignalNode *findMinRecType(TSignalNode* root, int d, unsigned depth, bool IsPad)
{
    // Base cases
    if (root == NULL)
        return NULL;
  
    // Current dimension is computed using current depth and total
    // dimensions (k)
    unsigned cd = depth % 3;
  
    // Compare point with root with respect to cd (Current dimension)
    if (cd == d)
    {
        if (root->fLeft == NULL)
            return root;
        return findMinRecType(root->fLeft, d, depth+1, IsPad);
    }
  
    // If current dimension is different then minimum can be anywhere
    // in this subtree
    return minNodeType(root,
               findMinRecType(root->fLeft, d, depth+1, IsPad),
               findMinRecType(root->fRight, d, depth+1, IsPad), 
               d, 
               IsPad);
}
  
// A wrapper over findMinRec(). Returns minimum of d'th dimension
TSignalNode *findMin(TSignalNode* root, int d)
{
    // Pass current level or depth as 0
    return findMinRec(root, d, 0);
}

TSignalNode *findMinPad(TSignalNode* root, int d)
{
    // Pass current level or depth as 0
    return findMinRecType(root, d, 0, true);
}

TSignalNode *findMinWire(TSignalNode* root, int d)
{
    // Pass current level or depth as 0
    return findMinRecType(root, d, 0, false);
}

// Copies point p2 to p1
void copyPoint(double p1[], const double p2[])
{
   for (int i=0; i<3; i++)
       p1[i] = p2[i];
}
  
// Function to delete a given point 'point[]' from tree with root
// as 'root'.  depth is current depth and passed as 0 initially.
// Returns root of the modified tree.
TSignalNode *deleteNodeRec(TSignalNode *root, const TSignalNode& node, int depth)
{
    // Given point is not present
    if (root == NULL)
        return NULL;
  
    // Find dimension of current node
    int cd = depth % 3;
  
    // If the point to be deleted is present at root
    if (arePointsSame(*root, node))
    {
        // 2.b) If right child is not NULL
        if (root->fRight != NULL)
        {
            // Find minimum of root's dimension in right subtree
            TSignalNode *min = findMin(root->fRight, cd);
  
            // Copy the minimum to root
            copyPoint(root->fPoint, min->fPoint);
            root->fIndex = min->fIndex;
  
            // Recursively delete the minimum
            root->fRight = deleteNodeRec(root->fRight, *min, depth+1);
        }
        else if (root->fLeft != NULL) // same as above
        {
            TSignalNode *min = findMin(root->fLeft, cd);
            copyPoint(root->fPoint, min->fPoint);
            root->fIndex = min->fIndex;
            root->fRight = deleteNodeRec(root->fLeft, *min, depth+1);
        }
        else // If node to be deleted is leaf node
        {
            delete root;
            return NULL;
        }
        return root;
    }
  
    // 2) If current node doesn't contain point, search downward
    if (node.fPoint[cd] < root->fPoint[cd])
        root->fLeft = deleteNodeRec(root->fLeft, node, depth+1);
    else
        root->fRight = deleteNodeRec(root->fRight, node, depth+1);
    return root;
}
  
// Function to delete a given point from K D Tree with 'root'
 TSignalNode* deleteNode(TSignalNode *root, const TSignalNode& node)
{
   // Pass depth as 0
   return deleteNodeRec(root, node, 0);
}


    TSignalNode* fKDTreeRoot;
    //std::vector<TSignalNode*> fKDTreeEnds;

public:
   TrackyMatchModule(TARunInfo *runinfo, TrackyMatchFlags *f) : TARunObject(runinfo)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName = "TrackyMatch";
#endif
      if (fTrace) printf("TrackyMatchModule::ctor!\n");

      fFlags = f;

      diagnostic = fFlags->fDiag;  // dis/en-able histogramming
      fTrace     = fFlags->fTrace; // enable verbosity
   }

   ~TrackyMatchModule()
   {
      if (fTrace) printf("TrackyMatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("TrackyMatchModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
   }
   void EndRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("TrackyMatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }

   void PauseRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent *AnalyzeFlowEvent(TARunInfo *runinfo, TAFlags *flags, TAFlowEvent *flow)
   {
      if (fTrace) printf("TrackyMatchModule::Analyze, run %d, counter %d\n", runinfo->fRunNo, fCounter++);

      // turn off recostruction
      if (fFlags->fRecOff) {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }

      AgSignalsFlow *SigFlow = flow->Find<AgSignalsFlow>();
      if (!SigFlow) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }

      if (!SigFlow->awSig) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }
      if (fTrace) {
         printf("TrackyMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
         printf("TrackyMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
      }

      fKDTreeRoot = NULL;
      std::vector<TSignalNode> node_list;
      if (SigFlow->awSig) {
         int i = 0;
         printf("TrackyMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));

         for (const ALPHAg::signal &s : *(SigFlow->awSig)) {
             if (s.t>0)
                node_list.emplace_back(false,i,s);
             i++;
             //fKDTreeRoot = insert(fKDTreeRoot,TSignalNode(i++,false, s));
         }
      }

      if (SigFlow->pdSig) {
         printf("TrackyMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));

         int i = 0;
         for (const ALPHAg::signal &s : *(SigFlow->pdSig)) {
             if (s.t>0)
                node_list.emplace_back(true,i,s);
             i++;
             //fKDTreeRoot = insert(fKDTreeRoot,TSignalNode(i++,true,s));
             /*fSortedT.emplace_back(std::make_pair(i,s.t));
             fSortedPhi.emplace_back(std::make_pair(i,s.phi));
             fSortedZ.emplace_back(std::make_pair(i,s.z));
             i++;
            std::cout << s.t << "\t" << s.phi << "\t" << s.z << std::endl;*/
         }
      }
      for (TSignalNode& n: node_list)
      {
          fKDTreeRoot = insert(fKDTreeRoot,&n);
      }             

      std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>> *spacepoints = NULL;

      if (SigFlow->pdSig && SigFlow->pdSig) {
          spacepoints = new std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>>();
         for( int i = 0; i < node_list.size(); i++)
         {
             // Match in T, Phi and Z (wires dont have Z information so this is a bad idea?)
             //TSignalNode* mins[3] = {NULL};
             //for (int j = 0; j < 3; j++)
             // For now... only wires seek pads
             TSignalNode* thisNode = &node_list[i];

             if (thisNode->fIsPad)
                break;
             // Match in T and Phi
             TSignalNode* mins[2] = {NULL};
             for (int j = 0; j < 2; j++)
             {
                 mins[j] = findMin(thisNode, j);
             }
             // If its the closest in T and Phi... then pair them!
             if (mins[0] == mins[1])
             {
                //std::cout<<"mins match\n";
                // Time cut!
                if (fabs( mins[0]->fPoint[0] - thisNode->fPoint[0] ) < 0.1)
                   // Phi cut!
                   if (fabs( mins[0]->fPoint[1] - thisNode->fPoint[1] ) < 1)
                   {
                       //std::cout << "Cut passed"<<std::endl;
                       const ALPHAg::signal& wire_sig = SigFlow->awSig->at(thisNode->fIndex);
                       const ALPHAg::signal& pad_sig = SigFlow->awSig->at(thisNode->fIndex);
                       if (wire_sig.height>100 && pad_sig.height >100)
                       spacepoints->emplace_back(
                          std::make_pair(
                              SigFlow->awSig->at(thisNode->fIndex),
                              SigFlow->pdSig->at(mins[0]->fIndex)

                              )
                      );
                      }

             }
         }

      }
      
      /*
      std::sort(fSortedT.begin(), fSortedT.end(), [](const std::pair<int, double> &x,
                                       const std::pair<int, double> &y)
      {
         return x.second < y.second;
      });
      std::sort(fSortedPhi.begin(), fSortedPhi.end(), [](const std::pair<int, double> &x,
                                       const std::pair<int, double> &y)
      {
         return x.second < y.second;
      });
      std::sort(fSortedZ.begin(), fSortedZ.end(), [](const std::pair<int, double> &x,
                                       const std::pair<int, double> &y)
      {
         return x.second < y.second;
      });
      for (const std::pair<int,double>& p: fSortedT)
         std::cout<<"t= "<<p.second<<" (" << p.first <<" )\n";
      for (const std::pair<int,double>& p: fSortedPhi)
         std::cout<<"phi= "<<p.second<<" (" <<p.first<<" )\n";*/
      //double TMean = std::accumulate(fSortedT.begin(), fSortedT.end(), 0.0) / (double)fSortedT.size();
      //double PhiMean = std::accumulate(fSortedPhi.begin(), fSortedPhi.end(), 0.0) / (double)fSortedPhi.size();
      //double ZMean = std::accumulate(fSortedZ.begin(), fSortedZ.end(), 0.0) / (double)fSortedZ.size();

      // allow events without pwbs
      if (SigFlow->combinedPads) {
         // spacepoints = match->CombPoints(spacepoints);
      } else if (fFlags->fForceReco) // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
      {
         if (fTrace) printf("TrackyMatchModule::Analyze, NO combined pads, Set Z=0\n");
         // delete match->GetCombinedPads();?
         //         spacepoints = match->FakePads( SigFlow->awSig );
      }

      if (spacepoints) {
         if (fFlags->fTrace) printf("TrackyMatchModule::Analyze, Spacepoints # %d\n", int(spacepoints->size()));
         if (spacepoints->size() > 0) SigFlow->AddMatchSignals(spacepoints);
      } else
         printf("TrackyMatchModule::Analyze Spacepoints should exists at this point\n");

      delete spacepoints;
      return flow;
   }
};

class TrackyMatchFactory : public TAFactory {
public:
   TrackyMatchFlags fFlags;

public:
   void Help()
   {
      printf("TrackyMatchFactory::Help\n");
      printf("\t--forcereco\t\tEnable reconstruction when no pads are associated with the event by setting z=0\n");
   }
   void Usage() { Help(); }

   void Init(const std::vector<std::string> &args)
   {
      TString json = "default";
      // printf("TrackyMatchFactory::Init!\n");
      for (unsigned i = 0; i < args.size(); i++) {
         if (args[i] == "--recoff") fFlags.fRecOff = true;
         if (args[i] == "--diag") fFlags.fDiag = true;
         if (args[i] == "--trace") fFlags.fTrace = true;
         if (args[i] == "--forcereco") fFlags.fForceReco = true;
         if (args[i] == "--anasettings") {
            i++;
            json = args[i];
            i++;
         }
      }
      fFlags.ana_settings = new AnaSettings(json);
      if (fFlags.fTrace) fFlags.ana_settings->Print();
   }

   TrackyMatchFactory() {}

   void Finish()
   {
      if (fFlags.fTrace == true) printf("TrackyMatchFactory::Finish!\n");
   }

   TARunObject *NewRunObject(TARunInfo *runinfo)
   {
      if (fFlags.fTrace == true)
         printf("TrackyMatchFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TrackyMatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new TrackyMatchFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
