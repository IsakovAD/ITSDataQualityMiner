#include "CCDBServer.h"
#include <TH1.h>
#include <TH2.h>
#include <TSystem.h>

using namespace std;


std::string CCDBServer::GetObjectList(std::string objectName){
    objectName = dataBaseType + objectName; 


    std::string output = ccdbApi.list(objectName.c_str(),false,"text/plain"); 

        std::cout<<"[INFO][CCDBServer] for object name: "<<objectName << " server returned response with length of: "<< output.size()<<std::endl; 

    return output;


}



TH1* CCDBServer::downloadObject(string RunNumber, string PassName, long timestamp, string fullPath, string ObjectType) const{

    TH1 *out = nullptr ;
    
    string qc_db_folder=dataBaseType;
    
    std::cout<<"========== dataBaseType= "<< dataBaseType << std::endl;
    if (dataBaseType=="aQC") qc_db_folder= "qc_async";
    if (dataBaseType=="qc_mc") qc_db_folder = "qc_mc";

    
    
    
    fullPath = qc_db_folder + fullPath;
    


    //TO-DO: check that timestamp is fine
    //if (timestamp.size() < 2) return out; 
    std::map<std::string, std::string> metadata;
    metadata["RunNumber"]=RunNumber;
    metadata["PassName"]=PassName;

   //  if ( ObjectType=="TH2")
   //       out= ccdbApi.retrieveFromTFileAny<TH2>(fullPath, metadata, timestamp); 
   //  else{
   //       std::cout<<"Downloading object with path: "<< fullPath << " and timestamp: "<< timestamp <<std::endl;
   //       out = ccdbApi.retrieveFromTFileAny<TH1>(fullPath, metadata, timestamp);
   //       std::cout<<"Done! Object was Downloaded "<<std::endl;
   //  }

   try {
      if ( ObjectType=="TH2")  out= ccdbApi.retrieveFromTFileAny<TH2>(fullPath, metadata, timestamp);
      else if  (ObjectType=="TEfficiency") {
            TEfficiency *hEff = ccdbApi.retrieveFromTFileAny<TEfficiency>(fullPath, metadata, timestamp);
            if (hEff){
               //[TO-DO]write function to convert TEff to TH1
               TH1 *teff_Num = (TH1*)hEff->GetPassedHistogram();
               teff_Num->Divide(hEff->GetTotalHistogram());
               for(int i=1; i<=teff_Num->GetNbinsX(); i++) {
            teff_Num->SetBinError(i, std::max(hEff->GetEfficiencyErrorLow(i), hEff->GetEfficiencyErrorUp(i)));
         }
               out = (TH1*) teff_Num->Clone("Efficiency");
               out->SetStats(0);
            }
      } else
            out = ccdbApi.retrieveFromTFileAny<TH1>(fullPath, metadata, timestamp);
      } catch (const std::exception& ex){
	   cout<<"[ERROR] Can't download object"<<fullPath  << " database: "<< dataBaseType<< " in Run: "<< RunNumber<<endl;
   
   
   }





   return out;
}

   TH1* CCDBServer::downloadObject(string RunNumber, QA_object object) const{

      //TO-do: probably will not work, check names of tasks and modules 
      



      string ModuleName;

      if (object.Task=="Tracks")
         ModuleName = dataBaseType=="qc" ? "ITSTrackTask" : "Tracks";
      else if (object.Task=="Clusters")
         ModuleName = dataBaseType=="qc" ? "ITSClusterTask" : "Clusters";
      else if ( object.Task.find("Mc") !=std::string::npos)
         ModuleName ="TracksMc";
      else {
         cout<<"[ERROR][downloadObject] Wrong Task name: "<< object.Task << " for object: "<< object.Name << " in database: "<< dataBaseType << "Expected: Tracks, Clusters, TracksMc" <<endl;
         return nullptr;
      }
         std::string fullPath = "/ITS/MO/"+ ModuleName + "/"+ object.Name;

          std::cout<<"[DEBUG] [downloadObject] Getting object with full path: "<<fullPath <<" apass is "<< apass <<std::endl;

         if (ModuleName == "TracksMc") ModuleName = "simulation";
        return downloadObject_db(RunNumber, apass, fullPath, object.ObjectType,ModuleName,periodName);
    }


    TH1* CCDBServer::downloadObject_db(std::string RunNumber, std::string PassName, std::string fullPath, std::string ObjectType, std::string module_name, std::string periodName) const{
        
        //string module_name = "tracks";
        std::cout<<"[DEBUG][CCDBServer] getting timestamp for: RunNumber= "<< RunNumber << " PassName= "<< PassName << " module_name= "<< module_name <<std::endl;
         long timestamp = myDataBase->getTimestamp(RunNumber,PassName,module_name,periodName);
         
         std::cout<<"Downloading object with timestamp: "<<timestamp<<std::endl;
         return downloadObject(RunNumber,PassName,timestamp,fullPath,ObjectType);




    }


long CCDBServer::getNROFs (const string& RunNumber) const{
      long nRofs = -1;

      string ModuleName = dataBaseType=="qc" ? "ITSTrackTask" : "Tracks";
      std::string fullPath = "/ITS/MO/"+ ModuleName + "/AssociatedClusterFraction";


      TH1D *hClustersPerROF = (TH1D*) downloadObject_db(RunNumber, "", fullPath, "TH1", "tracks",periodName);
//          TH1* CCDBServer::downloadObject_db(std::string RunNumber, std::string PassName, std::string fullPath, std::string ObjectType, std::string module_name){


      if (hClustersPerROF != NULL)  nRofs = hClustersPerROF->Integral();
      else cout<<"[ERROR] Problem with receiving number of ROFs for run: "<< RunNumber<<endl;
      return nRofs;
}


vector<string> CCDBServer::getTimeStamps(string objectName, string targetRun, string targetPass){

 objectName = dataBaseType + objectName; 

 vector<string> out_map;
 string objectlist = ccdbApi.list(objectName.c_str(),false,"text/plain");
 stringstream ss(objectlist);
 string word;
 string timestamp, runnumber, pass, periodName;

 std::cout<<" object name to get timestamps: " << objectName <<endl;
 bool isRunCorrect = false, isPassCorrect = false;
 while(ss>>word){

    //  cout<<"word = "<<word <<endl;	 
      if(word=="Validity:"){// take the one related to file creation
         ss>>word;
         timestamp = word;
      }
    if (word=="RunNumber"){
       ss>>word;
       ss>>word;
       runnumber = (string) word;
    }

    if (word=="PassName"){
       ss>>word;
       ss>>word;
       pass = word;
    }
    
    if (word=="PeriodName"){
       ss>>word;
       ss>>word;
      periodName = word;
   
       if (runnumber==targetRun && pass==targetPass ){
	         cout<<"Found Period name "<<periodName  << "pass="<<pass << " timestamp "<< timestamp<< " run: "<< runnumber<< endl;
		      out_map.push_back(timestamp);
       }
    }
  }

 return out_map;

}