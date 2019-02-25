#ifndef __TWaveform__
#define __TWaveform__

#include <TObject.h>
#include <vector>
#include <string>

class TWaveform: public TObject
{
private: 
  std::string fName;
  std::string fModel;
  //  std::vector<double> fWaveform;
  std::vector<int> fWaveform;

public:
  TWaveform() {};
  // TWaveform(std::string, std::vector<double>, std::string);
  // TWaveform(std::string, std::vector<double>);
  TWaveform(std::string, std::vector<int>, std::string);
  TWaveform(std::string, std::vector<int>);
  
  inline std::string GetElectrode() { return fName; }
  inline std::string GetModel() { return fModel; }
  //  inline std::vector<double> GetWaveform() { return fWaveform; }
  //  inline double GetWaveformAt(uint i) { return fWaveform.at(i); }
  inline std::vector<int> GetWaveform() { return fWaveform; }
  inline int GetWaveformAt(uint i) { return fWaveform.at(i); }

  inline void SetElectrode( std::string n ) { fName = n; }
  inline void SetElectrode( char* n ) { fName = n; }
  inline void SetModel( std::string m ) { fModel = m; }
  inline void SetModel( char* m ) { fModel = m; }

  // inline void SetWaveform( std::vector<double> wf ) { fWaveform = wf; }
  // inline void SetWaveformAt( uint i, double v) { fWaveform[i] = v; }
  // inline void PushBack(double v) { fWaveform.push_back(v); }
  inline void SetWaveform( std::vector<int> wf ) { fWaveform = wf; }
  inline void SetWaveformAt( uint i, int v) { fWaveform[i] = v; }
  inline void PushBack(int v) { fWaveform.push_back(v); }

ClassDef(TWaveform,1)
};

#endif
