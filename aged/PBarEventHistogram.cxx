//==============================================================================
// File:        PBarEventHistogram.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
/*
** class PBarEventHistogram
**
** This is a very special PHistImage object because the histogram
** scale for this object manifests itself as the hit colour for all
** image windows.  As such, some of these methods are declared static
** to allow calculation of the histogram scale even in the absence
** of an instantiation of this class.
*/
#include <math.h>
#include "ImageData.h"
#include "PBarEventHistogram.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PScale.h"
#include "PUtils.h"
#include "AgedWindow.h"
#include "menu.h"

#define MIN_HIST_RANGE              5
#define MIN_HIST_BINS               5

float   PBarEventHistogram::sMaxCalTime    = 1000.0;
float   PBarEventHistogram::sMaxCalCharge  = 10000.0;
float   PBarEventHistogram::sMinRangeFloat = 0.01;
int     PBarEventHistogram::sIsAutoScale   = 1;


//---------------------------------------------------------------------------------------
// PEventHistogram constructor
//
PBarEventHistogram::PBarEventHistogram(PImageWindow *owner, Widget canvas)
               : PHistImage(owner,canvas)
{
    ImageData *data = owner->GetData();
    
    mNumCols = data->num_cols;
    mHistCols = new int[mNumCols];
    mOverlayCol[0] = NUM_COLOURS + mNumCols;
    mIsLog = data->log_scale;   // restore log scale setting
    if (!mHistCols) quit("No memory for allocating colour array!");
    for (int i=0; i<mNumCols; ++i) {
        mHistCols[i] = NUM_COLOURS + i;
    }
}

PBarEventHistogram::~PBarEventHistogram()
{
    if (mGrabFlag) {
        ResetGrab(1);
    }
}

void PBarEventHistogram::Listen(int message, void *dataPt)
{
    switch (message) {
        case kMessageNewEvent:
        case kMessageEventCleared:
        case kMessageColoursChanged:
        case kMessageHitsChanged:
            SetDirty();
            break;
        default:
            PImageCanvas::Listen(message, dataPt);
            break;
    }
}

PHistImage* PBarEventHistogram::GetEventHistogram(ImageData *data)
{
    if (data->mWindow[BAR_HIST_WINDOW]) {
        return (PHistImage *)((PImageWindow *)data->mWindow[BAR_HIST_WINDOW])->GetImage();
    } else {
        return NULL;
    }
}

/*
** Make histogram and set x and y scale ranges
*/
void PBarEventHistogram::MakeHistogram()
{
    ImageData   *data = mOwner->GetData();
    int         i,n,num,slab;
    long        max;
    BarInfo     *bi  = data->barhits.bar_info;
    long        bit_mask;
    long        nbin = data->hist_bins;
    float       val, first, last, range;
    int         incr;
    
    mUnderscale = 0;
    mOverscale = 0;
    
    // set the X scale to integer if the data type is integer
    SetIntegerXScale(isIntegerDataType(data));
    
    /* get histogram bins */
    nbin = GetBins(data,&first, &last);
    range = last - first;
/*
** Calculate and draw histogram
*/
    if (mHistogram && nbin!=mNumBins) {
        delete [] mHistogram;
        ClearOverlays();
        mHistogram = NULL;
    }
    if (!mHistogram || !mOverlay[0]) {
        if (mHistogram) delete [] mHistogram;
        if (mOverlay[0]) delete [] mOverlay[0];
        // allocate histogram and overlay arrays
        mHistogram = new long[nbin];
        mOverlay[0] = new long[nbin];
        if (!mHistogram || !mOverlay[0]) {
            Printf("Out of memory for histogram\n");
            return;
        }
        mNumBins = nbin;                // set number of bins
        mNumOverlays = 1;
    }
    memset(mHistogram, 0, nbin * sizeof(long));
    memset(mOverlay[0], 0, nbin * sizeof(long));
    
    num = data->barhits.num_nodes;
    max = 0;
    incr = 1;
    
    // decide for ourselves whether the hit is under/overscale
    // because we may be in the process of changing the scale
    bit_mask = data->bit_mask & ~(HIT_UNDERSCALE | HIT_OVERSCALE);
    
    for (i=0; i<num; ++i, ++bi) {
    
        if (bi->flags & bit_mask) continue; /* only consider unmasked hits */

        /* calculate bin number  */
        val = (getHitValPad(data, bi) - first) * nbin / range;
        // convert val to an integral bin number
        if (isnan(val) || val < 0) {
            // ignore underscale hits if masked out
            if (data->bit_mask & HIT_UNDERSCALE) continue;
            n = 0;
            if (!(bi->flags & HIT_DISCARDED)) mUnderscale += incr;
        } else if (val >= nbin) {
            // ignore overscale hits if masked out
            if (data->bit_mask & HIT_OVERSCALE) continue;
            n = nbin - 1;
            if (!(bi->flags & HIT_DISCARDED)) mOverscale += incr;
        } else {
            n = (int)val;
        }
        if ((mHistogram[n] += incr) > max) max = mHistogram[n];
        // keep track of discarded hits in each bin
        if (bi->flags & HIT_DISCARDED) mOverlay[0][n] += incr;
    }
    /* calculate a nice even maximum value for the y axis */
    if (!(mGrabFlag & GRAB_Y)) {
        slab = 5;
        while (1) {
            if (max/slab < 10) break;
            slab *= 2;
            if (max/slab < 10) break;
            slab *= 5;
        }
        mYMax = (max/slab + 1) * slab;
        if (mYMax < 10) mYMax = 10;
        mYMin = 0;
    }
    // save the x scale range
    mXMin = first;
    mXMax = last;
}

