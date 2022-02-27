#ifndef __SIG_T__
#define __SIG_T__ 1

#include <cstdio>
#include <iostream>
#include <map>
#include <algorithm>
#include "TPCconstants.hh"
#include <cmath>

namespace ALPHAg {

class electrode
{
public:
   short sec;  // for anodes sec=0 for top, sec=1 for bottom
   // for pads [0,31] col (phi)
   int idx; // for anodes [0,255]
   // for pads [0,575] row (z)
   double gain;


   electrode():sec(-1),idx(-1),gain(1.)
   {}
   ~electrode()
   {
      
   }

   electrode(short s, int ind, double g):sec(s),  // AW:top/bottom PAD:col(phi)
                                         idx(ind),// AW:wire PAD:row(z)
                                         gain(g)
   {
      if(!(gain>0.)) gain=1.;
   }

   electrode(short s, int ind):sec(s),  // AW:top/bottom PAD:col(phi)
                               idx(ind),// AW:wire PAD:row(z)
                               gain(1.0)
   {}

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

   electrode(int ind, double g):gain(g)
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
      if(!(gain>0.)) gain=1.;
   }

   electrode(const electrode &el):sec(el.sec),idx(el.idx),gain(el.gain)
   {}
   electrode& operator=(const electrode &el)
   {
      sec = el.sec;
      idx = el.idx;
      gain = el.gain;
      return *this;
   }

   void setgain(double g) { if(g>0.) gain=g; };

   virtual void print() const
   {
      printf("electrode:: %d sector: %d (gain: %1.0f)\n",idx,sec,gain);
   };
};

class signal: public electrode
{
public:
   double t, height, errh;

  signal():electrode(),
	   t(ALPHAg::kUnknown),height(0.),errh(ALPHAg::kUnknown)
  {}
  ~signal()
  {
  }

   signal(const electrode& el, const double tt, const double hh, const double eh):electrode(el),
                                                                       t(tt)
   {
      height = hh/gain;  // should the gain be used here?
      errh = eh/gain;
   }

   signal(const signal &sig):electrode(sig),
                             t(sig.t), height(sig.height), errh(sig.errh)
   {}

   signal& operator=(const signal& sig)
   {
      //electrode members... can I be clever with calling the parent class's operator overload?
      sec = sig.sec;
      idx = sig.idx;
      gain = sig.gain;

      t = sig.t;
      height = sig.height;
      errh = sig.errh;
      return *this;
   }


   virtual void print() const
   {
      printf("electrode:: %d sector: %d (gain: %1.0f)\tsignal:: t=%1.0f ns H=%1.0f E=%1.0f\n",
             idx,sec,gain,t,height,errh);

   }

   struct indexorder {       // to sort signals by wire/pad number
      bool operator() (const signal& lhs, const signal& rhs) const {
         return lhs.idx<rhs.idx || (lhs.idx==rhs.idx && lhs.sec<rhs.sec);
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

   struct sectororder {       // to sort signals by wire/pad number
      bool operator() (const signal& lhs, const signal& rhs) const {
         return lhs.sec<rhs.sec;
      }
   };
};




class TWireSignal: public signal
{
   public:
   double phi, errphi;
   TWireSignal(): signal(), phi(ALPHAg::kUnknown), errphi(ALPHAg::kUnknown)
   {
   }
   
   TWireSignal(const electrode& el, const double tt, const double hh, const double eh):signal(el, tt,hh,eh)
   {
      phi = ALPHAg::_anodepitch * ( double(idx) + 0.5 );
      errphi = ALPHAg::_anodepitch * ALPHAg::_sq12;
   }

   TWireSignal(const TWireSignal &sig): signal(sig)
   {
      phi = sig.phi;
      errphi = sig.errphi;
   }

