//==============================================================================
// File:        Aged.h
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================

class AgAnalysisFlow;
class AgSignalsFlow;
class AgBarEventFlow;
class TBarEvent;
#include "AgFlow.h"
class TARunInfo;
class PWindow;
struct ImageData;

class Aged
{
public:
    Aged();
    ~Aged();
    
    TAFlags* ShowEvent(AgEvent* age, AgAnalysisFlow* anaFlow, AgSignalsFlow* sigFlow, AgBarEventFlow* barFlow, TAFlags* flags, TARunInfo* runinfo);

private:
    ImageData   *fData;
    PWindow     *fWindow;
};
