#pragma once



#include <string>

#include "CCDB/CcdbApi.h"
#include<TH1.h>
#include<TH2.h>
#include <TSystem.h>



class CCDBServer{
    public:

    CCDBServer(std::string dataBaseType, std::string ccdb_port, std::string apass) : dataBaseType(dataBaseType), ccdb_port(ccdb_port), apass(apass) {

    ccdbApi.init(ccdb_port);

    };

    TH1* downloadObject(std::string RunNumber, std::string PassName, std::string timestamp, std::string fullPath, std::string ObjectType);
    std::vector<std::string> getTimeStamps(std::string objectName, std::string targetRun, std::string targetPass);

    private:

   o2::ccdb::CcdbApi ccdbApi;
   std::string dataBaseType;
   std::string ccdb_port;
   std::string apass;



};