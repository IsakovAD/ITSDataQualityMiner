#pragma once

#include <TH1.h>
#include <TH2.h>
#include <TText.h>
#include <TStyle.h>
#include "QAObject.h"
#include <string>
#include <TVirtualPad.h> 
#include <iostream>

    const int NLayer = 7;
    const int NStaves[7] = { 12, 16, 20, 24, 30, 42, 48 };

struct BinCoordinates {
    double xMin, yMin;  // yMin only meaningful for TH2
    double xMax, yMax;  // yMax only meaningful for TH2
};

void setStyle();
void setMinMax(TH1 *obj1, TH1 *obj2);
void doROFNormalize(TH1 *obj,const long hROFs);
void FormatHisto(TVirtualPad *c1, TH1 *obj, const QA_object &object);


std::string TokenizePath(const std::string &input, const char &token);
TH1* performRatio(TH1 *obj_old, TH1 *obj_new, bool isDoCentralBarrelCut);
void performRatio(TVirtualPad *c1, TH1 *obj_new, TH1 *obj_old,
                               QA_object object, TString pass_new,
                               TString pass_old);
void PlotHisto(TVirtualPad *c1, TH1 *obj); 
BinCoordinates getMinMaxCoordinates(const TH1* hist);