   TWireSignal& operator=(const TWireSignal& sig)
   {
      //electrode members... can I be clever with calling the parent class's operator overload?
      sec = sig.sec;
      idx = sig.idx;
      gain = sig.gain;

      t = sig.t;
      height = sig.height;
      errh = sig.errh;
      phi = sig.phi;
      errphi = sig.errphi;
      return *this;
   }

};

class TPadSignal: public signal
{
   public:
   double z, errz;
   double phi, errphi;
   TPadSignal(): signal(), z(ALPHAg::kUnknown), errz(ALPHAg::kUnknown), phi(ALPHAg::kUnknown), errphi(ALPHAg::kUnknown)
   {
   }
   TPadSignal(const electrode& el, const double tt, const double hh, const double eh):signal(el, tt,hh,eh)
   {
      z = ( double(idx) + 0.5 ) * ALPHAg::_padpitch - ALPHAg::_halflength;
      errz = ALPHAg::_padpitch * ALPHAg::_sq12;
      phi = 2*M_PI / _padcol * ( double(sec) + 0.5 );
      errphi = 2*M_PI / _padcol * ALPHAg::_sq12;
   }

   TPadSignal(const electrode& el, const double tt, const double hh, const double eh, double _z, double _errz):
      signal(el, tt,hh,eh), z(_z), errz(_errz)
   {
      phi = 2*M_PI / _padcol * ( double(sec) + 0.5 );
      errphi = 2*M_PI / _padcol * ALPHAg::_sq12;
   }

   TPadSignal(const TPadSignal &sig): signal(sig)
   {
      z = sig.z;
      errz = sig.errz;
      phi = sig.phi;
      errphi = sig.errphi;
   }

   TPadSignal& operator=(const TPadSignal& sig)
   {
      //electrode members... can I be clever with calling the parent class's operator overload?
      sec = sig.sec;
      idx = sig.idx;
      gain = sig.gain;

      t = sig.t;
      height = sig.height;
      errh = sig.errh;
      z = sig.z;
      errz = sig.errz;
      phi = sig.phi;
      errphi = sig.errphi;
      return *this;
   }
};

struct wfholder
{
   std::vector<double> h;
   double val;
   unsigned int index;
   wfholder( unsigned int& ii, 
             std::vector<int>::const_iterator begin,  
             std::vector<int>::const_iterator end): h(begin,end)
   {
      val = -1.0;
      index=ii;
   }

   ~wfholder() { h.clear(); }

   void massage(double& ped, double& norm)
   {
      //SUBTRACT PEDESTAL
      std::for_each(h.begin(), h.end(), [ped](double& d) { d-=ped;});
      // NORMALIZE WF
      std::for_each(h.begin(), h.end(), [norm](double& v) { v*=norm;});
   }
   
   void print() const
   {
      std::cout<<"wfholder:: size: "<<h.size()
               <<", val: "<<val<<", index: "<<index<<std::endl;
   }
};

struct comp_hist_t
{
   bool operator() (wfholder* lhs, wfholder* rhs) const
   {
      return lhs->val > rhs->val;
   }
};

class wf_ref
{
public:
   int i;
   short sec;  // for anodes sec=0 for top, sec=1 for bottom
   std::vector<double> *wf;
   wf_ref(electrode el, std::vector<double> *wfv): i(el.idx), sec(el.sec), wf(wfv){ }
   wf_ref(int ii, short ss, std::vector<double> *wfv): i(ii), sec(ss), wf(wfv){ }
   ~wf_ref() { delete wf; }
};

class padmap
{
private:
   std::map<int,std::pair<int,int>> fmap;
public:
   padmap()
   {
      for(int r = 0; r<576; ++r)
         for(int s = 0; s<32; ++s)
            fmap[index(s,r)]=std::make_pair(s,r);
   }
   ~padmap()
   {
      fmap.clear();
   }

   inline void get(int i, int& sec, int& row)
   {
      sec=fmap[i].first;
      row=fmap[i].second;
   }
   inline void get(int i, short& sec, int& row)
   {
      sec=short(fmap[i].first);
      row=fmap[i].second;
   }
   inline int getsector(int i) const
   {
      int sec=-1;
      if( i>=0 && i<32*576 )
         sec=fmap.at(i).first;
      return sec;
   }
   inline int getrow(int i) const
   {
      int row = -1;
      if( i>=0 && i<32*576 )
         row = fmap.at(i).second;
      return row;
   }
   inline int index(int& sec, int& row) const
   {
      return sec + 32 * row;
   }
   inline int index(short& sec, int& row) const
   {
      return sec + 32 * row;
   }
   inline void print()
   {
      for (auto& x: fmap)
         std::cout << x.first << " => " << x.second.first << ":" << x.second.second << '\n';
   }
};


} //namespace ALPHAg
#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
