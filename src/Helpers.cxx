#include "Helpers.h"



void setStyle() {
  gStyle->SetLineScalePS(1);
  gStyle->SetGridStyle(1);
  gStyle->SetGridColor(17);
  gStyle->SetHatchesLineWidth(1);
  gStyle->SetPaintTextFormat("0.1f");
}


void setMinMax(TH1 *obj1, TH1 *obj2) {

  if (!obj1 || !obj2) {
    std::cout << "[ERROR] Null pointer passed to setMinMax! histograms remain "
                 "unchanged"
              << std::endl;
    return;
  }

  Double_t max = std::max(obj1->GetMaximum(), obj2->GetMaximum());
  Double_t min = std::min(obj1->GetMinimum(), obj2->GetMinimum());

  if (obj1->InheritsFrom("TH2")) {
    obj1->GetZaxis()->SetRangeUser(min, max * 1.1);
    obj2->GetZaxis()->SetRangeUser(min, max * 1.1);
  } else {
    obj1->GetYaxis()->SetRangeUser(min, max * 1.1);
    obj2->GetYaxis()->SetRangeUser(min, max * 1.1);
  }
}

void doROFNormalize(TH1 *obj,const long hROFs){
  obj->Scale(1./hROFs);
}

void FormatHisto(TVirtualPad *c1, TH1 *obj,
                              const QA_object &object) {

  if (!obj) {
    std::cout << "[ERROR] Null pointer passed to FormatHisto! histograms "
                 "remain unchanged"
              << std::endl;
    return;
  }

 


         
          if ( object.isLogy) c1->SetLogy();
          if ( object.isLogx) c1->SetLogx();
          if ( !obj->InheritsFrom("TH2")) c1->SetGridy();
  
}


std::string TokenizePath(const std::string &input, const char &token) {

  std::string out = input;
  while (out.find(token) != std::string::npos) {
    out = out.substr(out.find(token) + 1);
  }
  return out;
}


TH1* performRatio(TH1 *obj_old, TH1 *obj_new, bool isDoCentralBarrelCut) {

  if (!obj_new || !obj_old) {
    std::cout << "[ERROR] Null pointer passed to performRatio! ratio "
                 "histograms will not be calculated"
              << std::endl;
    return nullptr;
  }

  TH1 *obj_ratio;
  if (obj_old->InheritsFrom("TH2"))
    obj_ratio = (TH2 *)obj_old->Clone("ratio");
  else
    obj_ratio = (TH1 *)obj_old->Clone("ratio");

  for (int ix = 1; ix <= obj_new->GetNbinsX(); ix++)
    for (int iy = 1; iy <= obj_new->GetNbinsY(); iy++) {
      if (obj_new->GetBinContent(ix, iy) == 0)
        obj_ratio->SetBinContent(ix, iy, -0.01);
      else
        obj_ratio->SetBinContent(ix, iy,
                                 obj_ratio->GetBinContent(ix, iy) /
                                     obj_new->GetBinContent(ix, iy));
    }
  if  (obj_old->Integral()!=0) obj_ratio->Scale(obj_new->Integral() / obj_old->Integral());
  if (isDoCentralBarrelCut) {
      obj_ratio->GetXaxis()->SetRangeUser(-1.2, 1.2);
  }
  
  return obj_ratio;

}





