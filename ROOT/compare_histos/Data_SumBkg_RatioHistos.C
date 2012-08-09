//  2008 
//  Aran Garcia-Bellido (aran@fnal.gov)
//  University of Washington
//
//  Plot ratios of histograms.
//
//  You must have already loaded the overlay guy, or this will totally fail!
//
//  This will take the first histogram as the data, and then add all remaining
//  histos as the sum of backgrounds: it will then plot the ratio of DATA/SUMBKG
//

void Data_SumBkg_RatioHistos (TObjArray histograms, 
			      TString title="Main Title",
			      TString xtitle="x title",
			      TString ytitle="y title",
			      Int_t palette[10])
{
  // Number of histos to plot:
  Int_t ntot = histograms.GetEntries();
  
  //
  // The data should be the first entry! 
  //
  TH1 *data = histograms[0];
  data->Sumw2();
  // We want the plots with 20 bins:
  if (data->GetNbinsX()>20){
      int rebin = data->GetNbinsX()/20;
      data->Rebin(rebin);
  }
  //
  // Ok, now sum over the rest: all backgrounds
  //
  TH1 *sumbkg = NULL;
  bool good = true;
  Int_t nsum = 0;
  for (int i = 1; i < ntot; i++) {
    TH1 *h = histograms[i];
    TH1 *newh = h->Clone();
    newh->Sumw2();
    // rebin:
    if (newh->GetNbinsX()>20){
	int rebin = newh->GetNbinsX()/20;
	newh->Rebin(rebin);
    }

    good = good && (data->GetNbinsX() == newh->GetNbinsX());
    if (!good && data->GetNbinsX() != newh->GetNbinsX()) {
      cout << "Histogram '" << newh->GetName() << 
	  "' has different numbers of entries for histo 0 and histo " << i << endl;
    }

    if (good && i==1) sumbkg = newh;
    if (good && i>1) sumbkg->Add(newh);
    if (good) nsum++;
  }
  if (good && nsum == ntot-1) {// this ensures we added all histos we wanted to add
    int palette[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int styles[10]={1};
    int with_errors[10]={1};
    int goodNbins = -1; // This has to be -1. All rebinning must be done in this macro!!!
    TObjArray ratio;// we need an array to pass
    data->Divide(sumbkg);
    ratio.Add(data);// we only pass one histo
    OverlayHistos("StatBox", ratio, title, xtitle, "Ratio DATA/BKGSUM", palette, styles, with_errors, goodNbins);

  } else {
    cout << "Nothing was plotted for '" << data->GetName() << "'." << endl;
    cout << "Check binning and sources." << endl;
  }
}
