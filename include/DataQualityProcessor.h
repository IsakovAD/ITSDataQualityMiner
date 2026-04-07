#pragma once
#include "TPDF.h"
#include <fstream>
#include <vector>
#include <string>   

#include "DeadMapDecoder.h"
#include "ITSGeometry.h"
#include "CCDBServer.h"



struct Period {
  std::string name;
  float energy;
  std::string collision_system;
  std::string pass;
  Period(std::string input){

        std::stringstream ss(input);
        std::string token;

        std::getline(ss, name, ',');
        std::getline(ss, collision_system, ',');
        std::getline(ss, token, ',');
        energy = stof(token);
        std::getline(ss, pass, ',');

  }
};



class DataQualityProcessor{

    public:
    DataQualityProcessor(std::string dataBaseType = "qc_async", std::string ccdb_port = "ali-qcdb-gpn.cern.ch:8083", std::string apass = "none", std::string in_path = "../inputs", std::string out_path = "../downloads"){


        std::cout<<"Initializing DataQualityProcessor with db type: "<< dataBaseType << " and port: "<< ccdb_port <<std::endl;
        myGeo =  std::make_unique<ITSGeometry>(); 
        myDecoder =  std::make_unique<DeadMapDecoder>(*myGeo);
        myCCDBServer = std::make_unique<CCDBServer>(dataBaseType, ccdb_port, apass);
        inputPath=in_path;
        outputPath=out_path;

        std::cout<<"Initialization complete."<<std::endl;
    };
    int download_runs_mw();
    std::vector <std::string> getRuns (std::string path);


    private: 
    void process_run(std::string inputRun, std::string path, std::string pass);
    void process_period(Period input);
    std::string inputPath;
    std::string outputPath;  


    
std::unique_ptr<DeadMapDecoder>  myDecoder;
std::unique_ptr<ITSGeometry> myGeo;
std::unique_ptr<CCDBServer> myCCDBServer;

std::vector <std::pair<std::string,std::string>> inputs= {

	  {"/ITS/MO/Clusters/mw/Layer0/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer1/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer2/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer3/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer4/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer5/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer6/ClusterOccupation", "TH2"},
     {"/ITS/MO/Tracks/mw/AngularDistribution", "TH2"},
     {"/ITS/MO/Tracks/mw/NClusters", "TH1"},
     {"/ITS/MO/Tracks/mw/Ntracks", "TH1"},
     {"/ITS/MO/ITSTrackTask/NClustersPerTrackEta","TH2"}   //how will it work for non mw object?
};





};