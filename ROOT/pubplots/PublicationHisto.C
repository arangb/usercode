// Aran Garcia-Bellido. June 28 2009
// This macro will make a nice plot (intended for publication) of one histogram
// in several files

// It will stack the background and plot lines for the signal.
// Extra text can be added and the position of the legend can be changed 
// The most imoprtant things can be tuned from a config file.

// You can also add three histos on the fly (for example for types 1,2,3) for each sample 
// and then stack them as usual. To do that:
// Histo.Name: 3
// Histo.1.Name: h_jet_pt1_os
// Histo.2.Name: h_jet_pt2_os
// Histo.3.Name: h_jet_pt3_os

// 
// Borrowed very heavily from previous macros for singletop.

// To run: 
// root -l -q -b -x PublicationHisto.C\(\"myvariable.plot\"\)
// 
// If you want to create a gif file (specified in the .plot file), then you
// cannot run in the background, so remove the -b. 

TH1F* h_DATA;
TH1F* h_BKGSUM;
// default text size: 0.045
float textSize = 0.07;

void PublicationHisto(TString config=""){
    //gROOT->SetStyle("Plain"); gROOT->ForceStyle();
    gROOT->ProcessLine(".x /afs/cern.ch/user/a/aran/public/rootlogon.C");
    //gStyle->SetPadBottomMargin(.160); // Need this for subscripts
    //gStyle->SetPadTopMargin(.08);
    
    TEnv *params = new TEnv("PubHisto");
    params->ReadFile (config, kEnvChange);

    TString type=params->GetValue("Histo.Type", "PublicationHisto");
    if (type != "PublicationHisto"){
	printf(" Must have Histo.Type: PublicationHisto in config file");
	return;
    }
    // Fetch the histos from the different files:
    TString hname=params->GetValue("Histo.Name", "BOGUSNAME");
    TObjArray sameHistos;
    if (hname == "3"){// ok this means we want to add 3 histos
	TString h1=params->GetValue("Histo.1.Name", "BOGUS_H1");
	TString h2=params->GetValue("Histo.2.Name", "BOGUS_H2");
	TString h3=params->GetValue("Histo.3.Name", "BOGUS_H3");
	printf("Adding histos: %s, %s, %s \n",h1.Data(),h2.Data(), h3.Data());
	TObjArray sameHistos = GetThreeSameHistos(h1,h2,h3,params);
    } else {// normal case
	TObjArray sameHistos = GetSameHistos(hname,params);    
    }

    // Plot them:
    printf (" Found %3i histos \n",sameHistos.GetEntriesFast());
    TCanvas *c1 = new TCanvas("c1","PubHist",700,500);
    // This makes 2 pads, the lower being the ratio of the data to the bkg sum.
    bool doratio=params->GetValue("DoDataBkgSumRatio", false);
    TPad *pad1 = new TPad("pad1","pad1",0,0.3,1,1);
    if (doratio){
	pad1->SetBorderMode(0);
   	pad1->Draw();
   	pad1->cd();
        pad1->SetTopMargin(.08);
	pad1->SetBottomMargin(0.0);
    }
    PlotPubHisto(sameHistos,params);
    c1->cd();
    TPad *pad2 = new TPad("pad2","pad2",0,0,1,0.3);
    //pad2->SetTopMargin(0);
    if (doratio){
    	pad2->SetBorderMode(0);
    	pad2->Draw();
    	pad2->cd();
        pad2->SetBottomMargin(.35);
	pad2->SetTopMargin(0.0);
	pad2->SetTicks(1,1);
        PlotDataBkgSumRatioOnPad(h_DATA,h_BKGSUM,params);
    }
    // Write out file:
    TString outfile=params->GetValue("Output.File", "dummy.eps");
    c1->cd();
    c1->SaveAs(outfile.Data());
}

///
/// /////////////////////////////////////////////////////////////////////////////////////
/// 

