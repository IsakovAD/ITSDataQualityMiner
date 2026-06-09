#include "CCDBServer.h"

#include <TH1D.h>
#include <TEfficiency.h>


#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <filesystem>
#include "AssyncProcessor.h"


std::string inputPath;
std::string outputPath;
using namespace std;


int ReadParams(int argc, char** argv);


int main(int argc, char** argv){
        ReadParams(argc, argv);

        AssyncProcessor processor(inputPath);
        std::cout<<"Starting QA"<<std::endl;
        processor.StartQA();
        std::cout<<"Done! exiting"<<std::endl;

        //TO-DO: add tracking of faulty cases
        return 0;
};







int ReadParams(int argc, char** argv){
    po::options_description desc("Quality Assurance tool for ALICE ITS Assync Data");
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
    inputPath = vm["input"].as<std::string>();
    outputPath = vm["output"].as<std::string>();

    return 0;
}
