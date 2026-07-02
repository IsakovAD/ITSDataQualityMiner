#include "CCDBServer.h"

#include <TH1D.h>
#include <TEfficiency.h>

int main(){


    std::cout<<"Creating new server: "<< std::endl;
    CCDBServer *myServer = new CCDBServer ("qc_async", "ali-qcdb-gpn.cern.ch:8083","apass1");
    //CCDBServer *myServer = new CCDBServer ("qc_mc", "ali-qcdbmc-gpn.cern.ch:8083","passMC"); //should it be just type/link?



    // std::string RunNumber = "544028";
    // //std::string PassName = "passMC";
    // std::string PassName = "cpass0";
    // std::string fullPath = "/ITS/MO/Tracks/Ntracks";
    // std::string ObjectType = "TH1";
    // std::string module = "tracks";

//ITS-QA-QC:
// getting timestamp for: RunNumber= 563751 PassName= apass1 module_name= Tracks
// Not found: run=563751 apass=apass1 path=Tracks
// Downloading object with timestamp: -1


    //ali-qcdb-gpn.cern.ch:8083/qc_async/ITS/MO/Tracks/NClusters/1780762708787/PassName=apass1/RunNumber=563751/
    //ali-qcdb-gpn.cern.ch:8083/qc_async/ITS/MO/Tracks/Ntracks
    std::string RunNumber = "563751";
    // std::string PassName = "passMC";
    std::string PassName = "apass1";
    std::string fullPath = "/ITS/MO/Tracks/Ntracks";
    std::string ObjectType = "TH1";
    std::string periodName = "";
    std::string module = "Tracks";




    std::cout<<"Dowloading object: "<< fullPath<<std::endl;
    TH1D* hTest = (TH1D*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType, module,periodName);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest->Integral()<<std::endl;

    return -1;

             
    fullPath = "/ITS/MO/Clusters/Layer0/ClusterOccupation";
    ObjectType = "TH2";
    module = "clusters";

    std::cout<<"Dowloading obeject: "<< fullPath<<std::endl;
    TH2D* hTest_2 = (TH2D*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType,module,periodName);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest_2->Integral()<<std::endl;

    fullPath = "/ITS/MO/TracksMc/efficiency_phi";
    ObjectType = "TEfficiency";
    module = "simulation";

    std::cout<<"Dowloading object: "<< fullPath<<std::endl;
    TEfficiency* hTest_3 = (TEfficiency*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType, module,periodName);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest_3->GetName()<<std::endl;





    return 0;
}