TObjArray GetSameHistos(TString hname,TEnv *params){
    TObjArray histos; histos.Clear();
    int num_files = params->GetValue ("Files.Number", 0);    
    for (int i = 1; i <= num_files; i++) {
	ostringstream baseName;
	baseName << "Files." << i << ".";
	TString bName (baseName.str().c_str());	
	TString fname (params->GetValue(bName+"Name", "bogus_file"));
	Double_t factor = params->GetValue(bName+"Factor", 1.0);
	TFile *f = new TFile (fname, "READ");
	TH1F * h = (TH1*) f->Get(hname);
	if (h){
	    h->Sumw2(); // you need this for the KS test
	    h->Add (h, factor-1.000);// Apply scaling factor (default=1.0)
	    histos.Add(h);
	    cout << "reading histo " << i << " factor=" << factor << " integral " << h->Integral() << endl;
	} else {
	    printf(" Could not find histogram %s in file %s \n",hname.Data(),fname.Data());
	    exit;
	}
    }
    return histos;
}

///
/// Sometimes we have h_jet_pt1_os, h_jet_pt2_os, h_jet_pt3_os but no h_jet_pt
/// This function allows to add all three histos in one file, for all files
/// 
TObjArray GetThreeSameHistos(TString h1name, TString h2name, TString h3name, TEnv *params){
    TObjArray histos; histos.Clear();
    int num_files = params->GetValue ("Files.Number", 0);    
    for (int i = 1; i <= num_files; i++) {
	ostringstream baseName;
	baseName << "Files." << i << ".";
	TString bName (baseName.str().c_str());	
	TString fname (params->GetValue(bName+"Name", "bogus_file"));
	Double_t factor = params->GetValue(bName+"Factor", 1.0);
	TFile *f = new TFile (fname, "READ");
	TH1F * h1 = (TH1*) f->Get(h1name);
	TH1F * h2 = (TH1*) f->Get(h2name);
	TH1F * h3 = (TH1*) f->Get(h3name);
	if (h1 && h2 && h3){
	    h1->Sumw2(); h2->Sumw2(); h3->Sumw2(); // you need this for the KS test	    
	    h1->Add (h1, factor-1.000);// Apply scaling factor (default=1.0)
	    h2->Add (h2, factor-1.000);
	    h3->Add (h3, factor-1.000);
	    h1->Add (h2);
	    h1->Add (h3);
	    histos.Add(h1);
	    //cout << "reading histo " << i << " factor=" << factor << " integral " << h->Integral() << endl;
	} else {
	    printf(" Could not find histogram %s in file %s \n",h1name.Data(),fname.Data());
	    exit;
	}
    }
    return histos;

}

///
/// /////////////////////////////////////////////////////////////////////////////////////
/// 

