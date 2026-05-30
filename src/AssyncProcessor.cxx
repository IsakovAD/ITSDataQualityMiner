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

    PeriodName1 = params.at("MCPeriod_old");
    PeriodName2 = params.at("MCPeriod_new");

    //[TO-DO] error checks on wrong json format;
    //cout<<"Starting Analysis with: data_path= "<<data_path << " Data Type old: "<< Data1Type<< " Data Type new: "<< Data2Type << " Pass old: "<< Data1Pass << " Pass new: "<<Data2Pass << " MC period old: "<< PeriodName1 << " MC period new: "<< PeriodName2 <<endl; 
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
      else
        cout << "[ERROR] Wrong item in the input object list: " << key << ":"
             << item << endl;
    }
    vOut.emplace_back(current_object);
  }
  return vOut;
}

void AssyncProcessor::PrepareOutputFolders(){

  
  TString outname = ((string)data_path).substr(0, data_path.size() - 4);
  folder_name = "output/"+(string) outname; //TO-DO: generalize location of the ouput

  if (!std::filesystem::exists(folder_name))
    std::filesystem::create_directory(folder_name);

  std::ofstream log_file(folder_name + "/output.log", ofstream::out);
  original_cout_buffer = std::cout.rdbuf();

  std::cout.rdbuf(log_file.rdbuf());

}


int AssyncProcessor::StartQA() {


  CCDBServer server_new(Data2Type, Data2Pass, PeriodName2);
  CCDBServer server_old(Data1Type, Data1Pass, PeriodName1);


  vector<QA_object> vObjects_old = readObjects(Form("input/objects_%s.json", Data1Type.c_str()));
  vector<QA_object> vObjects_new = readObjects(Form("input/objects_%s.json", Data2Type.c_str()));

  //[TO-DO] make class for TCanvass
  int nObjects = 0, nCurrentPosition = 0, nRows = 10;
  TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 1440, nRows * 480);
  c1->Divide(3, nRows);

  
  for (int i = 1; i <= 3 * nRows; i++)
    c1->cd(i)->SetRightMargin(0.15);

  for (string run : runs) {

    cout << " ------------- run is " << run << endl;

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
      

      TH1 *obj_old, *obj_new;
      if (object_new.Name.find("avg") != string::npos) {
        obj_old = produceAverageClusterPlot(server_old, run, object_old);
        obj_new = produceAverageClusterPlot(server_new, run, object_new);
      } else {
        obj_old = server_old.downloadObject(run, object_old);
        obj_new = server_new.downloadObject(run, object_new);
      }

      FormatHisto(c1->cd(nCurrentPosition * 3 + 2), obj_new, object_new, run, server_new.getApass(), nROFs_new);
      FormatHisto(c1->cd(nCurrentPosition * 3 + 1), obj_old, object_old, run, server_old.getApass(), nROFs_old);
      setMinMax(obj_old, obj_new);

      PlotHisto(c1->cd(nCurrentPosition * 3 + 2), obj_new);
      PlotHisto(c1->cd(nCurrentPosition * 3 + 1), obj_old);

      performRatio(c1->cd(nCurrentPosition * 3 + 3), obj_new, obj_old, object_new, server_new.getApass(), server_old.getApass());



      //------------- operation with Canvases:
      c1->SetTitle(Form("Run %s, %s", run.c_str(), object_new.Name.c_str()));
      if (((nObjects + 1) % nRows == 0) || (nObjects == vObjects_new.size() * runs.size() - 1)) {
        if ((nObjects + 1) == nRows) {
          c1->Print(Form("%s/%s.pdf(", folder_name.c_str(), outname.Data()), "pdf");
        } else if (nObjects == vObjects_new.size() * runs.size() - 1) continue;
              else c1->Print(Form("%s/%s.pdf", folder_name.c_str(), outname.Data()), "pdf");
        
        nCurrentPosition = 0;
        c1->Clear();
        c1->Divide(3, nRows);
        for (int i = 1; i <= 3 * nRows; i++)
          c1->cd(i)->SetRightMargin(0.15);

      } else {
        nCurrentPosition++;
      }

      nObjects++;
    }
  }

  c1->Print(Form("%s/%s.pdf)", folder_name.c_str(), outname.Data()), "pdf");

  std::ofstream outfile(
      Form("%s/%s_out.txt", folder_name.c_str(), outname.Data()));
  for (const string &run : runs) {

    outfile << run << endl;
  }

  std::cout.rdbuf(original_cout_buffer);
  return 1;
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

    for (int iStave = 0; iStave < mNStaves[iLayer]; iStave++) {

      int ybin = iStave < (mNStaves[iLayer] / 2) ? 7 + iLayer + 1 : 7 - iLayer;
      int xbin =
          12 - mNStaves[iLayer] / 4 + 1 + (iStave % (mNStaves[iLayer] / 2));
      double sum = 0;
      for (int ix = 1; ix <= nBinsX[iLayer]; ix++) {
        sum += hClusterLayer->GetBinContent(ix, iStave + 1);
      }
      obj->SetBinContent(xbin, ybin, sum / hClusterLayer->GetNbinsX());
    }
  }

  return obj;
}