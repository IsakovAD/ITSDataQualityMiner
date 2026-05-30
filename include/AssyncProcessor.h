#pragma once

#include"simple_json_parser.h"




#include <TCanvas.h>
#include <TStyle.h>

#include "Helpers.h"
#include "QAObject.h"

#include<TH2.h>
#include<TH1.h>

#include "ITSGeometry.h"
#include "CCDBServer.h"


class AssyncProcessor{
    public:
        AssyncProcessor(const string& json_file_path){  
            parse_parameters(json_file_path);
            PrepareOutputFolders();
            setStyle();

            runs = getRuns("input/"+data_path);

        }   
        int StartQA();

	private:
        string folder_name;
        std::streambuf *original_cout_buffer;
        vector<string> runs;

        void PrepareOutputFolders();

        string data_path, Data1Type, Data2Type, Data1Pass, Data2Pass, PeriodName1, PeriodName2;
		void parse_parameters(const string& json_file_path);
		vector <string> getRuns (const string& path) const;
		vector<QA_object> readObjects(const string& file_name);
		TH2D* produceAverageClusterPlot(const CCDBServer& server, const TString& run, QA_object object );

};






}