void PlotPubHisto(TObjArray histograms,TEnv *params){
    // This is a modification of the AddHistos macro
    // Number of histos to plot:
    Int_t ntot = histograms.GetEntries();
    
    // Before anything, do overflow and underflow:
    for(Int_t i = 0; i<ntot; i++){
	if(histograms[i]==0) {
	    cout<<"Error in AddHistos: histogram "<<i<<" is a NULL pointer!"<<endl;
	    return;
	}
	TH1F * hthis = (TH1F*) histograms[i];
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
    }

    // define a few additional line styles:
    gStyle->SetLineStyleString(5,"20 12 4 12");
    gStyle->SetLineStyleString(6,"20 12 4 12 4 12 4 12");
    gStyle->SetLineStyleString(7,"20 20");
    gStyle->SetLineStyleString(8,"20 12 4 12 4 12");
    gStyle->SetLineStyleString(9,"80 25");
    gStyle->SetLineStyleString(10,"50 10 10 10");
    gStyle->SetLineStyleString(17,"30 25");
    gStyle->SetLineStyleString(20,"60 20");
    gStyle->SetLineStyleString(21,"60 20 20 20");
    int lineStyle[20];
    for(int i=0;i<20;i++) {
	lineStyle[i]=i;
    }

    // figure out the number of signals
    Int_t nsig=1;
    if(params->Defined("Histo.Signal.Title.1")) nsig=1;
    if(params->Defined("Histo.Signal.Title.2")) nsig=2;
    if(params->Defined("Histo.Signal.Title.3")) nsig=3;
    if(params->GetValue("Histo.ShowSignalSeparately",0)==0) nsig=0;
    cout << " I will use nsig = " << nsig << " signal sources from the end of the config file" << endl;
    
    // Do the cumulative summing, except for the data
    TObjArray addedhistos; addedhistos.Clear();
    TObjArray signalhistos; signalhistos.Clear();
    TString sampletitles[20];
    Int_t nbkg=0;
    for(Int_t i = 1; i<ntot; i++){// i runs over histograms[i], so data is for i=0
	ostringstream baseSrcName;
	baseSrcName << "Files." << i+1 << ".";// Counting starts at 1: Files.1.Name: Data
	TString bSrcName(baseSrcName.str().c_str());
	// skip some if we want to show them as lines
	TString htitle=params->GetValue(bSrcName+"Title","");
	sampletitles[i-1]=htitle;
	if(params->GetValue("Histo.ShowSignalSeparately",0)==1 &&
	   // skip the last two if the signal title is not defined:
	   ( ( !(params->Defined("Histo.Signal.Title")||params->Defined("Histo.Signal.Title.1")) && i>=ntot-nsig) 
	     // skip the signal if the signal title is defined
	     || params->GetValue("Histo.Signal.Title",".")==htitle
	     || params->GetValue("Histo.Signal.Title.1",".")==htitle
	     || params->GetValue("Histo.Signal.Title.2",".")==htitle
	     || params->GetValue("Histo.Signal.Title.3",".")==htitle
	     ) ) {
	    TH1F * hthis = (TH1F*) histograms[i]->Clone();
	    cout<<" Found signal in location "<<i+1<<" with name "<<htitle.Data()<<endl;
	    signalhistos.Add(hthis);
	} else {
	    TH1F * hthis = (TH1F*) histograms[i]->Clone();
	    addedhistos.Add(hthis); // Fill in the new TObjArray with a copy 
	    //cout << " Adding bkg " << i << " " << htitle.Data() << " " << hthis->Integral() << endl;
	    // add all of the backgrounds
	    if (i>1) {// i=0 is the data, and we must start with the second
		      // background to add the previous
		TH1F * hprevious = (TH1F*) addedhistos[i-2];
		if ( hthis->GetXaxis()->GetNbins() != hprevious->GetXaxis()->GetNbins() ) {
		    // Protection against _whoran histogram. 
		    // We cannot add two histograms with different numbers of bins!
		    cout<<"Error in AddHistos: incompatible number of bins!"<<endl;  
		    return;
		}
		hthis->Add(hprevious); // Do the addition
		addedhistos.RemoveAt(i-1); // And substitute whatever we had 
		addedhistos.AddAt(hthis,i-1);
		nbkg++;
		//cout << "Substituing bkg " << i << " + " << i-1 << " in addedhistos["<< i-1 <<"]" << endl;
	    }	    
	} // end of: if adding histograms
    }
    cout << " nbkg = " << nbkg << endl; 
    
    // Rebin histos if necessary, but first calculate KS:
    TH1F *hbkg = (TH1F*) addedhistos[nbkg]; // bkgsum
    TH1F *h0=((TH1F*) histograms[0])->Clone(); // data (is always the first of the list)
    double KS = h0->KolmogorovTest(hbkg);
    double chi2ndf = h0->Chi2Test(hbkg, "UWUFOFCHI2/NDF");
    //cout << title.Data() << " KS = " << KS << " chi2/NDF = " << chi2ndf << endl;
    // Rebin? Set nrebin = 0 to NOT do rebinning. 
    // Will rebin only histos whose maximum x axis value exceeds 20. 
    // Anything with less will most probably be already made of integers, so no
    // need to rebin that! 
    Int_t nbinsx = h0->GetXaxis()->GetNbins();
    Int_t nbinsy = 100;
    Int_t nrebin = 5;
    if ( nbinsx > 750 && nbinsx <= 1000) nrebin = 30;
    if ( nbinsx > 400 && nbinsx <= 750 ) nrebin = 25;//20
    if ( nbinsx > 300 && nbinsx <= 400 ) nrebin = 25;//15
    if ( nbinsx > 200 && nbinsx <= 300 ) nrebin = 25;//15
    if ( nbinsx > 150 && nbinsx <= 200 ) nrebin = 10;//10
    if ( nbinsx > 100 && nbinsx <= 150 ) nrebin = 10;//10
    if ( nbinsx > 50 && nbinsx <= 100 )  nrebin = 10;//10
    if ( nbinsx > 20 && nbinsx <= 50 )   nrebin = 2;
    if ( nbinsx <= 20 ) nrebin = 1;  
    
    printf(" Saw nbins =%4i, rebinning by nrebin =%2i to final %3i bins \n",nbinsx,nrebin,int(nbinsx/nrebin));	

    if ( nrebin != 0 ) {
	h0->Rebin(nrebin); // data
	for (Int_t i = 0; i<=nbkg; i++){
	    TH1F * h = (TH1F*) addedhistos[i];
	    h->Rebin(nrebin);
	}
	for (Int_t i = 0; i<nsig; i++){
	    TH1F * h = (TH1F*) signalhistos[i];
	    h->Rebin(nrebin);
	}
    }
    // Pass data and bkgsum to main:
    h_DATA = (TH1F*) h0->Clone();
    h_BKGSUM = (TH1F*) addedhistos[nbkg]->Clone();

    if(params->GetValue("Histo.Preliminary","yes")==TString("paper")) textSize=0.07;
    if(params->Defined("Histo.TextSize")) textSize=params->GetValue("Histo.TextSize",0.07);
    
    // Now, check largest dimensions so that we can plot all histograms at once.
    Float_t xmin=9999., xmax=-9999., ymin=9999., ymax=-9999.;
    for(Int_t i = 0; i<=nbkg; i++){
	TH1F * h = (TH1F*) addedhistos[i];
	ostringstream baseSrcName;
	baseSrcName << "Files." << i+1 << ".";
	TString bSrcName(baseSrcName.str().c_str());
	
	TAxis *axis = h->GetXaxis();
	if( axis->GetXmin() < xmin ) xmin = axis->GetXmin();
	if( axis->GetXmax() > xmax ) xmax = axis->GetXmax();
	if( h->GetMinimum() < ymin ) ymin = h->GetMinimum();
	if( h->GetMaximum() > ymax ) ymax = h->GetMaximum();
    }
    ymax = TMath::Nint(ymax*1.25+1); // Make enough room for the big legend
    TString title = h0->GetTitle();
    //
    // now check if we should simply use the ranges that was passed to us.
    if(params->Defined("Histo.Xmin")) xmin = params->GetValue("Histo.Xmin",0.);
    if(params->Defined("Histo.Xmax")) xmax = params->GetValue("Histo.Xmax",0.);
    if(params->Defined("Histo.Ymin")) ymin = params->GetValue("Histo.Ymin",0.);
    if(params->Defined("Histo.Ymax")) ymax = params->GetValue("Histo.Ymax",0.);
    
    // Now make the frame:
    TH2F * frame = new TH2F("frame","",nbinsx,xmin,xmax,nbinsy,ymin,ymax);
    cout<<" frame has xmin "<<xmin<<", xmax "<<xmax<<", ymax "<<ymax<<endl;

    FrameStyle(frame,params);    

    // Could plot in log scale...
    //gPad->SetLogy();

    // finally: Draw the frame
    frame->Draw();
    
    // Draw the background histos, in reverse order so as to be able to follow the legend even in B&W:
    for(Int_t i=nbkg; i>=0; i--){
	TH1F * h = (TH1F*) addedhistos[i];
	h->SetStats(kFALSE);
	
	ostringstream baseSrcName;
	baseSrcName << "Files." << i+2 << ".";// to account for the data which is Files.1
	TString bSrcName(baseSrcName.str().c_str());
	Int_t hcolor=params->GetValue(bSrcName+"Color",1); 	
	h->SetLineColor(1);      
	h->SetFillColor(hcolor);
        if (i==nbkg) printf(" Data Yield = %5.2f ; SumBkg = %5.2f ; Data-SumBkg diff = %5.2f%% \n",
	   		     h0->Integral(),h->Integral(),(h0->Integral()-h->Integral())*100./h0->Integral());
	printf(" plotting bkg i=%2i name=%20.20s file=%2i integral=%5.1f color=%2i\n",
	       i,sampletitles[i].Data(),i+2,h->Integral(),hcolor);
	int fillStyle=params->GetValue(bSrcName+"FillStyle",1001);
	h->SetFillStyle(fillStyle);
	h->DrawCopy("Hist,Same");
    }
    //
    // and draw the signal ones
    // draw them in reverse order so that the last one will be on top.
    //for(Int_t i=ntot-3; i<ntot; i++){
    for(Int_t i=nsig-1; i>=0; i--){
	ostringstream baseSrcName;
	baseSrcName << "Files." << ntot+1-nsig+i << ".";
	TString bSrcName(baseSrcName.str().c_str());
	
	Int_t hcolor=params->GetValue(bSrcName+"Color",1);
	TH1F * h = (TH1F*) signalhistos[i];
       	//if (sampletitles[ntot-1-nsig+i].Contains("m_{H}=90"))        h->Add (h, 1.07874865  -1.000);
	
	printf(" plotting sig i=%2i name=%20.20s file=%2i integral=%5.1f color=%2i\n",
	       i,sampletitles[ntot-1-nsig+i].Data(),ntot+1-nsig+i,h->Integral(),hcolor);
	// create a white background around each line (helps readibility):
	TH1F *h1=h->Clone();
	h1->SetStats(kFALSE);
	h1->SetLineWidth(6);
	h1->SetLineColor(0);h1->SetFillColor(0);
	h1->SetLineStyle(1);
	h1->SetFillStyle(0);
	h1->Draw("HIST,SAME");
	// now draw the proper line:
	h->SetStats(kFALSE);
	h->SetLineWidth(6);
	h->SetLineColor(hcolor);h->SetFillColor(0);	
	Int_t hlinestyle = params->GetValue(bSrcName+"LineStyle",1);
	h->SetLineStyle(hlinestyle);
	h->SetFillStyle(0);	
	// finally, draw!
	h->Draw("HIST,SAME");      
    } // end of: drawing signal as separate lines
    
    // Data is special: 
    // change the default size of the little bar at the end of the error bar here
    gStyle->SetEndErrorSize(3);
    // also, maybe don't display the error bars along the X axis:
    //gStyle->SetErrorX(0);  // X error bars not displayed
    gStyle->SetErrorX(0.5);  // X error bars have width of a bin
    // now set the rest
    h0->SetMarkerSize(2);
    // if there are too many points (>80), keep the marker size smaller
    if(h0->GetNbinsX()>=50) h0->SetMarkerSize(1);
    //if(h0->GetNbinsX()>=100) h0->SetMarkerSize(1);
    h0->SetLineWidth(3);
    h0->SetMarkerStyle(8);
    h0->SetMarkerColor(1);
    h0->Draw("E1,SAME");

    //
    // Print D0 and lumi:
    //
    TText *t1 = new TText();
    t1->SetTextFont(62);
    t1->SetTextColor(1);   
    t1->SetNDC();
    t1->SetTextAlign(12);
    t1->SetTextSize(textSize);
    // histogram output filename
    TString oFileName=params->GetValue("Histo.Output.Filename","bogus.eps");
    TString prelim="D\328 Preliminary";
    if(oFileName.EndsWith(".eps")) {
	prelim="D\349 Preliminary L=3.7 fb^-1#";
    }
    else if(oFileName.EndsWith(".gif")) {
	prelim="D\328 Preliminary L=3.7 fb^-1#";
    }
    t1->DrawTextNDC(0.13,0.965,prelim.Data());

    // a counter of how much text we have added from the top
    int nAddTextLines=0;
    
    // any additional text?
    for(int iText=1;iText<20;iText++) {
	ostringstream baseTextName;
	baseTextName << "Histo.AddText." << iText;
	TString bTextName(baseTextName.str().c_str());
	if(params->Defined(bTextName)) {
	    // we are adding a line of text
	    TLatex *t2 = new TLatex();
	    t2->SetTextFont(62);
	    t2->SetTextColor(13);   
	    t2->SetTextAlign(32);
	    t2->SetNDC();
	    t2->SetTextSize(textSize);
	    TString addText(params->GetValue(bTextName,"."));
	    float x0=0.94;
	    float y0=0.96-(nAddTextLines)*0.05;
	    
	    // check if the user specified an alternative location for the text
	    if(params->Defined(bTextName+".X0")) x0=params->GetValue(bTextName+".X0",0.94);
	    if(params->Defined(bTextName+".Y0")) y0=params->GetValue(bTextName+".Y0",0.8);
	    t2->SetTextSize(textSize);
	    
	    // and increment the counter keeping track of how much we added,
	    // but only if the user didn't move the label around.
	    if(!params->Defined(bTextName+".X0")) nAddTextLines++;
	    printf("AddText %4.2f %4.2f %s\n",x0,y0,addText.Data());
	    
	    t2->DrawLatex(x0,y0,addText.Data()); 
	}
    }// end additional text
    
    // now draw the frame axis again so that we can see the tick marks
    frame->Draw("sameaxis");
    
    // Legend:
    TString showLegend(params->GetValue("Histo.ShowLegend","."));
    if( showLegend != "no" ){ 
	float lgdxmin=.65, lgdxmax=.90, lgdymin=.50, lgdymax=.91;
	if(showLegend=="yes" || showLegend=="right") {
	    
	} else if (showLegend=="left"){
	    lgdxmin=.16;
	    lgdxmax=.42;
	}
	TLegend *lgd = new TLegend(lgdxmin,lgdymin,lgdxmax,lgdymax); 
	// This line makes the legend transparent (not grey, ewwww!): 
	lgd->SetBorderSize(0); lgd->SetTextSize(textSize*0.9);// 10% less size 
	lgd->SetTextFont(62); lgd->SetFillColor(0);
	// Plot the legend in reverse order (but data goes first):
	NiceAddEntry(lgd,h0,params->GetValue("Files.1.Title","Data"),"PL");
	for(Int_t i = nbkg; i>=0; i--){
	    TH1F * h = (TH1F*) addedhistos[i];
	    TString lgd_entry= sampletitles[i]; // sampletitles runs from 0 (firstbkg) to ntot-1
	    NiceAddEntry(lgd,h,lgd_entry.Data(),"F");	
	}
	for(Int_t i = nsig-1; i>=0; i--){
	    TH1F * h = (TH1F*) signalhistos[i];
	    TString lgd_entry = sampletitles[i+nbkg+1]; // sampletitles runs from 0 (firstbkg) to ntot-1
	    ostringstream basefactor;
	    basefactor << "Files." << i+nbkg+3 << "." << "Factor";
	    TString timesfactor(basefactor.str().c_str());
	    Double_t nprod = params->GetValue(timesfactor,1.000);
	    if (nprod != 1.0 ) lgd_entry.Form("%s x%2.0f",lgd_entry.Data(),nprod);
	    //cout << i+nbkg+3 << " " << nprod << " " << lgd_entry.Data() << endl;
	    NiceAddEntry(lgd,h,lgd_entry.Data(),"L"); 
	}
	lgd->Draw("SAME");	
    }// show legend


    // Draw the KS:
    TLatex *ks = new TLatex();
    ks->SetTextFont(62);
    ks->SetTextColor(1);   
    TString ks_val = Form("KS = %3.2f",KS);
    ks->SetTextAlign(11); ks->SetTextSize(0.03); // ks->SetTextAngle(90); 
    ks->DrawTextNDC(0.82,0.93,ks_val.Data());
    TString chi2_val = Form("#chi^{2}/ndf = %3.1f",chi2ndf);
    //ks->SetNDC(true); ks->DrawLatex(0.82,0.97,chi2_val.Data());
    //
    // Voila!
    //
}

