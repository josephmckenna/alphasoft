//==============================================================================
// File:        Aged.cxx
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================

#include "Aged.h"
#include "aged_version.h"

#include "CUtils.h"
#include "ImageData.h"
#include "AgedWindow.h"
#include "PEventControlWindow.h"

#include "TSpacePoint.hh"
#include "TStoreEvent.hh"
#include "TStoreHelix.hh" // TEMPORARY

#define AnyModMask          (Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

Aged::Aged()
{
    Printf((char *)"Version " AGED_VERSION "\n");
/*
** Create main window and menus
*/
    fWindow = new AgedWindow(1);
    fData = fWindow->GetData();
}

Aged::~Aged()
{
    delete fWindow;
    fWindow = NULL;
}

// dispatchEvent - dispatch the X event (PH 03/25/00)
// - handles menu accelerator key events internally
static void dispatchEvent(XEvent *event)
{
    const int       kBuffSize = 10;
    char            buff[kBuffSize];
    KeySym          ks;
    XComposeStatus  cs;
    static int      altPressed = 0;
    
    switch (event->type) {
        case KeyPress:
            XLookupString(&event->xkey, buff, kBuffSize, &ks, &cs);
            switch (ks) {
                case XK_Alt_L:
                    altPressed |= 0x01;
                    break;
                case XK_Alt_R:
                    altPressed |= 0x02;
                    break;
                default:
                    // handle as an accelerator if alt is pressed
                    if (event->xkey.state & AnyModMask &&   // any modifier pressed?
                        altPressed &&                       // was it Alt?
                        PWindow::sMainWindow &&             // main window initialized?
                        PWindow::sMainWindow->GetMenu() &&  // main window menu initialized?
                        // do the accelerator
                        PWindow::sMainWindow->GetMenu()->DoAccelerator(ks))
                    {
                        // return now since the event was an accelerator
                        // and we handled it internally
                        return;
                    }
                    break;
            }
            break;
        case KeyRelease:
            XLookupString(&event->xkey, buff, kBuffSize, &ks, &cs);
            switch (ks) {
                case XK_Alt_L:
                    altPressed &= ~0x01;
                    break;
                case XK_Alt_R:
                    altPressed &= ~0x02;
                    break;
            }
            break;
        case ConfigureNotify:
            // check window offset at every configure event
            // - This is odd I know, but it seems to work with different X clients
            // - The problem was that some X clients will move the window after it
            //   is created.  So we monitor the configureNotify messages and capture
            //   the first one to figure out how the client places the windows.
            if (PWindow::sMainWindow &&
                event->xconfigure.window == XtWindow(PWindow::sMainWindow->GetShell()))
            {
                PWindow::sMainWindow->CheckWindowOffset(event->xconfigure.border_width);
            }
            break;
    }
    // give the event to X
    XtDispatchEvent(event);
}

// callback from timer to show the next event
static void do_next(ImageData *data)
{
    if (data->trigger_flag == TRIGGER_CONTINUOUS) {
        data->mNext = 1;
        // generate a ClientMessage event to break us out of waiting in XtNextEvent()
        if (data->mMainWindow && !XPending(data->display)) {
            XClientMessageEvent xev;
            memset(&xev, 0, sizeof(xev));
            xev.type = ClientMessage;
            xev.format = 8;
            XSendEvent(data->display, XtWindow(data->mMainWindow->GetShell()), True, 0, (XEvent *)&xev);
        }
    }
}

#if 1 //TEST
void findWaveforms(TStoreEvent *anEvent, AgSignalsFlow* sigFlow)
{
    const TObjArray *points = anEvent->GetSpacePoints();
    int num = points->GetEntries();
    for (int i=0; i<num; ++i) {
        int found = 0;
        TSpacePoint* spi = (TSpacePoint*)points->At(i);
        if (!sigFlow->AWwf)
           printf("Aged::No AWwf in flow\n");
        else
           for (auto it=sigFlow->AWwf->begin(); it!=sigFlow->AWwf->end(); ++it) {
	  //if (i==0) printf("Wire i=%d s=%d\n",it->i,it->sec);
               if (it->i == spi->GetWire()) {
                   found |= 0x01;
                   break;
               }
           }
        if (!sigFlow->PADwf)
           printf("Aged::No PADwf in flow\n");
        else
           for (auto it=sigFlow->PADwf->begin(); it!=sigFlow->PADwf->end(); ++it) {
              int pad = it->sec+it->i*32;
               if (pad  == spi->GetPad()) {
                   found |= 0x02;
                   break;
               }
           }
        //printf("Spacepoint %d wire=%d pad=%d\n", i,spi->GetWire(),spi->GetPad());
        if (!(found & 0x01)) printf(" - no wire\n");
        if (!(found & 0x02)) printf(" - no pad\n");
    }
}
#endif

