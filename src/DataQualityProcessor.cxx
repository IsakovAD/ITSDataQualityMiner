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



std::vector <TString> DataQualityProcessor::getRuns (TString path){

    std::ifstream infile(path.Data());
    std::vector <TString> out;
    string in,str;
   
   while(std::getline(infile, str)){
     in = str.substr(0,6);
     //cout<<"pushing: " << in<<endl;
     out.push_back((TString)in);
   }
    return out;

}







void DataQualityProcessor::process_run(TString inputRun, TString path, TString pass){


  
  vector<DeadStave> ShutDowns = myDecoder->analysis(stol(inputRun.Data()),path);
  
  if (ShutDowns.size()==0) return;

    TFile *fOut = new TFile(Form("%s/fout_mw_%s.root",path.Data(),inputRun.Data()),"RECREATE");




  TString status = "";
  for (auto object: inputs){
	vector<string> timestamp_map = myCCDBServer->getTimeStamps(object.first,inputRun.Data(),pass.Data());
  std::cout<<"timestamp_map size: "<< timestamp_map.size()<<endl;
  int iTimestamp = 0;
  int previous_timestamp_state= 0;

  for (string timestamp: timestamp_map) std::cout<<"timestamp: "<< timestamp <<std::endl;

  //return;

	for (string timestamp: timestamp_map){
            iTimestamp++;
            int stave=-1;
            for (auto DeadStave: ShutDowns){
              std::cout<<"------------------- new check"<<std::endl;
              cout<<" Duration (min):"<<  (DeadStave.End - DeadStave.Begin)/(60*1000.) <<"From : "<< DeadStave.Begin << " to "<< DeadStave.End << " timestamp: "<< stol(timestamp) << "("<< stol(timestamp)-DeadStave.Begin << ";"<< DeadStave.End-stol(timestamp)<<") DeadStave.Stave= "<< DeadStave.Stave<<endl;
              
              bool PeriodFullyIncluded = stol(timestamp) > DeadStave.Begin && stol(timestamp) < DeadStave.End;
              
              int isEarly =stol(timestamp)-DeadStave.Begin <0  ? -1 : 1;
              int isLate = DeadStave.End-stol(timestamp) <0  ? -1 : 1;
              std::cout<< "isEarly= "<<isEarly << "  isLate: "<< isLate << " previous_timestamp_state "<< previous_timestamp_state <<std::endl;
              bool PeriodPartiallyIncluded = (previous_timestamp_state== 2) && (isEarly - isLate==-2);
              std::cout<<"PeriodPartiallyIncluded: "<< PeriodPartiallyIncluded <<std::endl;

                  

              previous_timestamp_state = isEarly - isLate;
              if (PeriodFullyIncluded || PeriodPartiallyIncluded){
                    std::cout<<"This timestamp is BAD!"<<std::endl;
                      
                    stave = DeadStave.Stave;
                    
                    int layer = myGeo->StaveToLayer(stave);

                    int found = object.first.find("Layer");
                    if (found!=std::string::npos && object.first[found+5]!=layer) continue; //getting layer number of object name
                    
                    
                    //std::cout<<" object: "<<object.first << "Has layer= "<< object.first[found+5];


                    status = "BAD";
                    break;
              }         
            }
            
            if (stave == -1){
              status = "GOOD";
              if (iTimestamp%5!=0) continue;
                
            }

            cout<<"getting object: "<< object.first<< " Run: "<< inputRun << " timestamp: "<< timestamp<< " status: "<< status<< "stave: "<< stave << endl;
            string obj_type = object.second;

            TH1*  obj = (TH1*) (myCCDBServer->downloadObject(inputRun.Data(), pass.Data(), timestamp, object.first, obj_type ));
                       if (obj == NULL) {
                           cout<<"File not found!"<<endl;
                           continue;
                       }
            obj->SetDirectory(nullptr);
		        TString title = obj->GetTitle()  + status;

          

		      TString name = Form("run%s_%s_%s_stave%d_%s",inputRun.Data(),obj->GetName(),timestamp.c_str(),stave,status.Data());
          if (obj_type=="TH1"){

		          TH1D* obj_write = new TH1D(name,title,obj->GetNbinsX(), obj->GetXaxis()->GetXmin(), obj->GetXaxis()->GetXmax());
			        for (int ix=1;ix<=obj->GetNbinsX();ix++){
			              obj_write->SetBinContent(ix,obj->GetBinContent(ix));
			        }
			        fOut->cd();
              obj_write -> Write();
		      } else{
             TH2D* obj_write = new TH2D(name,title,obj->GetNbinsX(), obj->GetXaxis()->GetXmin(), obj->GetXaxis()->GetXmax(), obj->GetNbinsY(),obj->GetYaxis()->GetXmin(), obj->GetYaxis()->GetXmax());
             for (int ix=1;ix<=obj->GetNbinsX();ix++){
		              for (int iy=1;iy<=obj->GetNbinsY();iy++)		  
                              obj_write->SetBinContent(ix,iy,obj->GetBinContent(ix,iy));

                          }
              fOut->cd();
              obj_write -> Write(); 
          }


	     }
  }
  fOut->cd();
  fOut->Close();


}



void DataQualityProcessor::process_period(Period input){
  TString path = Form("./outputs/%s/%.1f",input.collision_system.c_str(),input.energy);
  gSystem->mkdir(path.Data(),kTRUE);
  vector <TString> runs = getRuns(Form("../inputs/%s.txt",input.name.c_str())); //Todo: make it more flexible, e.g. pass as argument, or read from config file
  for (auto inputRun: runs) process_run(inputRun,path,input.pass);
}



int DataQualityProcessor::download_runs_mw(TString data_path = "runs_ML.txt"){

    std::cout<<"Downloading runs from path: "<< data_path <<std::endl;
    std::ifstream infile("../inputs/Periods.txt"); //Todo: make it more flexible, e.g. pass as argument, or read from config file

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