///
/// /////////////////////////////////////////////////////////////////////////////////////
/// 

void PlotDataBkgSumRatioOnPad(TH1F* hdata, TH1F* hbkgsum, TEnv* params){
    hdata->SetStats(0);
    hdata->Divide(hbkgsum);
    FrameStyle(hdata,params); // Make style same as main pad
    hdata->SetYTitle("Data/Bkg."); //hdata->SetYTitle("#splitline{Data/Background}{ratio}");
    hdata->SetTitle("");
    hdata->SetMinimum(-1.); // This should be somewhere close to 1.0
    hdata->SetMaximum(3.);
    hdata->GetYaxis()->SetNdivisions(5); // Different from main pad
    //
    // OK, all the style is different, because sizes are percentage of pad, and this second pad is smaller.
    float axisLabelSize=0.15; 
    hdata->SetLabelSize(axisLabelSize,"X");
    hdata->SetTitleSize(axisLabelSize,"X");
    hdata->SetLabelSize(axisLabelSize,"Y");
    hdata->SetTitleSize(axisLabelSize,"Y");
    hdata->SetTitleOffset(0.42,"Y");
    //
    hdata->GetXaxis()->SetTickLength(0.06);
    //gStyle->SetErrorX(0);  // X error bars not displayed
    gStyle->SetErrorX(0.5);  // X error bars have width of a bin
    gStyle->SetEndErrorSize(3);
    hdata->SetMarkerSize(2);
    // if there are too many points (>80), keep the marker size smaller
    if(hdata->GetNbinsX()>=50) h0->SetMarkerSize(1);
    hdata->SetLineWidth(3);
    hdata->SetMarkerStyle(8);
    hdata->SetMarkerColor(1);
    hdata->Draw("E1P");
    TLine* l = new TLine(hdata->GetXaxis()->GetXmin(),1.0,hdata->GetXaxis()->GetXmax(),1.0);
    l->SetLineStyle(2);
    l->Draw(); 
    printf(" PlotDataBkgSumRatioOnPad: data=%6i bkgsum=%6.3f \n",hdata->Integral(),hbkgsum->Integral());
}

