#pragma once

#include <TH1.h>
#include <TH2.h>

#include <TStyle.h>

#include "QAObject.h"

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

void FormatHisto(TVirtualPad *c1, TH1 *obj,
                              const QA_object &object, const string &run,
                              const TString apass, const long hROFs) {

  if (!obj) {
    std::cout << "[ERROR] Null pointer passed to FormatHisto! histograms "
                 "remain unchanged"
              << std::endl;
    return;
  }

  obj->SetTitle(Form("Run%s %s ", run.c_str(),
                     apass.Length() < 2 ? "online" : apass.Data()));
  	  if ( object.Name.find("VertexZ") != string::npos) obj->Rebin(100);


          if ( object.isDoROF_norm ) obj->Scale(1./hROFs);
          if ( object.isLogy) c1->SetLogy();
          if ( object.isLogx) c1->SetLogx();
          if ( !obj->InheritsFrom("TH2")) c1->SetGridy();
  
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

  string obj_title = TokenizePath(object.Name, '/');

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
    obj->Draw("colz");
  } else {
    obj->Draw("hist");
  }
}





string TokenizePath(const string &input, const char &token) {

  string out = input;
  while (out.find(token) != std::string::npos) {
    out = out.substr(out.find(token) + 1);
  }
  return out;
}
