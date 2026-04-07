#pragma once



#include <string>
using namespace std;

#include "CCDB/CcdbApi.h"
#include<TH1.h>
#include<TH2.h>
#include <TSystem.h>



class CCDBServer{
    public:

    CCDBServer(string dataBaseType, string ccdb_port, string apass) : dataBaseType(dataBaseType), ccdb_port(ccdb_port), apass(apass) {

    ccdbApi.init(ccdb_port);

    };
    ~CCDBServer();  

    TH1* downloadObject(string RunNumber, string PassName, string timestamp, string fullPath, string ObjectType);
    vector<string> getTimeStamps(string objectName, string targetRun, string targetPass);

    private:

   o2::ccdb::CcdbApi ccdbApi;
   string dataBaseType;
   string ccdb_port;
   string apass;



};