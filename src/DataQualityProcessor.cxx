//#include <string>
//#include <iostream>
//#include <time.h>
//#include <algorithm>
//#include "QualityControl/DatabaseFactory.h"
//#include "QualityControl/CcdbDatabase.h"
//#include "QualityControl/MonitorObject.h"
//#include "CCDB/CcdbApi.h"
//#include <TBufferJSON.h>
//#include <TH2.h>
//#include <THnSparse.h>
//#include <TFile.h>
//#include <TTree.h>
//#include <TSystem.h>


#include "DataQualityProcessor.h"
using namespace std;



std::vector <string> DataQualityProcessor::getRuns (string path){

    std::ifstream infile(path.c_str());
    std::vector <string> out;
    string in,str;


    while(std::getline(infile, str)){    
     str.erase(str.find_last_not_of(" \t\r\n") + 1);
     out.push_back(str);
   }
       std::cout<<"From file: "<< path << " we got " << out.size() << " runs" <<std::endl;
    return out;

}







void DataQualityProcessor::process_run(string inputRun, string path, string pass){


  
  vector<DeadStave> ShutDowns = myDecoder->analysis(stol(inputRun.c_str()),path);

  std::cout<<"Number of shutdowns for run "<< inputRun << ": "<< ShutDowns.size() <<std::endl;
  
  if (ShutDowns.size()==0) return;
string file_name = path + "/fout_mw_" + inputRun + ".root";
TFile *fOut = new TFile( file_name.c_str() ,"RECREATE");



  for (auto object: inputs){
	vector<string> timestamp_map = myCCDBServer->getTimeStamps(object.first,inputRun.c_str(),pass.c_str());
  std::cout<<"For object: "<< object.first << "with pass: "<< pass <<  " we got " << timestamp_map.size() << " timestamps"<<std::endl;
  int iTimestamp = 0;
  int previous_timestamp_state= 0;


	for (string timestamp: timestamp_map){
            long ts = -1;
            try {
                 ts = stol(timestamp);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid timestamp: " << timestamp << "\n";
                continue;
            }

            iTimestamp++;
            int stave=-1;
            previous_timestamp_state=0;
            for (auto DeadStave: ShutDowns){
              if (stave!=DeadStave.Stave) previous_timestamp_state=0; //resetting state for new stave
              //cout<<" Duration (min):"<<  (DeadStave.End - DeadStave.Begin)/(60*1000.) <<"From : "<< DeadStave.Begin << " to "<< DeadStave.End << " timestamp: "<< ts << "("<< ts-DeadStave.Begin << ";"<< DeadStave.End-ts<<") DeadStave.Stave= "<< DeadStave.Stave<<endl;
              
              bool PeriodFullyIncluded = ts > DeadStave.Begin && ts < DeadStave.End;
              
              int isEarly =ts-DeadStave.Begin <0  ? -1 : 1;
              int isLate = DeadStave.End-ts <0  ? -1 : 1;
              //std::cout<< "isEarly= "<<isEarly << "  isLate: "<< isLate << " previous_timestamp_state "<< previous_timestamp_state <<std::endl;
              bool PeriodPartiallyIncluded = (previous_timestamp_state== 2) && (isEarly - isLate==-2);


              previous_timestamp_state = isEarly - isLate;
              if (PeriodFullyIncluded || PeriodPartiallyIncluded){                      
                    stave = DeadStave.Stave;

                    break;
              }         
            }

string status = (stave == -1) ? "GOOD" : "BAD";           
           
if (stave == -1) {

    if (iTimestamp % 30 != 0) continue;
} else {
    int layer = myGeo->StaveToLayer(stave);
    int found = object.first.find("Layer");
    if (found != std::string::npos) {
        int layerInName = object.first[found+5] - '0';
        if (layerInName != layer) continue;
    }
}


            cout<<"getting object: "<< object.first<< " Run: "<< inputRun << " timestamp: "<< timestamp<< " status: "<< status<< "stave: "<< stave << endl;
            string obj_type = object.second;

            TH1*  obj = (TH1*) (myCCDBServer->downloadObject(inputRun.c_str(), pass.c_str(), timestamp, object.first, obj_type ));
                       if (obj == NULL) {
                           cout<<"File not found!"<<endl;
                           continue;
                       }
            obj->SetDirectory(nullptr);
		        string title = obj->GetTitle()  + status;

          

          string name = "run" + inputRun + "_" + obj->GetName() + "_" + timestamp + "_stave" + to_string(stave) + "_" + status; 
          if (obj_type=="TH1"){

		          TH1D* obj_write = new TH1D(name.c_str(),title.c_str(),obj->GetNbinsX(), obj->GetXaxis()->GetXmin(), obj->GetXaxis()->GetXmax());
			        for (int ix=1;ix<=obj->GetNbinsX();ix++){
			              obj_write->SetBinContent(ix,obj->GetBinContent(ix));
			        }
			        fOut->cd();
              obj_write -> Write();
                delete obj_write;

		      } else{
             TH2D* obj_write = new TH2D(name.c_str(),title.c_str() ,obj->GetNbinsX(), obj->GetXaxis()->GetXmin(), obj->GetXaxis()->GetXmax(), obj->GetNbinsY(),obj->GetYaxis()->GetXmin(), obj->GetYaxis()->GetXmax());
             for (int ix=1;ix<=obj->GetNbinsX();ix++){
		              for (int iy=1;iy<=obj->GetNbinsY();iy++)		  
                              obj_write->SetBinContent(ix,iy,obj->GetBinContent(ix,iy));

                          }
              fOut->cd();
              obj_write -> Write(); 
                delete obj_write;

          }


	     }
  }
  fOut->cd();
  fOut->Close();
  delete fOut;



}



void DataQualityProcessor::process_period(Period input){

  string path = outputPath + "/" + input.collision_system + "/" + to_string(input.energy);
  gSystem->mkdir(path.c_str(),kTRUE);
  string full_name = inputPath + "/" + input.name + ".txt";
  vector <string> runs = getRuns(full_name); //Todo: make it more flexible, e.g. pass as argument, or read from config file
  for (auto inputRun: runs) process_run(inputRun,path,input.pass);
}



int DataQualityProcessor::download_runs_mw(){

    std::cout<<"Downloading runs from path: "<< inputPath<<std::endl;
    std::ifstream infile(inputPath+"/Periods.txt"); //Todo: make it more flexible, e.g. pass as argument, or read from config file

    vector <Period> vPeriods;
    string str;
   
   while(std::getline(infile, str)){
     vPeriods.emplace_back(str);
   }

   for (auto period:vPeriods){
    std::cout<<"Processing period: "<< period.name << " collision system: "<< period.collision_system << " energy: "<< period.energy << " pass: "<< period.pass <<std::endl;
      process_period(period);
   }
  return 0;
}