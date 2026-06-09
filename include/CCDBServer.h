#pragma once



#include <string>

#include "CCDB/CcdbApi.h"
#include<TH1.h>
#include<TH2.h>
#include<TEfficiency.h>
#include <TSystem.h>
#include "CCDBDataBase.h"

#include "QAObject.h" 





struct CCDB_item{
    string timestamp;
    string runnumber;
    string pass;
    string periodName;
    string object;
};


struct QCModule{
    std::string name_;
    std::string target_object_;
    QCModule(std::string name, std::string target_obect): name_(name), target_object_(target_obect){};
    QCModule(){};
};


class CCDBServer{
    public:

    CCDBServer(std::string dataBaseType_, std::string ccdb_port_, std::string apass_) : dataBaseType(dataBaseType_), ccdb_port(ccdb_port_), apass(apass_) {
    ccdbApi.init(ccdb_port_);

    GetModulesList(dataBaseType_);




    std::cout<<"updating data base: "<<std::endl;
    myDataBase = new CCDBDataBase("./inputs/database/db_"+dataBaseType_+".db");
    updateDataBase();

    std::cout<<"========================== created data base with  dataBaseType= "<< dataBaseType_ << " apass= "<<apass_ << " ccdb_port= "<< ccdb_port_<<std::endl; 

    };

    TH1* downloadObject(std::string RunNumber, std::string PassName, long timestamp, std::string fullPath, std::string ObjectType) const;

    TH1* downloadObject_db(std::string RunNumber, std::string PassName, std::string fullPath, std::string ObjectType, std::string module_name) const;

    TH1* downloadObject(string RunNumber, QA_object object) const;


    std::vector<std::string> getTimeStamps(std::string objectName, std::string targetRun, std::string targetPass);

    std::string GetObjectList(std::string target_object);

    long getNROFs (const string& RunNumber) const;

    string getApass() const {return apass;}
    string getPort() const {return ccdb_port;}
    bool getIsMC() const {return dataBaseType=="mc";}       
    
    void GetModulesList(std::string dataBaseType){
            //modules.push_back(QCModule("tracks","/ITS/MO/Tracks/NClustersPerTrackEta")); //To-Do: should be configurable by json
        std::string config_path = "../inputs/db_module.conf";
        std::ifstream file(config_path);
        //std::vector<QCModule> result;
        std::string line, db_type;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;            
            std::istringstream ss(line);
            QCModule module;
            ss >>db_type>> module.name_ >> module.target_object_;
            if (db_type == dataBaseType){
                std::cout<<"======= pushing new module: "<<  module.name_ << ": "<<  module.target_object_  << std::endl;
                modules.push_back(module);
            }
            //if (module.module.empty() || obj.path.empty()) continue;  // skip bad lines
            
        }


    }
    
    void updateDataBase(){
        
        for (auto [module_name,target_object]: modules){
            long timestamp = myDataBase->getNewestTimestamp(module_name);
            std::cout<<"update database for module: "<< module_name <<std::endl;
            updateModule(timestamp,module_name,GetObjectList(target_object));
            //get string stream
        }
    }

    void updateModule(long timestamp_max, std::string module_name, std::string object_list){

        //to-do: write separate function for single item
      std::cout<<"At the update module"<<std::endl;
      std::stringstream ss(object_list); 
      std::string word, runnumber, pass, periodName;
      long timestamp=-1;
      int nInsertions = 0, nParsing = 0;
       while(ss>>word){
                nParsing++;
                //std::cout<<"word ="<< word <<std::endl;
              if(word=="Validity:"){

                std::cout<<"new insertion with: "<< runnumber << " "<< pass << " "<< module_name << " "<< timestamp <<std::endl;
                myDataBase->upsert(runnumber, pass, module_name, timestamp);

                nInsertions++;


                    ss>>word;
                    timestamp = stol(word);
                    if (timestamp<=timestamp_max) {
                        break;
                    }
                }
            if (word=="RunNumber"){ //change to RunNumber if not alice-ccdb.cern ????           
                ss>>word;
                ss>>word;
                runnumber = word;
            }
    
            if (word=="PeriodName"){
                ss>>word;
                ss>>word;
                periodName = word;
            }
            if (word=="PassName"){
                ss>>word;
                ss>>word;
                pass = word;
            }

  }


    std::cout<<"For module_name: "<< module_name << " we filled "<< nInsertions << " timemstamps, finishing..." <<std::endl;



       



    }


    private:

    



   o2::ccdb::CcdbApi ccdbApi;
   std::string dataBaseType;
   std::string ccdb_port;
   std::string apass;
   CCDBDataBase *myDataBase;
   std::vector<QCModule> modules; //


};