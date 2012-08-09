// 3/20/2004 
// Aran Garcia-Bellido (aran@fnal.gov)
// University of Washington
//
// This file is meant to be called from compare_data_background.C or compare_histos.C.
// The input files should be detailed in add_files.list
// It is EXTREMELY important that the first file in that file is the DATA, then
// the QCD, then the W+jets, and the signal(s) last.
// You can also, and it is recommended, specify a fancy legend name in that file like: 
// t#bar{t} #rightarrow lep + jets 
// And of course the color. 
// Ok, so we have 7 files with DATA, QCD, WJETS, TTBARDILEP, TTBARLJETS,
// SCHANNEL and TCHANNEL, and we want to plot the data as dots and then all bkgs
// stacked up. And the signals as lines (if not, then set nsig=0, and the signals 
// will be stacked up as background).
// This macro will be called with the same histo from each of the 7 files stored
// in histograms. We will generate another TObjArray in which the second bkg is
// the sum of the first and the second, the third is the sum of the previous and
// the third, etc ... so as to stack the histos. 
// Then the usual procedure to make things nice! 

void AddHistos(TObjArray histograms, 
		       TString title="Main Title",
		       TString xtitle="x title",
		       TString ytitle="y title",
		       Int_t nsig=0,
		       Int_t palette[10]){
 
  // Number of histos to plot:
  const Int_t ntot = histograms.GetEntries();
  Double_t yield[ntot]=0.0;
  bool draw_legend = true;
  // nsig is the number of signal histograms at the end of the array. 
  //This is used to know how many histograms need to be drawn as lines (and not stacked up). 
  //If you want the signal(s) to also be stacked up as the backgrounds, use nsig=0.

  // Check we have what we expect (the order should be: data, qcd, wjets, etc...)  
  // Do the cumulative summing, except for the first two and the signals: 
  TObjArray addedhistos; addedhistos.Clear();
  for(Int_t i = 0; i<ntot; i++){
      TH1F * hthis = (TH1F*) histograms[i];
      hthis->Sumw2(); // you need this for the KS test
      // include the overflow/underflow bins:
      int numbins = hthis->GetNbinsX(); //this is the last bin plotted
      double hicontent = hthis->GetBinContent(numbins);
      double overflow  = hthis->GetBinContent(numbins+1);// this bin contains the overflow
      double locontent = hthis->GetBinContent(1);// this is the first bin plotted
      double underflow = hthis->GetBinContent(0);// this bin contains the underflow
      if (underflow>0 || overflow>0){
	  //printf("%-20s numbins=%4i hicontent=%4.2f over=%4.2f locontent=%4.2f underflow=%4.2f \n",
	  //	 title.Data(),numbins,hicontent,overflow,locontent,underflow);
      }
      hthis->SetBinContent(numbins,hicontent+overflow);
      hthis->SetBinContent(1,locontent+underflow);
      //
      yield[i] = hthis->Integral(); // yields have to be calculated including overflow bins!
      addedhistos.Add(hthis); // Fill in the new TObjArray with a copy 
      //cout << i  << " " << hthis->Integral() << " " << hthis->GetName() << endl;
      // Now, do the stacking:
      if (i>1 && i<ntot-nsig) {// Will not add the signal histograms
	  TH1F * hprevious = (TH1F*) addedhistos[i-1];
	  if ( hthis->GetXaxis()->GetNbins() != hprevious->GetXaxis()->GetNbins() ) {
	      cout << "Not stackable histos! Exiting." << endl;
	      // Protection against _whoran histogram. 
	      // We cannot add two histograms with different numbers of bins! 
	      return;
	  }
	  hthis->Add(hprevious); // Do the addition
	  addedhistos.RemoveAt(i); // And substitute whatever we had
	  addedhistos.AddAt(hthis,i);
      }
      // At the end of this loop, if we start with: data, qcd, wjets, ttbar, zz, tb, tqb (and nsig=2, the last two), then:
      // addedhistos={0:data,1:qcd,2:qcd+wjets,3:qcd+wjets+ttbar,4:qcd+wjets+ttbar+zz,5:tb,6:tqb}
      // So ntot=7, nsig=2, and the background sum is located in addedhistos[ntot-nsig-1=4] 
  }
  
  // Before we rebin, we should calculate the KS.
  // If there are nsig signal histograms at the end of addedhistos, then the total sum of bacgrounds will be in ntot-nsig-1
  TH1F *h0 = (TH1F*) addedhistos[0];
  TH1F * hbkg = (TH1F*) addedhistos[ntot-nsig-1]; //see above
  double KS = h0->KolmogorovTest(hbkg);
  double chi2ndf = h0->Chi2Test(hbkg, "UWUFOFCHI2/NDF");
  //cout << title.Data() << " KS = " << KS << " chi2/NDF = " << chi2ndf << endl;
  // Rebin? Set nrebin = 0 to NOT do rebinning. 
  // Will rebin only histos whose maximum x axis value exceeds 20. 
  // Anything with less will most probably be already made of integers, so no
  // need to rebin that! 
  Int_t bins = h0->GetXaxis()->GetNbins();
  Int_t nrebin = 5;
  if ( bins > 750 && bins <= 1000) nrebin = 25;
  if ( bins > 400 && bins <= 750 ) nrebin = 20;
  if ( bins > 300 && bins <= 400 ) nrebin = 15;
  if ( bins > 200 && bins <= 300 ) nrebin = 10;
  if ( bins > 150 && bins <= 200 ) nrebin = 10;
  if ( bins > 100 && bins <= 150 ) nrebin = 5;
  if ( bins > 50 && bins <= 100 )  nrebin = 10;
  if ( bins > 20 && bins <= 50 )   nrebin = 2;
  if ( bins <= 20 ) nrebin = 1;  

  if ( nrebin != 0 ) { 
      for (Int_t i = 0; i<ntot; i++){
	  TH1F * h = (TH1F*) addedhistos[i];
	  h->Rebin(nrebin);
      }
  }

  // Now, check largest dimensions so that we can plot all histograms at once.
  Float_t xmin, xmax, ymin, ymax;
  for(Int_t i = 0; i<ntot; i++){
    TH1F * h = (TH1F*) addedhistos[i];
    TAxis *axis = h->GetXaxis();
    if( axis->GetXmin() < xmin ) xmin = axis->GetXmin();
    if( axis->GetXmax() > xmax ) xmax = axis->GetXmax();
    if( h->GetMinimum() < ymin ) ymin = h->GetMinimum();
    if( h->GetMaximum() > ymax ) ymax = h->GetMaximum();
  }

  // Prepare the TLegend(x1, y1, x2, y2)
  TLegend *lgd = new TLegend(.65,.55,.89,.89); 
  // This line makes the legend transparent (not grey, ewwww!): 
  lgd->SetBorderSize(0); lgd->SetTextSize(0.05); lgd->SetTextFont(62); lgd->SetFillColor(0);
  // Plot the legend in reverse order (but data goes first):
  // This is important so that the legend can also be followed in B&W
  TString lgd_entry = h0->GetName();
  lgd_entry.Form("%s %5.0f",h0->GetName(),yield[0]);
  NiceAddEntry(lgd,h0,lgd_entry.Data(),"PL");// data point
  for(Int_t i = ntot-nsig-1; i>0; i--){// bkgs only
      TH1F * h = (TH1F*) addedhistos[i];
      lgd_entry.Form("%s %5.1f",h->GetName(),yield[i]);
      NiceAddEntry(lgd,h,lgd_entry.Data(),"F");
  }
  for(Int_t i = ntot-1; i>ntot-nsig-1; i--){// signals only
      TH1F * h = (TH1F*) addedhistos[i];
      //h->SetLineWidth(3); h->SetLineColor(palette[i]);
      lgd_entry.Form("%s %5.1f",h->GetName(),yield[i]);
      NiceAddEntry(lgd,h,lgd_entry.Data(),"L");
  }
  ymax = TMath::Nint(ymax*1.15+1); // Make enough room for the big legend
  Int_t nbinsx = h0->GetXaxis()->GetNbins();
  Int_t nbinsy = 100; 
  TString title = h0->GetTitle();

  // Now make the frame:
  TH2F * frame = new TH2F("frame","",nbinsx,xmin,xmax,nbinsy,ymin,ymax);
  if ( xtitle = "" ) {// Use the main title as axis title if we don't have any
    frame->SetXTitle(title.Data());
  } else {
    frame->SetXTitle(xtitle.Data());
  }
  frame->SetYTitle(ytitle.Data()); 
  frame->SetStats(false);
  frame->Draw();

  // Draw data and backgrounds first, in reverse order: from last to first! 
  for(Int_t i=ntot-nsig-1; i>=0; i--){
    TH1F * h = (TH1F*) addedhistos[i];
    h->SetStats(kFALSE);
    // Data is special:
    if (i==0) {//data
      h->SetMarkerSize(1.5); 
      h->SetMarkerStyle(8);
      h->SetMarkerColor(palette[i]);
      h->Draw("E1,SAME");
    } else {//backgrounds
      h->SetFillStyle(1001);
      h->SetFillColor(palette[i]);
      h->SetLineWidth(0);
      h->Draw("HISTSAME"); // HIST is needed if you run Sumw2.
    }
  }
  // We draw the signal lines last, so that they can be seen over the rest.
  for(Int_t i = ntot-1; i>ntot-nsig-1; i--){//signals
    TH1F * h = (TH1F*) addedhistos[i];
    h->SetStats(kFALSE);
    h->SetLineColor(palette[i]);
    h->SetLineWidth(5);
    h->SetLineStyle(1);
    h->Draw("HISTSAME"); // HIST is needed if you run Sumw2.
  }
  
  // Are we preliminary or not? 
  TLatex *t1 = new TLatex(); 
  t1->SetTextFont(62);
  t1->SetTextColor(1);   
  t1->SetTextAlign(12);
  t1->SetTextSize(0.06);
  t1->DrawTextNDC(0.13,0.95,"D\349 Run II Preliminary");
  t1->SetTextAlign(32);
  t1->DrawTextNDC(0.93,0.95,"L = 2.6 fb^-1");
  // Since I don't manage to give a general title, I have to do it the hard way:
  //  TPaveLabel *pl = new TPaveLabel(0.1,0.90,0.90,0.95,"Final event selection","brNDC");
  //  pl->SetBorderSize(0);
  //pl->Draw();

  if ( draw_legend ) {
    lgd->Draw("SAME");
  }
  
  // Draw the KS:
  TString ks_val = Form("KS = %3.2f",KS);
  t1->SetTextAlign(12); t1->SetTextSize(0.03); // t1->SetTextAngle(90); 
  t1->DrawTextNDC(0.55,0.93,ks_val.Data());
  TString chi2_val = Form("#chi^{2}/ndf = %3.1f",chi2ndf);
  t1->SetNDC(true); t1->DrawLatex(0.55,0.96,chi2_val.Data());
  //
  // Voila!
  //
}

