#include "CCDBServer.h"
#include <TH1.h>
#include <TH2.h>
#include <TSystem.h>

using namespace std;


TH1* CCDBServer::downloadObject(string RunNumber, string PassName, string timestamp, string fullPath, string ObjectType){

    TH1 *out = nullptr ;
    fullPath = dataBaseType + fullPath;
    
    if (timestamp.size() < 2) return out; 
    std::map<std::string, std::string> metadata;
    metadata["RunNumber"]=RunNumber;
    metadata["PassName"]=PassName;

    if ( ObjectType=="TH2")
         out= ccdbApi.retrieveFromTFileAny<TH2>(fullPath, metadata, stol(timestamp)); 
    else
         out = ccdbApi.retrieveFromTFileAny<TH1>(fullPath, metadata, stol(timestamp));
 
   return out;
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