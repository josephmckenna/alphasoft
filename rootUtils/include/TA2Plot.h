#ifdef BUILD_A2

#ifndef _TALPHA2PLOT_
#define _TALPHA2PLOT_
#include "TCanvas.h"
#include "TLatex.h"
#include "TAPlot.h"
#include "TSISEvent.h"
#include "TSVD_QOD.h"
#include "TSISChannels.h"
#include "TA2Spill.h"
#include "TA2SpillGetters.h"

class TSISPlotEvents: public TObject
{
   public:
      std::vector<int> fRunNumber;
      std::vector<double> fTime;  //Plot time (based off official time)
      std::vector<double> fOfficialTime;
      std::vector<int> fCounts;
      std::vector<int> fSISChannel;
   public:
      //Std ctor and dtor
      TSISPlotEvents()
      {
      }
      ~TSISPlotEvents()
      {
      }
      //Copy ctor - !!!
      TSISPlotEvents(const TSISPlotEvents& sisPlotEvents) : TObject(sisPlotEvents)
      {
         //Deep copy vectors.
         for(size_t i=0; i<sisPlotEvents.fTime.size(); i++)
         {
            fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
            fTime.push_back( sisPlotEvents.fTime[i]);
            fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
            fCounts.push_back( sisPlotEvents.fCounts[i]);
            fSISChannel.push_back( sisPlotEvents.fSISChannel[i]);
         }
      }
      TSISPlotEvents operator+=(const TSISPlotEvents &rhs) 
      {
         //std::cout << "TSISPlotEvents += operator" << std::endl;
         this->fRunNumber   .insert(this->fRunNumber.end(),      rhs.fRunNumber.begin(),      rhs.fRunNumber.end() );
         this->fTime           .insert(this->fTime.end(),              rhs.fTime.begin(),              rhs.fTime.end() );
         this->fOfficialTime.insert(this->fOfficialTime.end(),   rhs.fOfficialTime.begin(),   rhs.fOfficialTime.end() );
         this->fCounts      .insert(this->fCounts.end(),         rhs.fCounts.begin(),         rhs.fCounts.end() );
         this->fSISChannel .insert(this->fSISChannel.end(),    rhs.fSISChannel.begin(),    rhs.fSISChannel.end() );
         return *this;
      }
      TSISPlotEvents& operator=(const TSISPlotEvents& sisPlotEvents)
      {
         for(size_t i = 0; i<sisPlotEvents.fTime.size(); i++)
         {
            this->fRunNumber.push_back( sisPlotEvents.fRunNumber[i]);
            this->fTime.push_back( sisPlotEvents.fTime[i]);
            this->fOfficialTime.push_back( sisPlotEvents.fOfficialTime[i]);
            this->fCounts.push_back( sisPlotEvents.fCounts[i]);
            this->fSISChannel.push_back( sisPlotEvents.fSISChannel[i]);
         }
         return *this;
      }
      friend TSISPlotEvents operator+(const TSISPlotEvents& lhs, const TSISPlotEvents& rhs)
      {
         //std::cout << "TSISPlotEvents addition operator" << std::endl;
         TSISPlotEvents outputplot(lhs); //Create new from copy
         //Vectors- need concacting
         outputplot.fRunNumber.insert(outputplot.fRunNumber.end(), rhs.fRunNumber.begin(), rhs.fRunNumber.end() );
         outputplot.fTime.insert(outputplot.fTime.end(), rhs.fTime.begin(), rhs.fTime.end() );
         outputplot.fOfficialTime.insert(outputplot.fOfficialTime.end(), rhs.fOfficialTime.begin(), rhs.fOfficialTime.end() );
         outputplot.fCounts.insert(outputplot.fCounts.end(), rhs.fCounts.begin(), rhs.fCounts.end() );
         outputplot.fSISChannel.insert(outputplot.fSISChannel.end(), rhs.fSISChannel.begin(), rhs.fSISChannel.end() );
         return outputplot;
      }
      void AddEvent(int runNumber, double time, double officialTime, int counts, int channel)
      {
         fRunNumber.push_back(runNumber);
         fTime.push_back(time);  
         fOfficialTime.push_back(officialTime);
         fCounts.push_back(counts);
         fSISChannel.push_back(channel);
      }
      int GetEventRunNumber(int event) const { return fRunNumber.at(event); }
      
