#ifndef _HISTO_STACKER_
#define _HISTO_STACKER_

#include "TH1D.h"
#include "AlphaColourWheel.h"
#include "TLegend.h"
#include "TCanvas.h"

#include <iostream>

class HistoStacker
{
   private:
      AlphaColourWheel fColour;
      std::vector<TH1D*> fHistos;
      TLegend* fLegend;
   public:
      HistoStacker();
      virtual ~HistoStacker();
      void Clear()
      {
         fColour.Reset();
         fHistos.clear();
         delete fLegend;
         fLegend = new TLegend();
      }

      void AddHisto(TH1D* h)
      {
         fHistos.push_back(h);
         std::cout<< fHistos.size() << " histograms held"<<std::endl;
      }

      void AddHisto(const std::vector<TH1D*> hh)
      {
         for (TH1D* h: hh)
            AddHisto(h);
      }

      void AddHisto(const std::vector<std::vector<TH1D*>> hhh)
      {
         for (const std::vector<TH1D*>& hh: hhh)
            AddHisto(hh);
      }
      
      void AddHisto(TCanvas* c)
      {
         TList* list = c->GetListOfPrimitives();
         int histos_added = 0;
         for (int i = 0; i < list->GetSize(); i++)
         {
            TObject* o = list->At(i);
            if (strcmp(o->ClassName(),"TH1D") == 0)
            {
               AddHisto((TH1D*) o);
               histos_added++;
            }
         }
         std::cout<<histos_added<<" histograms added"<<std::endl;
      }

      TCanvas* DrawStacked()
      {
         TCanvas* c = new TCanvas();
         double max = 0;
         TH1D* tallest_plot;
         for (TH1D* h: fHistos)
         {
            if (max < h->GetMaximum())
            {
               max = h->GetMaximum();
               tallest_plot = h;
            }
            h->SetLineColor(fColour.GetCurrentColour());
            fLegend->AddEntry(h);
            fColour.GetNewColour();
         }
         //Draw the tallest plot first
         tallest_plot->Draw();
         for (TH1D* h: fHistos)
         {
            if (h != tallest_plot)
               h->Draw("SAME");
         }
         fLegend->Draw();
         c->Draw();
         return c;
      }
};


#endif
