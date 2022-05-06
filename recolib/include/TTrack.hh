// Track class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: June 2016

#ifndef __TTRACK__
#define __TTRACK__ 1

#include "TObject.h"
#include "TObjArray.h"
#include "TVector3.h"

#include "TPolyLine.h"
#include <iomanip>
#include <map>

#include "TSpacePoint.hh"

class TTrack: public TObject
{
protected:
  std::vector<TSpacePoint> fPoints;
  int fNpoints;
  double fB;

  int fStatus;
  int fParticle;

  int fPointsCut;

  TVector3 fResidual;
  std::vector<double> fResiduals;
  std::map<double,double> fResidualsRadii;
  std::map<double,double> fResidualsPhi;
  std::map< std::pair<double,double>, double> fResidualsXY;
  double fResiduals2;

  const TVector3* fPoint;

public:
  TTrack();
  TTrack(TObjArray*, double);
  TTrack(const TObjArray*);
  TTrack(double);

  virtual ~TTrack();

  TTrack( const TTrack& );
  TTrack& operator=( const TTrack& );

  virtual void Fit();

  int AddPoint(const TSpacePoint&);
  inline const std::vector<TSpacePoint>* GetPointsArray() const {return &fPoints;}
  const std::pair<const TSpacePoint*,const TSpacePoint*> GetLastSpacePoint() const { 
    std::pair<const TSpacePoint*,const TSpacePoint*> FirstLast= {nullptr,nullptr};
    double Rmin = 1.0/0.0;
    double Rmax = -1.;
    for (const TSpacePoint& p: fPoints)
    {
       const double R = p.GetR();
       if ( R < Rmin)
       {
          Rmin = R;
          FirstLast.second = &p;
       }
       if ( R > Rmax)
       {
          Rmax = R;
          FirstLast.first = &p;
       }
    }
    return FirstLast;
  }
  inline void SetPointsArray(std::vector<TSpacePoint>* array)   {fPoints=*array;}
  inline int GetNumberOfPoints()           const {return fNpoints;}
  inline void SetNumberOfPoints(int np)          {fNpoints = np;}

  inline void SetMagneticField(double b) { fB = b; }
  inline double GetMagneticField() const { return fB;}

  inline int GetStatus() const {return fStatus;}
  inline void SetStatus(int s) {fStatus=s;}

  inline void SetParticleType(int pdg) {fParticle=pdg;}
  inline int GetParticleType() const   {return fParticle;}

  inline void SetPointsCut(unsigned cut) {fPointsCut=cut;}
  inline unsigned GetPointsCut() const   {return fPointsCut;}

  // Evaluate the function
  virtual TVector3 Evaluate( double )        {TVector3 v(0.,0.,0.); return v;}
  virtual TVector3 EvaluateErrors2( double ) {TVector3 v(0.,0.,0.); return v;}
  virtual TVector3 GetPosition(double )      {TVector3 v(0.,0.,0.); return v;}
  virtual TVector3 GetError2(double )        {TVector3 v(0.,0.,0.); return v;}

  virtual double GetApproxPathLength();

  virtual double CalculateResiduals();
  virtual TVector3 GetResidual()                   const { return fResidual; }
  virtual std::vector<double> GetResidualsVector() const { return fResiduals; }  
  virtual double GetResidualsSquared()             const { return fResiduals2; }

  virtual std::map<double,double> GetResidualsRadiusMap() const { return fResidualsRadii; } 
  virtual std::map<double,double> GetResidualsPhiMap() const { return fResidualsPhi; }
  virtual std::map<std::pair<double,double>,double> GetResidualsXYMap() const { return fResidualsXY; }

  virtual void SetResidual(TVector3 r)                    { fResidual=r; }
  virtual void SetResidualsVector(std::vector<double>& r) { fResiduals=r; }
  virtual void SetResidualsSquared(double rq)             { fResiduals2=rq; }
  
  virtual bool IsGood();
  virtual void Reason();

  inline const TVector3* GetPoint() const     { return fPoint; }
  inline void SetPoint(const TVector3* point) { fPoint=point; }
  virtual double MinDistPoint(TVector3&);
  virtual double MinRad() {return 0.;}

  virtual void Print(Option_t *option="") const;
  virtual void Clear(Option_t *option="");

  ClassDef(TTrack,2)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
