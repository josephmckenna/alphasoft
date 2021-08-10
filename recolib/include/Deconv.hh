#ifndef __DECONV__
#define __DECONV__ 1

#include <cassert>
#include <iostream>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "SignalsType.hh"
#include "AnaSettings.hh"

#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TFile.h>

#include "Alpha16.h"
#include "Feam.h"

std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
uint8_t IsAnodeClose(int w1, int w2);
uint8_t Min(uint8_t x, uint8_t y);
bool IsAnodeNeighbour(int w1, int w2, int dist);

class Deconv {
	public:
		Deconv(AnaSettings*,bool);
		std::vector<ALPHAg::signal>* GetSignal() { return fS; }
		std::vector<ALPHAg::signal>* GetPeaks() { return fPeaks; }
		std::vector<ALPHAg::wf_ref>* GetWaveForms()  { return waveforms; }
		void SetTrace(bool t)      { fTrace=t; }
		void SetDiagnostic(bool d) { fDiagnostic=d; }
		void SetDisplay(bool a)    { fAged=a; }
	protected:
		double fMax;
		double fRange;
		double fThres;
		double fPeak;
		bool fDiagnostic;
		int fBinSize;
		bool isalpha16;
		double fDelay;
		unsigned theBin;
		std::vector<double> fResponse;
		std::vector<double> fRescale;
		std::vector<ALPHAg::electrode> fIndex;
		std::vector<ALPHAg::signal>* fPeaks;
		std::vector<ALPHAg::wf_ref>* waveforms;
		std::vector<ALPHAg::signal>* fS;
		double fScale;
		bool fTrace;
		bool fAged;
		AnaSettings* ana_settings;
		int pedestal_length;
		ALPHAg::comp_hist_t wf_comparator;
		std::vector<ALPHAg::wfholder*>* wforder(std::vector<ALPHAg::wfholder*>* subtracted, const unsigned b);

		void ReadResponseFile(const int bin, const int scale, const std::string f_name);
		void ReadRescaleFile(const std::string f_name);
		double CalculatePedestal(std::vector<int>& adc_samples);
		double GetPeakHeight(std::vector<int>& adc_samples, int& i, double& ped);
		double GetPeakTime(std::vector<int>& adc_samples);
		std::vector<ALPHAg::signal>* Deconvolution( std::vector<ALPHAg::wfholder*>* subtracted,
										   bool isanode);
		virtual void Subtract(ALPHAg::wfholder* hist1,std::vector<ALPHAg::wfholder*>* wfmap,const unsigned b, const double ne) = 0;
};

class DeconvPad : public Deconv {
	public:
		DeconvPad(AnaSettings* s, TFile* fout, int run, bool norm, bool diag);
		int FindTimes(const FeamEvent*);
	private:
		std::vector<int> fPadSecMask;
		std::vector<int> fPadRowMask;
		ALPHAg::padmap* pmap;
		static TH2D* hPWBped;
		static TProfile* hPWBped_prox;
		static TH1D* hAvgRMSPad;

		bool MaskPads(short& sec, int& row);
		void Subtract(ALPHAg::wfholder* hist1,std::vector<ALPHAg::wfholder*>* wfmap,const unsigned b, const double ne);
};

class DeconvWire : public Deconv {
	public:
		DeconvWire(AnaSettings* s, TFile* fout, int run, bool norm, bool diag);
		int FindTimes(const Alpha16Event*);
	private:
		std::vector<double> fFactors;//C
		std::vector<int> fMask;
		static TH2D* hADCped;//C
		static TProfile* hADCped_prox;//C
		static TH1D* hAvgRMSTop;//C
		void Subtract(ALPHAg::wfholder* hist1,std::vector<ALPHAg::wfholder*>* wfmap,const unsigned b, const double ne);
		bool MaskWires(int& aw) { return std::find(fMask.begin(), fMask.end(), aw) != fMask.end(); }
};

#endif
