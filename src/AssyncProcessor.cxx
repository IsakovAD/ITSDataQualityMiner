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
    cout<<"============= Starting Analysis with: data_path= "<<data_path << " Data Type old: "<< Data1Type<< " Data Type new: "<< Data2Type << " Pass old: "<< Data1Pass << " Pass new: "<<Data2Pass << " MC period old: "<< MCPeriodName1 << " MC period new: "<< MCPeriodName2 <<endl; 
  } else {
    cout << "[ERROR] can't open .json with parameters" << endl;
    exit(1);
  }
}


vector<QA_object> AssyncProcessor::readObjects(const string &file_name) {

  //[to-do] stoi can throw exception
  vector<QA_object> vOut;

  auto object_json_file = parseJSON(file_name);

  for (const auto &object : object_json_file) {
    QA_object current_object;
    for (const auto &[key, item] : object) {
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
        cout << "[ERROR] Wrong item in the input object list: " << key << ":"
             << item << endl;
    }
    vOut.emplace_back(current_object);
  }
  return vOut;
}

void AssyncProcessor::PrepareOutputFolders(){

  std::cout<<" at the PrepareOutputFolders"<<std::endl;
  outname = ((string)data_path).substr(0, data_path.size() - 4);
  folder_name = "output/its-qa-qc/"+(string) outname; //TO-DO: generalize location of the ouput

  if (!std::filesystem::exists(folder_name))
    std::filesystem::create_directory(folder_name);

  
  std::cout<<" creating log"<<std::endl;
  std::ofstream log_file(folder_name + "/output.log", ofstream::out);

   std::cout<<" cout buffer creating: "<<std::endl;
  original_cout_buffer = std::cout.rdbuf();
   std::cout<<" connecting cout buffer to log file"<<std::endl;
  //!!!!!!!!!!!!!!!! std::cout.rdbuf(log_file.rdbuf());
  
  std::cout<<"PrepareOutputFolders is finished!"<<std::endl;

}


int AssyncProcessor::StartQA() {


  string ccdb_port;

  if ( Data2Type == "qc_mc") ccdb_port  = "ali-qcdbmc-gpn.cern.ch:8083";
  else ccdb_port  = "ali-qcdb-gpn.cern.ch:8083";

  //[to-do] Check if we need period and how it works with MC

  CCDBServer server_new(Data2Type, ccdb_port, Data2Pass);

  if ( Data1Type == "qc_mc") ccdb_port  = "ali-qcdbmc-gpn.cern.ch:8083";
  else ccdb_port  = "ali-qcdb-gpn.cern.ch:8083";


  CCDBServer server_old(Data1Type, ccdb_port, Data1Pass);


  vector<QA_object> vObjects_old = readObjects(Form("inputs/its-qa-qc/objects_%s.json", Data1Type.c_str()));
  vector<QA_object> vObjects_new = readObjects(Form("inputs/its-qa-qc/objects_%s.json", Data2Type.c_str()));



 PDFBuilder *myPDF = new PDFBuilder(10, folder_name.c_str());

 for (string run : runs) {

    cout << " ------------- run is " << run << endl;
    myPDF->AddText("run "+ run);
    long nROFs_old = server_old.getNROFs(run);
    long nROFs_new = server_new.getNROFs(run);


    for (QA_object object_new : vObjects_new) {
      if (!object_new.isEnabled)    continue;

      QA_object object_old; 
      const auto it = find_if(vObjects_old.begin(), vObjects_old.end(),
                              [&object_new](const QA_object &obj) {
                                return object_new.Name == obj.Name;
                              });
      if (it != vObjects_old.end()) object_old = *it;
      
      std::cout<<"[DEBUG] [before if] Downloading object: "<<object_new.Name << " old: "<<object_old.Name  << std::endl;                        
      TH1 *hist_old, *hist_new;
      if (object_new.Name.find("avg") != string::npos) {
        hist_old = produceAverageClusterPlot(server_old, run, object_old);
        hist_new = produceAverageClusterPlot(server_new, run, object_new);
      } else {
        std::cout<<"-------- [DEBUG] Downloading object old: "<<object_old.Name << std::endl;
        hist_old = server_old.downloadObject(run, object_old);
         std::cout<<"-------- [DEBUG] Downloading object new: "<<object_old.Name << std::endl;
        hist_new = server_new.downloadObject(run, object_new);
      }

      //FormatHisto(c1->cd(nCurrentPosition * 3 + 2), obj_new, object_new, run, server_new.getApass(), nROFs_new);
      //FormatHisto(c1->cd(nCurrentPosition * 3 + 1), obj_old, object_old, run, server_old.getApass(), nROFs_old);
      
      if ( object_old.isDoROF_norm ) doROFNormalize(hist_old,nROFs_old);
      if ( object_new.isDoROF_norm ) doROFNormalize(hist_new,nROFs_new);
      if (hist_old)
        if ( object_old.Name.find("VertexZ") != string::npos) hist_old->Rebin(100);
      
      if (hist_new)
        if ( object_new.Name.find("VertexZ") != string::npos) hist_new->Rebin(100);

      //setMinMax(hist_old, hist_new); // have troubels with normalized plots!

      // perform ratio - return object 
      // Run Check how ratio is compatible with THR
      // if check is fine - skip object
      // if check is bad - plot object

      TH1* ratio = performRatio(hist_new, hist_old, object_old.isCentralBarrelCut);


      string title;

      if (hist_new){
        title = hist_new->GetTitle();
        hist_new->SetTitle(Form("New data: %s ", 
                      Data1Pass.size() < 2 ? "online" : Data1Pass.c_str()));
      }
      if (hist_old){ 
        title = hist_old->GetTitle();
        hist_old->SetTitle(Form("Old data: %s",
                      Data2Pass.size() < 2 ? "online" : Data2Pass.c_str()));
      }

      if (ratio){
            ratio->SetTitle(Form("Ratio: %s / %s", 
                           Data1Pass.size() < 2 ? "online" : Data1Pass.c_str(),
                           Data2Pass.size() < 2 ? "online" : Data2Pass.c_str()));
      }

      string analysis_result = title + ":      " + doCompare(hist_new, hist_old, 0.01, object_old.isCentralBarrelCut);
      std::cout<<"================= result is: "<< analysis_result << std::endl;
      if (analysis_result.size()>0) {
        //myPDF->AddTitle(result);
        //myPDF->AddDraw({obj_new,obj_old,ratio},result);
        myPDF->AddDraw({
              {hist_new, object_new},
              {hist_old, object_old},
              {ratio,   object_new}   
        }, analysis_result, run);

      }
      //myPDF->AddDraw({obj_new});

    }
  }
    myPDF->close();





  std::ofstream outfile(
      Form("%s/%s_out.txt", folder_name.c_str(), outname.c_str()));
  for (const string &run : runs) {

    outfile << run << endl;
  }

  std::cout.rdbuf(original_cout_buffer);
  return 1;
}