///
/// /////////////////////////////////////////////////////////////////////////////////////
/// 

void FrameStyle(TH1* frame, TEnv* params){
    // get the x- and y-axis titles
    TString ytitle=params->GetValue("Histo.YTitle","");
    if ( params->Defined("Histo.XTitle")) {
	frame->SetXTitle(params->GetValue("Histo.XTitle",""));
    } else {
	frame->SetXTitle(h_DATA->GetTitle());
    }
    frame->SetYTitle(ytitle.Data()); 
    // also set the text size for the X and Y axis titles and numbers
    // do this globally for the style we are using
    float axisLabelSize=textSize;
    //frame->GetXaxis()->SetLabelSize(axisLabelSize);
    //frame->GetYaxis()->SetLabelSize(axisLabelSize);
    //frame->GetXaxis()->SetTitleSize(axisLabelSize);
    //frame->GetYaxis()->SetTitleSize(axisLabelSize);
    frame->SetLabelSize(axisLabelSize,"XY");
    frame->SetTitleSize(axisLabelSize,"XY");
    frame->GetXaxis()->SetTickLength(0.02);
    frame->GetYaxis()->SetTickLength(0.02);

    frame->SetStats(false);
    // reduce the axis title offset if the fonts are very large
    if(textSize>0.055) frame->SetTitleOffset(1.0,"XY");
    // also change the X axis title offset to move it farther away from the numbers
    if(params->Defined("Histo.XTitle.Offset")) {
	float xtitoffset=params->GetValue("Histo.XTitle.Offset",1.0);
	frame->SetTitleOffset(xtitoffset,"XY");
    }
    // set the axes divisions
    frame->GetXaxis()->SetNdivisions(505,true);
    if(params->Defined("Histo.XNdivisions")) frame->GetXaxis()->SetNdivisions(params->GetValue("Histo.XNdivisions",505),kTRUE);
    if(params->Defined("Histo.YNdivisions")) frame->GetYaxis()->SetNdivisions(params->GetValue("Histo.YNdivisions",505),kTRUE);
    
    // make sure the X axis title and Y axis title are in black!
    frame->GetXaxis()->SetTitleColor(1);
    frame->GetYaxis()->SetTitleColor(1);
}

///
/// /////////////////////////////////////////////////////////////////////////////////////
/// 

void NiceAddEntry(TLegend* leg, TH1F* hist, TString name, TString type){
// The stupid lgd->SetTextAlign(13) does not work on the entries!!! 
// You have to add the entry in this convoluted way for the text to be properly displayed 
// if it has some math like super or subscript or a #bar{}
    TLegendEntry *entry=leg->AddEntry(hist,name.Data(),type.Data());
    entry->SetTextAlign(12); // default H: left V: center
    if ( name.Contains("#bar") || name.Contains("^{")) {
	//entry->SetTextAlign(11); // special case for ShowHistosLines
    	entry->SetTextAlign(13); // H: left V: top
    } else if (name.Contains("_{")) {
	entry->SetTextAlign(11); // H: left V: bottom
    }
}
