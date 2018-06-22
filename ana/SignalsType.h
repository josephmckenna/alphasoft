#ifndef __SIG_T__
#define __SIG_T__ 1
struct electrode
{
   electrode(short s, int ind, double g = 1.)
   { 
      sec = s; 
      idx = ind; 
      gain = g;
   };
   electrode(int ind)
   {
      sec = 1; // bottom
      idx = -1;
      if( ind >= 256 )
         {
            sec = 0; // top
            idx = ind - 256;
         }
      else
         idx = ind;
      gain=1.;
   }
#if 0
   void setwire(int ind)
   {
      sec = 1; // bottom
      idx = -1;
      if( ind >= 256 )
         {
            sec = 0; // top
            idx = ind - 256;
         }
      else
         idx = ind;
      gain=1.;
   }
   void setpad(short s, int ind)
   { 
      sec = s; 
      idx = ind; 
      gain = 1.;
   };
#endif
   short sec;  // for anodes sec=0 for top, sec=1 for bottom
   int idx;
   double gain;
   void print(){ printf("electrode:: %d sector: %d (gain: %1.0f)\n",idx,sec,gain); };
};

class signal
{
public:
   int i;
   short sec;  // for anodes sec=0 for top, sec=1 for bottom
   double t, height, z;
   signal(electrode el, double tt, double hh)
   {
      i = el.idx;
      sec = el.sec;
      t = tt;
      height = hh/el.gain;  // should the gain be used here?
      //      z = kUnknown;
      z = -9.e9;
   }
   
   signal(int ii, short ss, double tt, double hh)
   {
      i = ii;
      sec = ss;
      t = tt;
      height = hh;
      //z = kUnknown;
      z = -9.e9;
   }

   struct indexorder {       // to sort signals by wire/pad number
      bool operator() (const signal& lhs, const signal& rhs) const {
         return lhs.i<rhs.i || (lhs.i==rhs.i && lhs.sec<rhs.sec);
      }
   };
   
   struct timeorder{        // to sort signals by time
      bool operator() (const signal& lhs, const signal& rhs) const {
         return lhs.t<rhs.t;
      }
   };
   
   struct heightorder {       // to sort signals by signal size
      bool operator() (const signal& lhs, const signal& rhs) const {
         return lhs.height>rhs.height;
      }
   };
};

struct wfholder 
{
   std::vector<double> *h;
   double val;
   unsigned int index;
};

struct comp_hist 
{
   bool operator() (const wfholder &lhs, const wfholder &rhs) const
   {return lhs.val >= rhs.val;}
};

#endif
