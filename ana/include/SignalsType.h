#ifndef __SIG_T__
#define __SIG_T__ 1

#include "TPCconstants.hh"

class electrode
{
public:
  electrode():sec(-1),idx(-1),gain(1.)
  {}

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

  void setgain(double g) { if(g>0.) gain=g; };
  
  short sec;  // for anodes sec=0 for top, sec=1 for bottom
              // for pads [0,31] col (phi)
  int idx; // for anodes [0,255]
           // for pads [0,575] row (z)
  double gain;
  virtual void print()
  { 
    printf("electrode:: %d sector: %d (gain: %1.0f)\n",idx,sec,gain); 
  };
};

class signal: public electrode
{
public:
  double t, height, z, errz;

  signal():electrode(),
	   t(kUnknown),height(0.),
	   z(kUnknown),errz(kUnknown)
  {}

  signal(electrode el, double tt, double hh):electrode(el),
					     t(tt),z(kUnknown),errz(kUnknown)
  {
    height = hh/el.gain;  // should the gain be used here?
  }
   
  signal(short ss, int ii, double tt, double hh):electrode(ss, ii),
						 t(tt),z(kUnknown),errz(kUnknown)
  {
    height = hh/gain;
  }

  signal(int ii, double tt, double hh):electrode(ii),
				       t(tt),z(kUnknown),errz(kUnknown)
  {
    height = hh/gain;
  }

  signal(short ss, int ii, 
	 double tt, double hh, 
	 double zz, double ez=kUnknown):electrode(ss, ii),
					t(tt),z(zz),errz(ez)
  {
    height = hh/gain;
  }
  
  signal(const signal &sig):electrode(sig),
			    t(sig.t), height(sig.height),
			    z(sig.z), errz(sig.errz)
  {}
  
  virtual void print()
  {
    printf("electrode:: %d sector: %d (gain: %1.0f)\tsignal:: t=%1.0f ns H=%1.0f\n",
	   idx,sec,gain,t,height);
    
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

struct wfholder 
{
  std::vector<double> *h;
  double val;
  unsigned int index;
  void print() const
  {
    std::cout<<"wfholder:: size: "<<h->size()
	     <<", val: "<<val<<", index: "<<index<<std::endl;
  }
};

struct comp_hist_t 
{
  bool operator() (wfholder* lhs, wfholder* rhs) const
  {
    return lhs->val >= rhs->val;
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
 
    row=fmap[i].second;
  }
  inline int getsector(int i) const 
  {
    int sec=-1;
    if( i>=0 && i<32 )
      sec=fmap.at(i).first;
    return sec;
  }
  inline int getrow(int i) const 
  {
    int row = -1;
    if( i>=0 && i<576 )
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

#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
