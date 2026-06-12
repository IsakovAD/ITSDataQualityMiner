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

#include <vector>
#include <filesystem>
#include <fstream>


class AssyncProcessor{
    public:
        AssyncProcessor(const string& json_file_path, const double ratio_thr_):mReportFile(), ratio_thr(ratio_thr_) {  
            parse_parameters(json_file_path);
            PrepareOutputFolders();

            setStyle();


            runs = getRuns("inputs/its-qa-qc/"+data_path);
            mReportFile.open(folder_name+"/Report.txt");
            if (!mReportFile.is_open())
             throw std::runtime_error("[ERROR][AssyncProcessor] Failed to open report file in:"+folder_name+"/Report.txt");
            std::cout<<"[INFO][AssyncProcessor] Starting analysis with "<< runs.size() << " runs!"<<std::endl;
        }   
        int StartQA();

	private:
        string folder_name;
        string outname;
        std::vector<string> runs;
        std::ofstream mReportFile;
        double ratio_thr = 0.05;

        void PrepareOutputFolders();

        string data_path, Data1Type, Data2Type, Data1Pass, Data2Pass, MCPeriodName1, MCPeriodName2;
		void parse_parameters(const string& json_file_path);
		std::vector <string> getRuns (const string& path) const;
		std::vector<QA_object> readObjects(const string& file_name);
        std::string doCompare(const TH1* hRatio, const double ratio_thr, const bool isCentralBarrelCut); 
		TH2D* produceAverageClusterPlot(const CCDBServer& server, const std::string& run, const QA_object& object );
        TH1*  getAssyncObject (const QA_object& object, const std::string& run, const CCDBServer& server);
        void formatAssyncHistogram(const QA_object& object, TH1* histogram, const long nROFs, const std::string& apass, const std::string& type);
};



