#include "TFile.h"
#include "TKey.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TGraphAsymmErrors.h"
#include "TRandom3.h"

#include <cmath>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <time.h>

#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "JetMETCorrections/Modules/interface/JetResolution.h"
#include "DataFormats/Math/interface/deltaPhi.h"

using namespace std;
void setPars(const string& parFile);
void FillHist1D(const TString& histName, const Double_t& value, const double& weight);
void FillHist2D(const TString& histName, const Double_t& value1, const Double_t& value2, const double& weight);

bool isMC;
TString inName;
map<TString, TH1*> m_Histos1D;
map<TString, TH2*> m_Histos2D;
double weight0, weight;
const int MAXGEN = 20;
int main(int argc, char* argv[]){

 if (argc == 1) { cout << "Please provide a parameter file." << endl; return -1; }

  string parFile = argv[1];
  setPars(parFile);
  weight0 = -1;
  //if (argc == 3)     { string wFile = argv[2]; setWeight(wFile); }
  if (weight0 == -1) { cout << "Weight set to 1" << endl; weight0 = 1.; }
  else                 cout << "Weight set to " << weight0 << endl;

  TString hname;
  hname = Form("costoplep");
  m_Histos1D[hname] = new TH1D(hname,hname,40,-1,1);
  hname = Form("cosantitoplep");
  m_Histos1D[hname] = new TH1D(hname,hname,40,-1,1);
  hname = Form("costopvscosantitop");
  m_Histos2D[hname] = new TH2D(hname,hname,40,-1,1,40,-1,1);

  //Open Files//

  TFile* inFile = TFile::Open(inName);
  TTree* T = (TTree*) inFile->Get("T");
  Long64_t nEntries = T->GetEntries();
  cout << endl << nEntries << " Events" << endl;
  cout << "Processing " + inName << endl;
  TString name = inName( inName.Last('/')+1, inName.Last('.')-inName.Last('/')-1);
  //cout << name << endl;

  //Set Branches//

  ULong64_t event;
  int run, nPV;
  float rho, mu;
  int nGen=MAXGEN, gen_status[nGen], gen_PID[nGen], gen_index[nGen], gen_mother0[nGen], gen_mother1[nGen];
  float gen_pt[nGen], gen_mass[nGen], gen_eta[nGen], gen_phi[nGen];

  T->SetBranchAddress("event", &event);
  T->SetBranchAddress("run", &run);
  T->SetBranchAddress("nPV", &nPV);
  T->SetBranchAddress("rho", &rho);
  if (isMC) T->SetBranchAddress("mu", &mu);


  if (name.Contains("tt", TString::kIgnoreCase) || name.Contains("zprime", TString::kIgnoreCase) || name.Contains("gkk", TString::kIgnoreCase)) {
    T->SetBranchAddress("nGen", &nGen);
    T->SetBranchAddress("gen_status", gen_status);
    T->SetBranchAddress("gen_PID", gen_PID);
    T->SetBranchAddress("gen_pt", gen_pt);
    T->SetBranchAddress("gen_mass", gen_mass);
    T->SetBranchAddress("gen_eta", gen_eta);
    T->SetBranchAddress("gen_phi", gen_phi);
    T->SetBranchAddress("gen_index",  gen_index);
    T->SetBranchAddress("gen_mother0",  gen_mother0);
    T->SetBranchAddress("gen_mother1",  gen_mother1);
  }
  //Loop Over Entries//
  time_t start = time(NULL);

  for (Long64_t n=0; n<nEntries; n++) {
    T->GetEntry(n);
    weight = weight0;
    if (isMC) {
      //weight *= pileup_weights->GetBinContent( pileup_weights->FindBin(mu) );
      TLorentzVector top, antitop;
      TLorentzVector LEPfromTOP, LEPfromANTITOP;
      TVector3 beta_top , beta_antitop;
      if (name.Contains("tt", TString::kIgnoreCase) || name.Contains("zprime", TString::kIgnoreCase) || name.Contains("gkk", TString::kIgnoreCase)) {
        for (int i=0; i<nGen; i++) {
          if (gen_PID[i]==6 && gen_status[i]>30) top.SetPtEtaPhiM( gen_pt[i], gen_eta[i], gen_phi[i], gen_mass[i] );
          else if (gen_PID[i]==-11 || gen_PID[i]==-13) LEPfromTOP.SetPtEtaPhiM( gen_pt[i], gen_eta[i], gen_phi[i], gen_mass[i] );
          else if (gen_PID[i]==-6 && gen_status[i]>30) antitop.SetPtEtaPhiM( gen_pt[i], gen_eta[i], gen_phi[i], gen_mass[i] );
          else if (gen_PID[i]==11 || gen_PID[i]==13)   LEPfromANTITOP.SetPtEtaPhiM( gen_pt[i], gen_eta[i], gen_phi[i], gen_mass[i] );
          beta_top = top.BoostVector() ;
          beta_antitop = antitop.BoostVector() ;
        }
      } 
      double top_lep_cos, antitop_lep_cos;
      if (top.Mag()!=0 && LEPfromTOP.Mag()!=0){
        LEPfromTOP.Boost(-beta_top);
        top_lep_cos =  LEPfromTOP.Vect().Unit().Dot(-beta_top.Unit());
      }
      if (antitop.Mag()!=0 && LEPfromANTITOP.Mag()!=0){
        LEPfromANTITOP.Boost(-beta_antitop);
        antitop_lep_cos =  LEPfromANTITOP.Vect().Unit().Dot(-beta_antitop.Unit());
      }
      FillHist1D("costoplep", top_lep_cos, weight);
      FillHist1D("cosantitoplep", antitop_lep_cos, weight);
      FillHist2D("costopvscosantitop", top_lep_cos, antitop_lep_cos, 1.);
      
    }
  }
  cout << difftime(time(NULL), start) << " s" << endl;
  //Write Histograms//
  TString outName = name + "_analyzed.root";
  TFile* outFile = new TFile(outName,"RECREATE");
  outFile->cd();

  for (map<TString, TH1*>::iterator hid = m_Histos1D.begin(); hid != m_Histos1D.end(); hid++) {
    outFile->cd();
    TString prefix = hid->first(0, 1);
    if ( prefix.IsDigit() ) outFile->cd(outName + ":/" + prefix);
    hid->second->Write();
  }
  for (map<TString, TH2*>::iterator hid = m_Histos2D.begin(); hid != m_Histos2D.end(); hid++) {
    outFile->cd();
    TString prefix = hid->first(0, 1);
    if ( prefix.IsDigit() ) outFile->cd(outName + ":/" + prefix);
    hid->second->Write();
  }
  outFile->Write();
  delete outFile;
  outFile = 0;

}