// Show ALPHA-g event in the display
TAFlags* Aged::ShowEvent(AgEvent* age, AgAnalysisFlow* anaFlow, AgSignalsFlow* sigFlow, AgBarEventFlow* barFlow, TAFlags* flags, TARunInfo* runinfo)
{
    ImageData *data = fData;

    if (data) clearEvent(data);

    TStoreEvent *anEvent = anaFlow->fEvent;

    if (!anEvent) return flags;

#if 0 //TEST
    findWaveforms(anEvent, sigFlow); //TEST

    const TObjArray *points = anEvent->GetSpacePoints();
    if (points) {
        int num = points->GetEntries();
        printf("...........................................\n");
        for (int i=0; i<num; ++i) {
            TSpacePoint* spi = (TSpacePoint*) points->At(i);
            double x=spi->GetX(),y=spi->GetY(),z=spi->GetZ();
            if (x*x+y*y>100) continue;
            printf("BAD %d) %16lx %16lx %16lx %g %g %g time=%g h=%g r=%g phi=%g wire=%d pad=%d\n",
                i, *(unsigned long *)&x, *(unsigned long *)&y, *(unsigned long *)&z,
                x,y,z,
                spi->GetTime(),spi->GetHeight(),spi->GetR(),spi->GetPhi(),spi->GetWire(),spi->GetPad());
        }
    }
#endif

#if 0 //TEST
    const TObjArray *helices = anEvent->GetHelixArray();
    printf("%d HELICES -----------------\n",helices ? helices->GetEntries() : -1);
    if (helices && helices->GetEntries() > 0) {
        for (int i=0; i<helices->GetEntries(); ++i) {
            TStoreHelix *helix = (TStoreHelix *)helices->At(i);
            printf("Helix %d) c=%g phi=%g d=%g lam=%g xyz=(%g %g %g) p=(%g %g %g) beta=%g %d %d\n", i,
                helix->GetC(), helix->GetPhi0(), helix->GetD(), helix->GetLambda(),
                helix->GetX0(), helix->GetY0(), helix->GetZ0(),
                helix->GetMomentumV().X(),helix->GetMomentumV().Y(),helix->GetMomentumV().Z(),
                helix->GetFBeta(), helix->GetBranch(), helix->GetStatus());
        }
    }
#endif

    if (data) {
        PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
        if (pe_win) {
            pe_win->Show();
        } else {
            data->mMainWindow->CreateWindow(EVT_NUM_WINDOW);
        }
        if (data->trigger_flag == TRIGGER_SINGLE) {
            setTriggerFlag(data,TRIGGER_OFF);
        }

        // copy the space point XYZ positions into our Node array
        const TObjArray *points = anEvent->GetSpacePoints();
        if (points) {
            int num = points->GetEntries();
            Node *node = (Node *)XtMalloc(num*sizeof(Node));
            if (!node) {
                printf("Out of memory!\n");
                return flags;
            }
            data->hits.hit_info  = (HitInfo *)XtMalloc(num*sizeof(HitInfo));
            if (!data->hits.hit_info) {
                printf("Out of memory!\n");
                free(node);
                return flags;
            }
            data->hits.nodes = node;
            data->hits.num_nodes = num;
            memset(data->hits.hit_info, 0, num*sizeof(HitInfo)); 
            memset(node, 0, num*sizeof(Node));
            HitInfo *hi = data->hits.hit_info;
            for (int i=0; i<num; ++i, ++node, ++hi) {
                TSpacePoint* spi = (TSpacePoint*)points->At(i);
                node->x3 = spi->GetX() / AG_SCALE;
                node->y3 = spi->GetY() / AG_SCALE;
                node->z3 = spi->GetZ() / AG_SCALE;
                hi->wire = spi->GetWire();
                hi->pad = spi->GetPad();
                hi->time = spi->GetTime();
                hi->height = spi->GetHeight();
                hi->error[0] = spi->GetErrX();
                hi->error[1] = spi->GetErrY();
                hi->error[2] = spi->GetErrZ();
                hi->index = i;
                if (std::isnan(hi->time)) hi->time = -1;
                if (std::isnan(hi->height)) hi->height = -1;
            }
        }
        
        if (barFlow)
        {
          //barFlow->BarEvent->Print();
          std::vector<BarHit>* bars=barFlow->BarEvent->GetBars();
          if (bars->size())
          {
            int num = bars->size();
            Node *barnode = (Node *)XtMalloc(num*sizeof(Node));
            if (!barnode) {
                printf("Out of memory!\n");
                return flags;
            }
            data->barhits.bar_info  = (BarInfo *)XtMalloc(num*sizeof(BarInfo));
            if (!data->barhits.bar_info) {
                printf("Out of memory!\n");
                free(barnode);
                return flags;
            }
            data->barhits.nodes = barnode;
            data->barhits.num_nodes = num;
            double MeanTDC=0.; int GoodTDC=0;
            memset(data->barhits.bar_info, 0, num*sizeof(BarInfo)); 
            memset(barnode, 0, num*sizeof(Node));
            BarInfo *bi = data->barhits.bar_info;
            for (int i=0; i<num; ++i, ++barnode, ++bi) {
                BarHit bar=bars->at(i);
                double x,y;
                bar.GetXY(x,y);
                barnode->x3 = x;// /AG_SCALE;
                barnode->y3 = y; // /AG_SCALE;
                barnode->z3 = bar.GetTDCZed();// /AG_SCALE;
                bi->ADCtop = bar.GetAmpTop();
                bi->ADCbot = bar.GetAmpBot();
                bi->TDCtop = bar.GetTDCTop();
                bi->TDCbot = bar.GetTDCBot();
                bi->index = bar.GetBar();
                if (fabs(bar.GetTDCZed())<2.) //If TDC Z data in range of TPC
                {
                   double tdc=(bar.GetTDCTop()+bar.GetTDCBot())/2.;
                   //std::cout<<"TDC:"<<tdc<<"  z:"<<bar.GetTDCZed()<<std::endl;
                   MeanTDC+=tdc;
                   GoodTDC++;
                }
            }
            data->barhits.meantdc = MeanTDC/double(GoodTDC);
          }
        }
        /* calculate the hit colour indices */
        calcHitVals(data);
        calcBarVals(data);

        data->agEvent = anEvent;
        data->anaFlow = anaFlow;
        data->sigFlow = sigFlow;
        data->age     = age;
        data->barFlow = barFlow;
        data->run_number = runinfo->fRunNo;
        data->event_id = anEvent->GetEventNumber();
    
        sendMessage(data, kMessageNewEvent);

        if (data->trigger_flag == TRIGGER_CONTINUOUS) {
            long delay = (long)(data->time_interval );
            XtAppAddTimeOut(data->the_app, delay, (XtTimerCallbackProc)do_next, data);
        }
        if (data->trigger_flag == TRIGGER_QUIT) {
             if (!flags) flags=new TAFlags();
            *flags=TAFlag_QUIT;
            return flags;
        }
    }
    // main event loop
    while (data && data->mMainWindow!=NULL && !data->mNext) {
        if (!data->the_app) return flags;
        XEvent theEvent;
        XtAppNextEvent(data->the_app, &theEvent);
        // fast-forward to most recent pointer motion event (avoids
        // falling behind current mouse position if drawing is slow)
        if (theEvent.type == MotionNotify) {
            while (XCheckTypedEvent(data->display, MotionNotify, &theEvent)) { }
        }
        if ( theEvent.type == ClientMessage) //X11 command...
        {
           XClientMessageEvent* e=(XClientMessageEvent*)&theEvent;
           //std::cout<<e->message_type<<std::endl;
           if (e->message_type==303)  //Click on X (close)
           {
              if (!flags) flags=new TAFlags();
             *flags=TAFlag_QUIT; //Call end run
             return flags;
           }
        }

        // dispatch the X event
        dispatchEvent(&theEvent);
        // update windows now if necessary (but only after all X events have been dispatched)
        if (!XPending(data->display)) PWindow::HandleUpdates();
    }
    data->mNext = 0;
    return flags;
}