      std::string CSVTitleLine() const
      {
         return std::string("Run Number,") + 
                "Plot Time (Time axis of TAPlot)," +
                "OfficialTime (run time)," +
                "SIS Channel," +
                "Counts\n";
      }

      std::string CSVLine(size_t i) const
      {
         //This is a little fugly
         std::string line;
         line =std::string("") + fRunNumber.at(i) + "," +
                fTime.at(i) + "," +
                fOfficialTime.at(i) + "," +
                fSISChannel.at(i) + "," +
                fCounts.at(i) + "\n";
         return line;
      }

      size_t size() const
      {
         return fRunNumber.size();
      }
      
      int CountTotalCountsInChannel(int ch) const
      {
         int events = 0;
         const size_t range = fCounts.size();
         for(size_t i = 0; i<fTime.size(); i++)
         {
            if (fSISChannel[i] == ch)
               events += fCounts[i];
         }
         return events;
      }
      ClassDef(TSISPlotEvents,1);
};

class TA2Plot: public TAPlot
{
   protected:
      std::vector<int> fSISChannels;

      //Detector SIS channels
      std::map<int, int> fTrig;
      std::map<int, int> fTrigNobusy;
      std::map<int, int> fAtomOr;

      //Dump marker SIS channels:
      std::map<int, int> fCATStart;
      std::map<int, int> fCATStop;
      std::map<int, int> fRCTStart;
      std::map<int, int> fRCTStop;
      std::map<int, int> fATMStart;
      std::map<int, int> fATMStop;
   
      //Beam injection/ ejection markers:
      std::map<int, int> fBeamInjection;
      std::map<int, int> fBeamEjection;
      
      double fZMinCut;
      double fZMaxCut;

   public:
      TSISPlotEvents SISEvents;  
      
      //Default members, operators, and prints.
      TA2Plot(bool zeroTime = true);
      TA2Plot(double zMin, double zMax,bool zeroTime = true);
      TA2Plot(const TAPlot& object);
      TA2Plot(const TA2Plot& object);
      virtual ~TA2Plot();
      friend TA2Plot operator+(const TA2Plot& lhs, const TA2Plot& rhs);
      TA2Plot& operator+=(const TA2Plot& rhs);
      TA2Plot& operator=(const TA2Plot& rhs);
   
      //Setters and getters
      void SetSISChannels(int runNumber);
      
      int GetTrigNoBusyChannel(int runNumber)
      {
         return fTrigNobusy[runNumber];
      }
      int GetTrigChannel(int runNumber)
      {
         return fTrig[runNumber];
      }
      
      //Adding events and dumps
      void AddSVDEvent(TSVD_QOD* SVDEvent);
      void AddSISEvent(TSISEvent* SISEvent);
      void AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition );
      void AddDumpGates(int runNumber, std::vector<TA2Spill> spills );
      //If spills are from one run, it is faster to call the function above
      void AddDumpGates(std::vector<TA2Spill> spills );

      //Load, fill, draw, or save the object      
      void LoadRun(int runNumber, double firstTime, double lastTime);
      void SetUpHistograms();
      void FillHisto(bool applyCuts=true, int mode=0);
      TCanvas* DrawCanvas(const char* name="cVTX",bool applyCuts=true, int mode=0);
      void WriteEventList(std::string fileName, bool append = true);
      void ExportCSV(std::string filename, bool PassedCutOnly = true);
   private:
      void AddEvent(TSISEvent* event, int channel,double timeOffset=0);
      void AddEvent(TSVD_QOD* event,double timeOffset=0);
   
   ClassDef(TA2Plot, 1)
};

#endif
#endif
