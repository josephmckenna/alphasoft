#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <random>

class SignalsGenerator
{
private:
  std::vector<double> fAWaval;
  std::vector<double> fPADaval;

  std::map<uint,std::vector<double>*> fAnodeSignals;
  std::map<std::pair<int,int>,std::vector<double>*> fPadSignals;

  std::set<std::pair<int,int>> fPadHit;
  std::map<std::pair<int,int>,std::vector<int>*> fPadSignals_zerosuppression;

  std::map<uint,std::vector<int>*> fAnodeReadout;
  std::map<std::pair<int,int>,std::vector<int>*> fPadReadout;
  
  double fAnodeNoiseLevel;
  double fPadNoiseLevel;
  double fAnodeNoisePkPk;
  double fPadNoisePkPk;

  double fBinWidth;
  int fNbins;
  int fPedLen;

  double* fInductionAnodes;
  double fPadsChargeSigma;

  void AddSignal(int& bin, double& scale, std::vector<double>* aval, std::vector<double>* sig);
  double MakeNoise(double&);

  double fPadsChargeProfile(double&,double&,double&);
  int GetBin(double&);

  double mV2ADC;

  std::mt19937_64 gen;
  std::uniform_real_distribution<double> fNoise;

public:
  SignalsGenerator(double aw_noise, double pad_noise);
  virtual ~SignalsGenerator();

  inline void SetAnodeNoiseLevel(double nl) { 
    fAnodeNoiseLevel = nl; 
    fAnodeNoisePkPk = fAnodeNoiseLevel*sqrt(12.)*mV2ADC*0.5;}
  inline double GetAnodeNoiseLevel() const  { return fAnodeNoiseLevel; }
  inline void SetPadNoiseLevel(double nl) { 
    fPadNoiseLevel = nl; 
    fPadNoisePkPk = fPadNoiseLevel*sqrt(12.)*mV2ADC*0.5;}
  inline double GetPadNoiseLevel() const  { return fPadNoiseLevel; }
  void PrintNoiseLevels();

  void Initialize();
  void Reset();
  std::vector<double> Response(std::ifstream&);

  void AddAnodeSignal(uint& wire, double& time, double& gain);
  void AddPadSignal(std::pair<int,int>& pad, double& time, double& gain, double& z);

  std::vector<int>* GetAnodeSignal(uint aw);
  std::map<uint,std::vector<int>*>* GetAnodeSignal();

  std::vector<int>* GetPadSignal(std::pair<int,int> pad);
  std::map<std::pair<int,int>,std::vector<int>*>* GetPadSignal();

  std::map<std::pair<int,int>,std::vector<int>*>* GetZsPadSignal();

  inline void SetBinWidth(double w)  { fBinWidth = w; }
  inline void SetNumberOfBins(int n) { fNbins = n; }
  inline double GetBinWith() const   { return fBinWidth; } 
  inline int GetNumberOfBins() const { return fNbins; }
};
