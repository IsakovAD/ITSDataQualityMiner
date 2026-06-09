#pragma once


#include "CCDBServer.h"
#include "simple_json_parser.h"
#include <TCanvas.h>
#include <TStyle.h>

#include "Helpers.h"
#include "QAObject.h"

#include<TH2.h>
#include<TH1.h>

#include "ITSGeometry.h"
#include "PDFBuilder.h"

#include <filesystem>
#include <fstream>


class AssyncProcessor{
    public:
        AssyncProcessor(const string& json_file_path):mReportFile(){  
            std::cout<<"parsing params:"<<std::endl;
            parse_parameters(json_file_path);
            std::cout<<"preparing folders:"<<std::endl;
            PrepareOutputFolders();
            std::cout<<"setting style:"<<std::endl;

            setStyle();
            std::cout<<"Getting runs:"<<std::endl;


            runs = getRuns("inputs/its-qa-qc/"+data_path);
            
            mReportFile.open(folder_name+"/Report.txt");
            if (!mReportFile.is_open())
             throw std::runtime_error("Failed to open report file in:"+folder_name+"/Report.txt");
            std::cout<<"We have: "<< runs.size() << " runs!"<<std::endl;

        }   
        int StartQA();

	private:
        string folder_name;
        string outname;
        std::streambuf *original_cout_buffer;
        vector<string> runs;
        std::ofstream mReportFile;

        void PrepareOutputFolders();

        string data_path, Data1Type, Data2Type, Data1Pass, Data2Pass, MCPeriodName1, MCPeriodName2;
		void parse_parameters(const string& json_file_path);
		vector <string> getRuns (const string& path) const;
		vector<QA_object> readObjects(const string& file_name);
        string doCompare(TH1* hNew, TH1* hOld, double ratio_thr, bool isCentralBarrelCut); 
		TH2D* produceAverageClusterPlot(const CCDBServer& server, const TString& run, QA_object object );

};