void performRatio(TVirtualPad *c1, TH1 *obj_new, TH1 *obj_old,
                               QA_object object, TString pass_new,
                               TString pass_old) {

  if (!obj_new || !obj_old) {
    std::cout << "[ERROR] Null pointer passed to performRatio! ratio "
                 "histograms will not be calculated"
              << std::endl;
    return;
  }

  TH1 *obj_ratio;
  if (obj_old->InheritsFrom("TH2"))
    obj_ratio = (TH2 *)obj_old->Clone("ratio");
  else
    obj_ratio = (TH1 *)obj_old->Clone("ratio");

  for (int ix = 1; ix <= obj_new->GetNbinsX(); ix++)
    for (int iy = 1; iy <= obj_new->GetNbinsY(); iy++) {
      if (obj_new->GetBinContent(ix, iy) == 0)
        obj_ratio->SetBinContent(ix, iy, -0.01);
      else
        obj_ratio->SetBinContent(ix, iy,
                                 obj_ratio->GetBinContent(ix, iy) /
                                     obj_new->GetBinContent(ix, iy));
    }

  obj_ratio->Scale(obj_new->Integral() / obj_old->Integral());

  if (!obj_old->InheritsFrom("TH2")) {
    c1->SetGridy();
  }

  std::string obj_title = TokenizePath(object.Name, '/');

  obj_ratio->SetTitle(Form("%s: %s / %s", obj_title.c_str(),
                           pass_old.Length() < 2 ? "online" : pass_old.Data(),
                           pass_new.Length() < 2 ? "online" : pass_new.Data()));
  obj_ratio->SetStats(0);

  obj_ratio->Draw("text,colz");
}



void PlotHisto(TVirtualPad *c1, TH1 *obj) {

   c1->cd();	
  if (!obj) {
    std::cout << "[ERROR] Null pointer passed to PlotHisto! histograms will "
                 "not be plotted"
              << std::endl;
    TText* errorText = new TText(0.5, 0.5, "ERROR WHEN READING OBJECT");
    errorText->SetTextAlign(22); // Center alignment (horizontal and vertical)
    errorText->SetTextSize(0.05); // Adjust text size as needed
    errorText->SetTextFont(42); // Standard ROOT font
    errorText->SetTextColor(kRed); // Red color for visibility

    // Draw the text on the pad
    errorText->Draw();	



    return;
  }

  if (obj->InheritsFrom("TH2")) {
    
                // anything <= this draws as white
     if ( std::string(obj->GetTitle()).find("Ratio") != std::string::npos) { 
      obj->SetMinimum(1e-10);
      obj->Draw("text,colz");
     }
     else obj->Draw("colz");

  } else {
    obj->Draw("hist");
  }
}





BinCoordinates getMinMaxCoordinates(TH1* hist) {
    
    BinCoordinates result{0, 0, 0, 0};

    // get active range
    int firstBinX = hist->GetXaxis()->GetFirst();
    int lastBinX  = hist->GetXaxis()->GetLast();

    // maximum - ROOT's GetMaximumBin respects SetRangeUser
    int maxBin = hist->GetMaximumBin();
    int binX, binY, binZ;
    hist->GetBinXYZ(maxBin, binX, binY, binZ);
    result.xMax = hist->GetXaxis()->GetBinCenter(binX);
    result.yMax = hist->GetYaxis()->GetBinCenter(binY);

    if (hist->InheritsFrom("TH2")) {
        TH2* h2 = (TH2*)hist;

        int firstBinY = h2->GetYaxis()->GetFirst();
        int lastBinY  = h2->GetYaxis()->GetLast();

        int minBinX = -1, minBinY = -1;
        double minContent = std::numeric_limits<double>::max();
        for (int ix = firstBinX; ix <= lastBinX; ix++)  // respects range
            for (int iy = firstBinY; iy <= lastBinY; iy++) {
                double val = h2->GetBinContent(ix, iy);
                if (val > 0 && val < minContent) {
                    minContent = val;
                    minBinX = ix;
                    minBinY = iy;
                }
            }
        result.xMin = h2->GetXaxis()->GetBinCenter(minBinX);
        result.yMin = h2->GetYaxis()->GetBinCenter(minBinY);

    } else {
        int minBinX = -1;
        double minContent = std::numeric_limits<double>::max();
        for (int ix = firstBinX; ix <= lastBinX; ix++) {  // respects range
            double val = hist->GetBinContent(ix);
            if (val > 0 && val < minContent) {
                minContent = val;
                minBinX = ix;
            }
        }
        result.xMin = hist->GetXaxis()->GetBinCenter(minBinX);
        result.yMin = 0;
    }

    return result;
}