///
/// Do it now -- but for 2d. Here we just draw several of them.
///
void AddHistos2D (TCanvas *c1,
		  TObjArray histograms, 
		  TString title="Main Title",
		  TString xtitle="x title",
		  TString ytitle="y title")
{
  ///
  /// Calculate the number of rows
  ///

  int n = histograms.GetEntriesFast();
  int rows = 0;
  int columns = 100;
  while (columns > 3) {
    rows++;
    columns = n / rows;
    if (rows * columns < n) {
      columns++;
    }
  }

  c1->Divide (columns, rows);
  gStyle->SetPalette(1);
  for (int i = 0; i < n; i++) {
    TH2 *h = histograms[i];
    c1->cd(i+1);
    h->Draw("COLZ");
    TLegend *lgd = new TLegend (0.7, 0.7, 0.89, 0.89);
    lgd->AddEntry (h->GetName(), h->GetName(), "l");
    lgd->Draw();
  }
}


Float_t FindMax(TH1F* _histo1,TH1F* _histo2 )
{
  // Find max and min
  Float_t data_max = _histo1->GetMaximum();
  Float_t back_max = _histo2->GetMaximum();
  Float_t max = data_max;
  if ( back_max > max )
    max = back_max;
  return( max );
}

void NiceAddEntry(TLegend* leg, TH1F* hist, TString name, TString type){
// The stupid lgd->SetTextAlign(13) does not work on the entries!!!
// You have to add the entry in this convoluted way for the text to be properly aligned
// in the vertical direction if it has some math like super or subscript or a #bar{}
    TLegendEntry *entry=leg->AddEntry(hist,name.Data(),type.Data());
    entry->SetTextAlign(12); // default H: left V: center
    if ( name.Contains("#bar") || name.Contains("^{")) {
        entry->SetTextAlign(13); // H: left V: top
	if (type == "l") entry->SetTextAlign(11); // special case for ShowHistosLines
    } else if (name.Contains("_{")) {
        entry->SetTextAlign(11); // H: left V: bottom
    }
}




