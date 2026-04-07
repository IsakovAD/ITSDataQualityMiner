#include "DataQualityProcessor.h"
#include "CCDBServer.h"
#include "ITSGeometry.h"





int main(){

  std::cout<<"Starting data quality miner..."<<std::endl;
  DataQualityProcessor myProcessor("qc_async", "ali-qcdb-gpn.cern.ch:8083","none");

  int res = myProcessor.download_runs_mw("runs_ML.txt");
  std::cout<<"Finished processing runs, result: "<< res <<std::endl;
  return res;
}