/*
** Set histogram label string
*/
void PBarEventHistogram::SetHistogramLabel()
{
    ImageData   *data = mOwner->GetData();

    if (!mLabel) {
        mLabel = new char[128];
        if (!mLabel) return;
    }
    GetHistogramLabel(data, mLabel);
}

void PBarEventHistogram::GetHistogramLabel(ImageData *data, char *buff)
{
    strcpy(buff,data->dispName);
    switch (data->wDataType) {
        case IDM_ERROR:
            strcat(buff," (mm)");
            break;
    }
}

void PBarEventHistogram::DoGrab(float xmin, float xmax)
{
    mXMin = xmin;
    mXMax = xmax;
    CheckScaleRange();
}

// called after a grab is completed
void PBarEventHistogram::DoneGrab()
{
    ImageData *data = mOwner->GetData();
    
    data->log_scale = mIsLog;   // keep log scale setting current
    
    // must re-calculate hit values and redraw images if colour scale changed
    SetBins(data, mXMin, mXMax);

    calcBarVals(data);
    sendMessage(data,kMessageHitsChanged);
}

void PBarEventHistogram::ResetGrab(int do_update)
{
    // let the base class reset the grab flag
    PHistImage::ResetGrab(do_update);
    
    if (do_update) {
        // reset colour scale to defaults if this is an auto-scale
        if (sIsAutoScale) {
            ImageData *data = mOwner->GetData();
            SetBins(data, mXMin, mXMax);
            calcBarVals(data);
            sendMessage(data,kMessageHitsChanged);
        } else {
            // otherwise just redraw the histogram
            SetDirty();
        }
    }
}

/* get histogram bin parameters */
long PBarEventHistogram::GetBins(ImageData *data, float *first_pt, float *last_pt)
{
    long        nbin;
    float       first=0, last=0.;//, range;
    PHistImage  *hist = GetEventHistogram(data);
    
    /* handle manual scales */
    if (hist && (hist->GetGrabFlag() & GRAB_X)) {
        *first_pt = first = hist->GetScaleMin();
        *last_pt  = last  = hist->GetScaleMax();
        nbin = hist->GetNumBins();
        //range = last - first;
        // (recalculate here the number of bins for some data types)
        switch (data->wDataType) {
            case IDM_DISP_BAR:
                nbin = 64;
                break;
        }
        return(nbin);
    }
    nbin = data->hist_bins;
    sIsAutoScale = 1;
    
    /* get histogram scales */
    switch (data->wDataType) {
        case IDM_TIME:
            first = data->bar_time_min;
            last  = data->bar_time_max;
            sIsAutoScale = 1;
            break;
        case IDM_HEIGHT:
            first = data->bar_height_min;
            last  = data->bar_height_max;
            break;
        //case IDM_ERROR:
        //    first = data->error_min;
        //    last  = data->error_max;
        //    break;
        case IDM_DISP_BAR:
            first = 0;
            last = NUM_AG_BARS;
            nbin = NUM_AG_BARS;
            sIsAutoScale = 1;
            break;
    }
    // update current scale limits if the event histogram
    if (hist && (hist->GetScaleMin()!=first || hist->GetScaleMax()!=last)) {
        hist->SetScaleMin(first);
        hist->SetScaleMax(last);
        hist->UpdateScaleInfo();
    }

    *first_pt = first;
    *last_pt  = last;
    
    return(nbin);
}
 

/* set histogram range */
void PBarEventHistogram::SetBins(ImageData *data, float first, float last)
{
    /* set histogram scales */
    switch (data->wDataType) {
        case IDM_TIME:
            data->bar_time_min = (int)first;
            data->bar_time_max = (int)last;
            break;
        case IDM_HEIGHT:
            data->bar_height_min = (int)first;
            data->bar_height_max = (int)last;
            break;
        case IDM_ERROR:
            data->error_min = (int)first;
            data->error_max = (int)last;
            break;
        case IDM_DISP_WIRE:
        case IDM_DISP_PAD:
            break;  // do nothing for now (autoscaling)
    }
}
 
void PBarEventHistogram::SetScaleLimits()
{
    float min, max, min_rng;
    
    GetLimits(mOwner->GetData(), &min, &max, &min_rng);
    
    mXMinMin = min;
    mXMaxMax = max;
    mXMinRng = min_rng;
}

void PBarEventHistogram::SetMaxCalScale(float maxT, float maxQ, float minRng)
{
    sMaxCalTime = maxT;
    sMaxCalCharge = maxQ;
    sMinRangeFloat = minRng;
}

/* get histogram maximum range */
void PBarEventHistogram::GetLimits(ImageData *data,float *min_pt, float *max_pt, float *min_rng)
{
    float       xmin, xmax, rmin;
    
    rmin = MIN_HIST_RANGE;  // default minimum range
    
    /* get histogram scales */
    switch (data->wDataType) {
        case IDM_TIME:
            xmin = -5000;
            xmax = 5000;
            break;
        case IDM_HEIGHT:
            xmin = 0;
            xmax = 20000;
            break;
        case IDM_ERROR:
            xmin = 0;
            xmax = 100;
            rmin = .1;
            break;
        case IDM_DISP_BAR:
            xmin = 0;
            xmax = NUM_AG_BARS;
        default:
            xmin = -1e6;
            xmax = 1e6;
            rmin = sMinRangeFloat;
            break;
    }
    *min_pt = xmin;
    *max_pt = xmax;
    *min_rng = rmin;
}
 