string AssyncProcessor::doCompare(TH1* obj_new, TH1* obj_old, double ratio_thr, bool isCentralBarrelCut ){

  string result = "";
  if (!obj_new || !obj_old){
    std::cout<<"[doCompare] empty objects, skipping"<<std::endl;
    return "One of objects missing";


  }
  TH1* hRatio = performRatio(obj_new, obj_old,isCentralBarrelCut);

  auto [xMin, yMin, xMax, yMax] = getMinMaxCoordinates(hRatio);

  std::cout<<"comparing Min= "<< hRatio->GetMinimum(0) << " Max= "<< hRatio->GetMaximum() << " with THR: "<< ratio_thr << std::endl;

  if (hRatio->InheritsFrom("TH2")) {
    if (1-hRatio->GetMinimum(0) > ratio_thr) result += string( Form("   Larger by %.2f at [%.1f,%.1f]",hRatio->GetMinimum(0),xMin, yMin));
    if (hRatio->GetMaximum()-1 > ratio_thr)  result += string( Form("    Smaller by %.2f at [%.1f,%.1f]",hRatio->GetMaximum(),xMax, yMax));

  } else {
    if (1-hRatio->GetMinimum(0) > ratio_thr) result += string( Form("   Larger by %.2f at [%.1f]",hRatio->GetMinimum(0),xMin));
    if (hRatio->GetMaximum()-1 > ratio_thr)  result += string( Form("    Smaller by %.2f at [%.1f]",hRatio->GetMaximum(),xMax));

  }


 
  return result;

} 



vector<string> AssyncProcessor::getRuns(const string &path) const {

  std::ifstream infile(path);

  vector<string> out;
  string str;
  if (!infile.is_open()) {
    cout << "[ERROR] wrong path to the file with the runs: " << path << endl;
    exit(0);
  }

  while (std::getline(infile, str)) {
     try {
        int run = stoi(str.substr(0, 6));
     } catch(...){
	cout<<"################################### something wrong during conversion string: "<< str << "to 6-digit int due to"<<endl;
    	continue;
     } 
     out.emplace_back(str.substr(0, 6));
/*
    if (str.size() >= 6)
      out.emplace_back(str.substr(0, 6));

    else {
      cout << "[ERROR] wrong line in the run list: " << str << " skipping!"
           << endl;
    }
*/    
  }
  return out;
}


TH2D *AssyncProcessor::produceAverageClusterPlot(const CCDBServer &server,
                                             const TString &run,
                                             QA_object object) {


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

    TH2D *hClusterLayer = (TH2D *)server.downloadObject(run.Data(), temp);

    if (!hClusterLayer) {
      cout << "[ERROR] can't open cluster object for average plot for Layer: "
           << iLayer << endl;
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