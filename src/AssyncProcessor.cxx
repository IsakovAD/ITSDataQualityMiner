#include "AssyncProcessor.h"



void AssyncProcessor::parse_parameters(const string &json_file_path) {

  auto json_file = parseJSON(json_file_path);
  if (!json_file.empty()) {
    const auto &params = json_file[0];


    //TO-DO: get rid of of 1 and 2 in the names

    data_path = params.at("run_list");
    Data1Type = params.at("DataType_old");
    Data2Type = params.at("DataType_new");
    Data1Pass = params.at("DataPass_old");
    Data2Pass = params.at("DataPass_new");
    
    MCPeriodName1 = params.at("MCPeriod_old");
    MCPeriodName2 = params.at("MCPeriod_new");

    //[TO-DO] error checks on wrong json format;
    std::cout<<"[INFO] [AssyncProcessor] Starting Analysis with: data_path= "<<data_path << " Data Type old: "<< Data1Type<< " Data Type new: "<< Data2Type << " Pass old: "<< Data1Pass << " Pass new: "<<Data2Pass << " MC period old: "<< MCPeriodName1 << " MC period new: "<< MCPeriodName2 <<std::endl; 
  } else {
    std::cout << "[ERROR] [AssyncProcessor] can't open .json with parameters" << std::endl;
    exit(1);
  }
}


std::vector<QA_object> AssyncProcessor::readObjects(const string &file_name) {

  //[to-do] stoi can throw exception
  std::vector<QA_object> vOut;

  auto object_json_file = parseJSON(file_name);

  for (const auto &object : object_json_file) {
    QA_object current_object;
    for (const auto &[key, item] : object) {

     try {
          if (key == "Path")
            current_object.Name = item;
          else if (key == "Task")
            current_object.Task = item;
          else if (key == "ObjectType")
            current_object.ObjectType = item;
          else if (key == "isEnabled")
            current_object.isEnabled = stoi(item);
          else if (key == "isDoROF_norm")
            current_object.isDoROF_norm = stoi(item);
          else if (key == "isLogy")
            current_object.isLogy = stoi(item);
          else if (key == "isLogx")
            current_object.isLogx = stoi(item);
          else if (key == "isCentralBarrelCut")
            current_object.isCentralBarrelCut = stoi(item);  
          else
            std::cout << "[WARNING] [AssyncProcessor] Wrong item in the input object list: " << key << ":"
                << item << std::endl;

      } catch(...){
	      std::cout<<"[WARNING][AssyncProcessor][readObjects] "<< item << "to 6-digit int due to"<<std::endl;
    	  continue;
     } 

    }
    vOut.emplace_back(current_object);
  }
  return vOut;
}

void AssyncProcessor::PrepareOutputFolders(){

  //outname = ((string)data_path).substr(0, data_path.size() - 4);
  outname = std::filesystem::path(data_path).stem();
  folder_name = "output/its-qa-qc/"+(string) outname; //TO-DO: generalize location of the ouput

  if (!std::filesystem::exists(folder_name))
    std::filesystem::create_directories(folder_name);

  
  std::ofstream log_file(folder_name + "/output.log", std::ofstream::out);
  std::cout<<"PrepareOutputFolders is finished!"<<std::endl;

}

TH1* AssyncProcessor::getAssyncObject (const QA_object& object, const std::string& run, const CCDBServer& server){
  TH1* out = nullptr;
  if (object.Name.find("avg") != string::npos) out = produceAverageClusterPlot(server, run, object);
  else out = server.downloadObject(run, object);

  return out;
}

void AssyncProcessor::formatAssyncHistogram(const QA_object& object, TH1* histogram, const long nROFs, const std::string& apass, const std::string& type){


    if ( object.isDoROF_norm && nROFs>0) doROFNormalize(histogram,nROFs);
    if ( object.Name.find("VertexZ") != string::npos) histogram->Rebin(100);

    histogram->SetTitle(Form("%s data: %s", type.c_str(),
                      Data1Pass.size() < 2 ? "online" : apass.c_str()));
}

