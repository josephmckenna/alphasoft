//==============================================================================
// File:        PDeconvWindow.h
//
// Description: Window to display waveform data
//
// Revisions:   2012/03/06 - PH Created
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __DeconvWindow_h__
#define __DeconvWindow_h__

#include <Xm/Xm.h>
#include "PImageWindow.h"
#include "PListener.h"
#include "PMenu.h"
#include "ImageData.h"

class PHistImage;

class PDeconvWindow : public PImageWindow, public PListener, public PMenuHandler {
public:
    PDeconvWindow(ImageData *data);
    virtual ~PDeconvWindow();
    
    virtual void    UpdateSelf();
    virtual void    Listen(int message, void *message_data);
    virtual void    DoMenuCommand(int anID);
    virtual void    ScrollValueChanged(EScrollBar bar, int value);

private:
    void            SetChannels(int chan_mask);
    
    Widget          mChannel[kMaxWaveformChannels]; // channel canvas widgets
    PHistImage    * mHist[kMaxWaveformChannels];
    int             mLastNum;                       // hit number of last display waveforms
    int             mChanMask;                      // channels shown
};


#endif // __DeconvWindow_h__
