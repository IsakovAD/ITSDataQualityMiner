#include "TPDF.h"
#include <fstream>
#include <vector>
#include <string>   

#include "DeadMapDecoder.h"
#include "ITSGeometry.h"
#include "CCDBServer.h"


using namespace std;

struct Period {
  string name;
  float energy;
  string collision_system;
  string pass;
  Period(string input){

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
    DataQualityProcessor(string dataBaseType = "qc_async", string ccdb_port = "ali-qcdb-gpn.cern.ch:8083", string apass = "none", string in_path = "../inputs", string out_path = "../downloads"){


        std::cout<<"Initializing DataQualityProcessor with db type: "<< dataBaseType << " and port: "<< ccdb_port <<std::endl;
        myGeo = new ITSGeometry();
        myDecoder = new DeadMapDecoder(*myGeo);
        myCCDBServer = new CCDBServer(dataBaseType, ccdb_port, apass);
        inputPath=in_path;
        outputPath=out_path;

        std::cout<<"Initialization complete."<<std::endl;
    };
    int download_runs_mw();
    std::vector <string> getRuns (string path);


    private: 
    void process_run(string inputRun, string path, string pass);
    void process_period(Period input);
    string inputPath;
    string outputPath;  


    
DeadMapDecoder *myDecoder;
ITSGeometry *myGeo;
CCDBServer *myCCDBServer;

vector <std::pair<string,string>> inputs= {

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