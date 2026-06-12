#pragma once



#include <string>

#include "CCDB/CcdbApi.h"
#include<TH1.h>
#include<TH2.h>
#include<TEfficiency.h>
#include <TSystem.h>
#include "CCDBDataBase.h"

#include "QAObject.h" 
#include <memory>




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


    CCDBServer(const std::string& dataBaseType_, const std::string& apass_): CCDBServer(dataBaseType_, getCCDBport(dataBaseType_), apass_) {}

    CCDBServer(std::string dataBaseType_, std::string ccdb_port_, std::string apass_) : dataBaseType(dataBaseType_), ccdb_port(ccdb_port_), apass(apass_) {
        ccdbApi.init(ccdb_port_);
        if (!ccdbApi.isHostReachable()) {
            std::cout<<"[ERORR][CCDBServer] CCDB " << ccdb_port_ <<" could not be reached !"<<std::endl;
            exit(0);
        }
        GetModulesList(dataBaseType_);
        if (modules.size()==0) {
            std::cout<<"[ERORR][CCDBServer] No modules for CCDB were found at ./inputs/database/db_module.conf, check the file"<<std::endl;
            exit(0);

        }
        std::cout<<"[INFO][CCDBServer] ===============  updating data base: ============== "<<std::endl;
        myDataBase = std::make_unique<CCDBDataBase>("./inputs/database/db_"+dataBaseType_+".db");
        if (!myDataBase){
            std::cout<<"[ERORR][CCDBServer] can't load database with timestamps at: " << "./inputs/database/db_"+dataBaseType_+".db, exiting"<<std::endl;
            exit(0);
        }
        updateDataBase();
        std::cout<<"[INFO][CCDBServer] created data base with  dataBaseType= "<< dataBaseType_ << " apass= "<<apass_ << " ccdb_port= "<< ccdb_port_<<std::endl; 

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
    
    static std::string getCCDBport(const std::string& type) {
        return type == "qc_mc" ? "ali-qcdbmc-gpn.cern.ch:8083"
                               : "ali-qcdb-gpn.cern.ch:8083";
    }
    
    void GetModulesList(std::string dataBaseType){
        std::string config_path = "./inputs/database/db_module.conf";
        std::ifstream file(config_path);
        std::string line, db_type;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;            
            std::istringstream ss(line);
            QCModule module;
            ss >>db_type>> module.name_ >> module.target_object_;
            if (db_type == dataBaseType){
                modules.push_back(module);
            }            
        }


    }
    
    void updateDataBase(){
        
        for (auto [module_name,target_object]: modules){
            long timestamp = myDataBase->getNewestTimestamp(module_name);
            std::cout<<"[INFO][CCDBServer] Update database for module: "<< module_name << " with the newest timestamp: "<<timestamp  <<std::endl;
            updateModule(timestamp,module_name,GetObjectList(target_object));
        }
    }

    void updateModule(long timestamp_max, std::string module_name, std::string object_list){

        //to-do: write separate function for single item
      std::stringstream ss(object_list); 
      std::string word="", runnumber="", pass="", periodName="";
      long timestamp=-1, timestamp_created=-1;
      int nInsertions = 0, nParsing = 0;
      int id = 0;
       while(ss>>word){
                nParsing++;
              //expected order of data: Validity (timestamp) -> RunNumber -> Period -> Pass
              //i.e. Each new item starts from Validity word 




              if(word=="Validity:"){

                                //check if element is fresher than newest entry in local db:
                if (timestamp_created>0 && timestamp_created<=timestamp_max) { // true if this item is already in the local db 
                        std::cout<<"[INFO][CCDBServer] Timestamp: "<< timestamp_created << " is smaller or equal to first entry in local db: "<<timestamp_max << " finishing "<<std::endl;
                        break;
                }





                //filling new timestamp to db
                if (timestamp > 0){

                    std::cout<<"[INFO][CCDBServer][UpdateModule] New insertion with: "<< runnumber << " "<< pass << " "<< module_name << " "<< timestamp <<" timestamp_created= "<< timestamp_created <<std::endl;
                    myDataBase->upsert(runnumber, pass, module_name, timestamp,timestamp_created);
                    nInsertions++;
                }
                //processing the new element
                ss>>word;
                timestamp = stol(word); //add check on stol coversion 

                }
            if(word=="Created:"){
                ss>>word;
                timestamp_created = stol(word);
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


    std::cout<<"[INFO][CCDBServer][UpdateModule] For module_name: "<< module_name << " we filled "<< nInsertions << " timemstamps, finishing..." <<std::endl;



       



    }


    private:

    



   o2::ccdb::CcdbApi ccdbApi;
   std::string dataBaseType;
   std::string ccdb_port;
   std::string apass;
   std::unique_ptr<CCDBDataBase> myDataBase;
   std::vector<QCModule> modules; //


};