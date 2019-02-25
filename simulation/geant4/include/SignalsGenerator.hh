#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <random>

class SignalsGenerator
{
private:
  std::vector<int> fAWaval;
  std::vector<int> fPADaval;

  std::map<uint,std::vector<int>> fAnodeSignals;
  std::map<std::pair<int,int>,std::vector<int>> fPadSignals;

  std::set<std::pair<int,int>> fPadHit;
  std::map<std::pair<int,int>,std::vector<int>> fPadSignals_zerosuppression;

  int fNoiseLevel;

  double fBinWidth;
  int fNbins;
  int fPedLen;

  double* fInductionAnodes;
  double fPadsChargeSigma;

  double fPadsChargeProfile(double&,double&,double&);
  int GetBin(double&);

  double mV2ADC;

  std::mt19937_64 gen;
  std::uniform_int_distribution<int> uuint;

public:
  SignalsGenerator(double noise_level);
  virtual ~SignalsGenerator();

  void Initialize();
  void Reset();
  std::vector<int> Response(std::ifstream&);

  void AddAnodeSignal(uint& wire, double& time, double& gain);
  void AddPadSignal(std::pair<int,int>& pad, double& time, double& gain, double& z);

  inline const std::vector<int>* GetAnodeSignal(uint aw) const { return &fAnodeSignals.at(aw); }
  inline const std::map<uint,std::vector<int>>* GetAnodeSignal() const { return &fAnodeSignals; }
  inline const std::vector<int>* GetPadSignal(std::pair<int,int> pad) const { return &fPadSignals.at(pad); }
  inline const std::map<std::pair<int,int>,std::vector<int>>* GetPadSignal() const { return &fPadSignals; }

  const std::map<std::pair<int,int>,std::vector<int>>* GetZsPadSignal();

  inline void SetBinWidth(double w)  { fBinWidth = w; }
  inline void SetNumberOfBins(int n) { fNbins = n; }
  inline double GetBinWith() const   { return fBinWidth; } 
  inline int GetNumberOfBins() const { return fNbins; }
};