void setPars(const string& parFile) {

  ifstream file(parFile);
  string line;

  while (getline(file, line)){

    if (line.length() > 0){
      while (line.at(0) == ' ') line.erase(0, 1);
    }

    int delim_pos = line.find(' ');
    if (delim_pos == -1) continue;

    string var = line.substr(0, delim_pos);
    line.erase(0, delim_pos + 1);

    while (line.at(0) == ' ') line.erase(0, 1);
    while (line.at(line.length()-1) == ' ') line.erase(line.length()-1, line.length());

    if (var == "isMC"){
      if (line == "true") isMC = true;
      else isMC = false;
    }
    else if (var == "inName") inName = line.data();
  }
  file.close();
}

void FillHist1D(const TString& histName, const Double_t& value, const double& weight) {
  map<TString, TH1*>::iterator hid=m_Histos1D.find(histName);
  if (hid==m_Histos1D.end())
    cout << "%FillHist1D -- Could not find histogram with name: " << histName << endl;
  else
    hid->second->Fill(value, weight);
}
void FillHist2D(const TString& histName, const Double_t& value1, const Double_t& value2, const double& weight) {
  map<TString, TH2*>::iterator hid=m_Histos2D.find(histName);
  if (hid==m_Histos2D.end())
    cout << "%FillHist2D -- Could not find histogram with name: " << histName << endl;
  else
    hid->second->Fill(value1, value2, weight);
}
