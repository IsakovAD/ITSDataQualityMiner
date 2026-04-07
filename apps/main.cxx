#include "DataQualityProcessor.h"
#include "CCDBServer.h"
#include "ITSGeometry.h"
#include <boost/program_options.hpp>
#include <iostream>
namespace po = boost::program_options;

using namespace std;


int main(int argc, char** argv) {
    
    po::options_description desc("ITS Data Quality Miner");
    desc.add_options()
        ("help,h",                                          "show help")
        ("input,i",   po::value<std::string>()->required(), "path to periods file")
        ("output,o",  po::value<std::string>()->default_value("../downloads"), "output directory");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        
        po::notify(vm);  
        
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << "\n\n" << desc << "\n";
        return 1;
    }

    // Access arguments:
    std::string inputPath = vm["input"].as<std::string>();
    std::string outputPath = vm["output"].as<std::string>();

    std::cout << "Input:  " << inputPath  << "\n";
    std::cout << "Output: " << outputPath << "\n";

    DataQualityProcessor myProcessor("qc_async", "ali-qcdb-gpn.cern.ch:8083","none",inputPath,outputPath);
    int res = myProcessor.download_runs_mw();
    return res;
}