int AssyncProcessor::StartQA() {


  CCDBServer server_new(Data2Type, Data2Pass,MCPeriodName2);
  CCDBServer server_old(Data1Type, Data1Pass, MCPeriodName1);

  std::vector<QA_object> vObjects_old = readObjects(Form("inputs/its-qa-qc/objects_%s.json", Data1Type.c_str()));
  std::vector<QA_object> vObjects_new = readObjects(Form("inputs/its-qa-qc/objects_%s.json", Data2Type.c_str()));

  PDFBuilder *myPDF = new PDFBuilder(10, folder_name.c_str());

 for (string run : runs) {

    std::cout << "[INFO][QA loop] Processing run: " << run << std::endl;
    myPDF->AddText("run "+ run);
    long nROFs_old = server_old.getNROFs(run);
    long nROFs_new = server_new.getNROFs(run);
    int nEmpty=0, nProblem=0;

    for (QA_object object_new : vObjects_new) {
      if (!object_new.isEnabled)    continue;

      QA_object object_old;
      bool isObjectNamesMatch = true; 
      const auto it = find_if(vObjects_old.begin(), vObjects_old.end(),
                              [&object_new](const QA_object &obj) {
                                return object_new.Name == obj.Name;
                              });
      if (it != vObjects_old.end()) object_old = *it;
      else {
          std::cout<<"[WARNING] [AssyncProcessor] No old object with name: "<<  object_new.Name << " proceeding with only the new object"<<std::endl;
          isObjectNamesMatch = false;
      }

      TH1 *hist_new =  getAssyncObject (object_new, run, server_new);

      TH1 *hist_old =  nullptr; 
      if (isObjectNamesMatch) hist_old=getAssyncObject (object_old, run, server_old);
     
              
      if (hist_old) formatAssyncHistogram(object_old,hist_old,nROFs_old, Data1Pass, "Old");
      if (hist_new) formatAssyncHistogram(object_new,hist_new,nROFs_new ,Data2Pass, "New");
      

      string analysis_result;                    
      TH1* ratio = performRatio(hist_old, hist_new, object_old.isCentralBarrelCut);
      if (ratio){
            ratio->SetTitle(Form("Ratio: %s / %s", 
                           Data1Pass.size() < 2 ? "online" : Data1Pass.c_str(),
                           Data2Pass.size() < 2 ? "online" : Data2Pass.c_str()));
            analysis_result = doCompare(ratio, ratio_thr, object_old.isCentralBarrelCut || object_new.isCentralBarrelCut );
      }else {
        nEmpty++;
      }


      
      string object_title = object_new.Name + ":      " + analysis_result;
     
      if (analysis_result.size()>0) {
        nProblem++;



      myPDF->AddDraw({
              {hist_old, object_old},
              {hist_new, object_new},              
              {ratio,   object_new}   
        }, object_title, run);
   
      }



    }


    mReportFile<<"Run: "<< run << "empty objects: "<< nEmpty <<" problematic objects: "<< nProblem << std::endl;
  }
    myPDF->close();





  std::ofstream outfile(
      Form("%s/%s_out.txt", folder_name.c_str(), outname.c_str()));
  for (const std::string &run : runs) {

    outfile << run << std::endl;
  }

  return 1;
}


string AssyncProcessor::doCompare(const TH1* hRatio, const double ratio_thr, const bool isCentralBarrelCut ){

  string result = "";


  auto [xMin, yMin, xMax, yMax] = getMinMaxCoordinates(hRatio);

  if (hRatio->InheritsFrom("TH2")) {
    if (1-hRatio->GetMinimum(0) > ratio_thr) result += string( Form("   Larger by %.2f at [%.1f,%.1f]",1-hRatio->GetMinimum(0),xMin, yMin));
    if (hRatio->GetMaximum()-1 > ratio_thr)  result += string( Form("    Smaller by %.2f at [%.1f,%.1f]",hRatio->GetMaximum()-1,xMax, yMax));

  } else {
    if (1-hRatio->GetMinimum(0) > ratio_thr) result += string( Form("   Larger by %.2f at [%.1f]",1-hRatio->GetMinimum(0),xMin));
    if (hRatio->GetMaximum()-1 > ratio_thr)  result += string( Form("    Smaller by %.2f at [%.1f]",hRatio->GetMaximum()-1,xMax));

  }


 
  return result;

} 



std::vector<string> AssyncProcessor::getRuns(const string &path) const {

  std::ifstream infile(path);

  std::vector<std::string> out;
  std::string str;
  if (!infile.is_open()) {
    std::cout << "[ERROR][AssyncProcessor] wrong path to the file with the runs: " << path << std::endl;
    exit(0);
  }

  while (std::getline(infile, str)) {
     try {
        int run = stoi(str.substr(0, 6));
     } catch(...){
	      std::cout<<"[WARNING][AssyncProcessor][getRuns] "<< str << "to 6-digit int due to"<<std::endl;
    	continue;
     } 
     out.emplace_back(str.substr(0, 6));
 
  }
  return out;
}


TH2D *AssyncProcessor::produceAverageClusterPlot(const CCDBServer &server,
                                             const string &run,
                                             const QA_object& object) {


  const int nBinsX[NLayer] = {9,9,9,16,16,28,28};


  TString in_name = object.Name;
  TH2D *obj = new TH2D("obj", in_name.Data(), 24, -12, 12, 14, 0, 14);
  obj->SetStats(0);

  QA_object temp;
  temp.Task = object.Task;
  temp.ObjectType = "TH2";
  string ClusterObjectName = in_name.ReplaceAll("avg_", "").Data();

  for (int iLayer = 0; iLayer < NLayer; iLayer++) {

    temp.Name = Form("Layer%d/%s", iLayer, ClusterObjectName.c_str());

    TH2D *hClusterLayer = (TH2D *)server.downloadObject(run.c_str(), temp);

    if (!hClusterLayer) {
      std::cout << "[ERROR] [AssyncProcessor] can't open cluster object for average plot for Layer: "
           << iLayer << std::endl;
      delete obj;
      return nullptr;
    }

    for (int iStave = 0; iStave < NStaves[iLayer]; iStave++) {

      int ybin = iStave < (NStaves[iLayer] / 2) ? 7 + iLayer + 1 : 7 - iLayer;
      int xbin =
          12 - NStaves[iLayer] / 4 + 1 + (iStave % (NStaves[iLayer] / 2));
      double sum = 0;
      for (int ix = 1; ix <= nBinsX[iLayer]; ix++) {
        sum += hClusterLayer->GetBinContent(ix, iStave + 1);
      }
      obj->SetBinContent(xbin, ybin, sum / hClusterLayer->GetNbinsX());
    }
  }

  return obj;
}