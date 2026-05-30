#include "CCDBServer.h"

#include <TH1D.h>
#include <TEfficiency.h>

int main(){


    std::cout<<"Creating new server: "<< std::endl;
    //CCDBServer *myServer = new CCDBServer ("qc_async", "ali-qcdb-gpn.cern.ch:8083","apass0");
    CCDBServer *myServer = new CCDBServer ("qc_mc", "ali-qcdbmc-gpn.cern.ch:8083","passMC"); //should it be just type/link?

    std::string RunNumber = "544028";
    std::string PassName = "passMC";
    std::string fullPath = "/ITS/MO/Tracks/Ntracks";
    std::string ObjectType = "TH1";
    std::string module = "tracks";



    std::cout<<"Dowloading object: "<< fullPath<<std::endl;
    TH1D* hTest = (TH1D*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType, module);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest->Integral()<<std::endl;


             
    fullPath = "/ITS/MO/Clusters/Layer0/ClusterOccupation";
    ObjectType = "TH2";
    module = "clusters";

    std::cout<<"Dowloading obeject: "<< fullPath<<std::endl;
    TH2D* hTest_2 = (TH2D*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType,module);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest_2->Integral()<<std::endl;

    fullPath = "/ITS/MO/TracksMc/efficiency_phi";
    ObjectType = "TEfficiency";
    module = "simulation";

    std::cout<<"Dowloading object: "<< fullPath<<std::endl;
    TEfficiency* hTest_3 = (TEfficiency*) myServer->downloadObject_db(RunNumber, PassName, fullPath,ObjectType, module);
    std::cout<<"Done! Downloaded Object "<<fullPath <<" has integral: "<< hTest_3->GetName()<<std::endl;





    return 0;
}