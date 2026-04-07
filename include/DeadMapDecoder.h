#include "ITSGeometry.h"
#pragma once

#pragma cling add_include_path(".")
#pragma cling add_include_path("../include")


#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
#include <algorithm>

#include <map>
#include <vector>

#include <TBufferJSON.h>
#include <TH1.h>
#include <TH2.h>
#include <THnSparse.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TVector2.h>
#include <TH2Poly.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TMath.h>
#include "CommonConstants/LHCConstants.h"
//#include <nlohmann/json.hpp>

//#include "Logger.h"

#include "DataFormatsITSMFT/NoiseMap.h"
#include "DataFormatsITSMFT/TimeDeadMap.h"
#include "CCDB/BasicCCDBManager.h"




#include "ITSGeometry.h"



using namespace TMath;


struct DeadStave {
    long Begin = 0;
    long End = 0;
    int Stave;
    DeadStave(long Begin, long End, int Stave): Begin(Begin), End(End), Stave(Stave) {};


};


class DeadMapDecoder {
public:
    DeadMapDecoder(const ITSGeometry& geo);

    std::vector<uint16_t> expandvector(std::vector<uint16_t> words, std::string version, string opt);    
    std::vector<DeadStave> analysis(int RunNumber, string path);

    private:

    const ITSGeometry& mGeo;
    bool acceptSingleChips = true;    

    };