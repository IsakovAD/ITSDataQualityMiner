#include <string>
#include <vector>

#include <TCanvas.h>
#include <TPad.h>
#include <TLatex.h>

#include <TH1.h>

#include "Helpers.h"
#include "QAObject.h"

class PDFBuilder{

    public:
        PDFBuilder(int nRows, std::string path):nRows_(nRows),path_(path) {

             c1 = new TCanvas("c1", "c1", 0, 0, xRes_, nRows * yRes_);
             c1->Divide(1,nRows_);
            // [to-do] set marging
            //    for (int i = 1; i <= 3 * nRows_; i++)
            //         c1->cd(i)->SetRightMargin(RighMargin_);

        };




void AddDraw(std::vector<std::pair<TH1*,QA_object>> inputs, const std::string& title, const std::string run) {
    TPad* pad = (TPad*)c1->cd(CurrentRow + 1);
    pad->Draw();

    AddTitle(pad, title);  // draws title in top 10%

    // histogram area in bottom 90%
    pad->cd();
    TPad* histPad = new TPad("histPad", "", 0.0, 0.0, 1.0, 0.9);
    histPad->Draw();
    histPad->cd();
    histPad->Divide(inputs.size(), 1);

    for (int i = 0; i < (int)inputs.size(); i++){

        if (!inputs[i].first) std::cout<<"skipping plot"<<std::endl;
        
        FormatHisto(histPad->cd(i + 1), inputs[i].first, inputs[i].second);
        PlotHisto(histPad->cd(i + 1), inputs[i].first);
    }
    CurrentRow++;
    if (CurrentRow == nRows_) {
        printPage();
        preparePage();
    }
}
    void AddText(const std::string& title) {
            TPad* pad = (TPad*)c1->cd(CurrentRow + 1);
            pad->cd();
            pad->SetFillColor(kGray);        // light grey
            pad->Draw();                      // apply the fill
            TLatex* latex = new TLatex(0.5, 0.5, title.c_str());
            latex->SetNDC();
            latex->SetTextAlign(22);
            latex->SetTextSize(0.15);
            latex->SetTextFont(62);
            latex->Draw();
            CurrentRow++;
    }

 
        void preparePage(){

            CurrentRow = 0;
            c1->Clear();
            c1->Divide(1,nRows_);

        }

        void printPage() {
            std::cout<<"[PDFBuilder] printing page "<<CurrentPage << std::endl;
            if (isFirstPage) {
                c1->Print(Form("%s/report.pdf(",path_.c_str()) , "pdf"); 
                isFirstPage = false;
            } else {
                c1->Print(Form("%s/report.pdf",path_.c_str()),  "pdf");  
            }
            CurrentPage++;

        // for (int i = 1; i <= 3 * nRows; i++)
        //   c1->cd(i)->SetRightMargin(0.15);

        }

void close() {
    printPage();
     std::cout<<"[PDFBuilder] closing  document with "<<CurrentPage << " pages"<< std::endl;
    c1->Print(Form("%s/report.pdf)",path_.c_str()), "pdf");      // always close at the end
}

    
    
    private:
        bool isFirstPage = true;

        int CurrentRow = 0;
        int CurrentPage = 1;
        int nRows_ = 10;
        std::string path_;
        TCanvas *c1;
        int xRes_ = 1440;
        int yRes_ = 480;
        double RighMargin_ = 0.15;

        void AddTitle(TPad* parent, const std::string& title) {
            parent->cd();
            TPad* titlePad = new TPad("titlePad", "", 0.0, 0.9, 1.0, 1.0);
            titlePad->Draw();
            titlePad->cd();
            TLatex* latex = new TLatex(0.5, 0.5, title.c_str());
            latex->SetNDC();
            latex->SetTextAlign(22);
            latex->SetTextSize(0.6);
            latex->SetTextFont(62);
            latex->Draw();
            parent->cd();  // return to parent after drawing
        }



};