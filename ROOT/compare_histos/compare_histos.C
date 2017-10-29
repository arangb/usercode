///
/// compare_histos.C
///
/// 22/3/2006 Yann Coadou (yann@fnal.gov)
///           Updated from a macro by Aran Garcia-Bellido to go into top_cafe.
///           Called from compare_files.sh
/// 2008 Updated by Aran Garcia-Bellido to do the ratio of data vs bkgsum with Data_SumBkg_RatioHistos.C
/// 2008 Updated by Aran Garcia-Bellido to print yields in nice table (analysis dependent)
/// 2012 Updated by Skyler Kasko and Aran Garcia-Bellido to be used with TBranches, and not just TH1s
/// 2017 AGB: Updated to run in ROOT 6. If we encounter a branch that is a folder, it is skipped, unless it's of type 
///           vector<TLorentzVector> in which case we plot its: @.size() 
///
///  Given a list of directories -- which have the same structure -- traverse them
/// making comparison plots (put all the plots on the same canvas) and write them out.
///
///  Sample config file can be found in the compare_files.list file.
/// 
#include <stdio.h>
#include "AddHistos.C"
#include "OverlayHistos.C"
#include "Data_SumBkg_RatioHistos.C"


class file_info : public TObject {
public:
  inline file_info (void)
    : _d(0), _offset(0), _startOffset(0), _dir_init(false), _factor(0), _color(1), _normalization(-1), _channel(-1), _style(0), _with_errors(0)
  {}
  inline file_info (const file_info &f)
    : _d(f._d), _offset(0), _startOffset(0), _dir_init (false), _title(f._title),
      _factor(f._factor),
      _color(f._color),
      _normalization(f._normalization),
      _channel(f._channel),
      _style(f._style),
      _with_errors(f._with_errors)
  {
  }
  inline const char *GetName (void) {
    return _d->GetName();
  }
  TDirectory *_d;
  int _offset;
  int _startOffset;
  TString _title;
  bool _dir_init;
  TList _dirList;
  double _factor;
  int _color;
  double _normalization;
  int _channel;
  int _style;
  int _with_errors;

  TString TranslateDirName (TString name);
  TString GetNormalizedName (void);
  int GetNumberDirectories (void);
  void InitDirList (void);
  inline TDirectory *GetDirectory (int index) {
    InitDirList ();
    return (TDirectory*) _dirList.At(index);
  }

};


TObjArray *LoadFiles (TEnv *params);

TEnv *LoadParameters(TString configfile="");

void PlotFiles (TObjArray *list, TDirectory *output, TEnv *params);
void PlotHistoList (TObjArray &histograms, TObjArray *fileinfo, TDirectory *output_dir, TEnv *params, int *palette, int *channel, int *styles, int *with_errors);
void PrintYields(TObjArray histograms, Int_t nsig=0);
TString Make_LaTeX_Title(TString title);

void compare_histos(TString configfile="")
{
  TEnv *params = LoadParameters(configfile);

  TObjArray *info = LoadFiles (params);
  if (info->GetEntriesFast() == 0) {
    cout << "No files found in input parameter files compare_files.list!" << endl;
    return;
  }

  //gROOT->SetStyle("Plain"); gROOT->ForceStyle();
  gROOT->ProcessLine(".x ~/public/rootlogon.C");
  gStyle->SetPadBottomMargin(.150); // Need this for subscripts

  TString outputFileName = params->GetValue("Output.File", "bogus.root");
  TFile *output = new TFile (outputFileName, "RECREATE");

  PlotFiles (info, output, params);
  output->Write();
  output->Close();
}

/// Load the input file -- compare_files.defaults and then the input file compare_files.list, or the file specified as argument
TEnv *LoadParameters (TString configfile)
{
  TEnv *result = new TEnv ("compare_files");
  result->ReadFile ("compare_files.defaults", kEnvChange);
  if (configfile == "")
    result->ReadFile ("compare_files.list", kEnvChange);
  else
    result->ReadFile (configfile, kEnvChange);

  return result;
}

/// LoadFiles -- Given the parameters, attempt to open all specified files
TObjArray *LoadFiles (TEnv *params)
{
  TObjArray *result = new TObjArray ();

  int num_files = params->GetValue ("Files.Number", 0);

  for (int i = 1; i <= num_files; i++) {
    ostringstream baseName;
    baseName << "Files." << i << ".";
    TString bName (baseName.str().c_str());

    TString fname (params->GetValue(bName+"Name", "bogus_file"));
    TString base (params->GetValue(bName+"BaseDir", "."));

    TFile *f = new TFile (fname, "READ");
    f->cd(base);

    file_info *info = new file_info ();
    info->_d = gDirectory;
    info->_title = params->GetValue(bName+"Title", "BogusStuff");

    ///
    /// Lets say there is a dir sitting in the middle of the first sequence (#5)
    /// and not in the second sequence. On the first sequence you would then
    /// specify that you wanted to start offset at 5, and the offset would be 1.
    ///

    info->_offset = params->GetValue(bName+"DirOffset", 0);
    info->_startOffset = params->GetValue(bName+"StartOffset", 0);

    info->_factor = params->GetValue(bName+"Factor", 1.0);

    info->_color = params->GetValue(bName+"Color", 1);
    info->_channel = params->GetValue(bName+"Channel", -1);
    info->_style = params->GetValue(bName+"Style", 0);
    info->_with_errors = params->GetValue(bName+"PlotErrors", 0);
    info->_normalization = params->GetValue(bName+"Normalization", -1.0);
    if (info->_normalization == -1) {
      info->_normalization = params->GetValue("Global.Normalization", -1.0);
    }
    result->Add(info);
  }

  return result;
}

/// PlotFiles -- do the work of traversing the plots! Recursively, even!
void PlotFiles (TObjArray *list, TDirectory *output_dir, TEnv *params)
{
  ///
  /// Use the first file in the list to run our deep-searching stradegy.
  ///
  file_info *master_file = (file_info*) list->At(0);
  int ndir = master_file->GetNumberDirectories();
  cout << "Processing file " << master_file->GetName();
  if (ndir > 0) {
    cout << " (" << ndir << " subdirs)";
  }
  cout << endl;

  ///
  /// First, process directories
  ///

  for (int didx = 0; didx < ndir; didx++) {
    ///
    /// Go down a level by making a copy...
    ///

    TObjArray newSubDirs;
    newSubDirs.SetOwner (true);
    TString f_name;
    file_info *first = 0;
    for (int i = 0; i < list->GetEntriesFast(); i++) {
      file_info *old = (file_info*) (*list)[i];
      file_info *newinfo = new file_info (*old);
	
      TDirectory *new_d = old->GetDirectory(didx);
      if (new_d == 0) {
	cout << "File " << i << " doesn't have the " << didx << " directory! This is bad!" << endl;
      }
      newinfo->_d = new_d;

      bool good = true;
      if (first == 0) {
	first = newinfo;
	f_name = first->GetNormalizedName();
      } else {
	TString s_name (newinfo->GetNormalizedName());
	if (f_name != s_name) {
	  cout << "File " << i << " doesn't have matching dir." << endl;
	  cout << "  Expected " << first->GetNormalizedName() << " but found "
	       << newinfo->GetNormalizedName() << endl;
	  cout << "  Skipping..." << endl;
	}
      }
      if (good) {
	newSubDirs.Add(newinfo);
      }
    }
    if (newSubDirs.GetEntriesFast() > 1) {
      TDirectory *subdir = output_dir->mkdir(first->GetName());
      PlotFiles (&newSubDirs, subdir, params);
    }
  }

  ///
  /// Now -- do the objects in the directory!
  ///
  master_file->_d->cd();
  TIter nextkey (master_file->_d->GetListOfKeys());
  TKey *key;
  TString match_only_histo = params->GetValue("PlotOnlyHistosWithName", "");
  cout << " Will only plot histograms or leaves containing the string: " << match_only_histo << endl;
  while (( key = (TKey*)nextkey() )) {
      TObject *obj = key->ReadObj();
      TString title = obj->GetName();//AGB temp 
      //if (!title.Contains("_ss")){//AGB only plot OS and sums
      if (obj->IsA()->InheritsFrom("TH1") && title.Contains(match_only_histo)) {	
	/// Ok -- need to make a plot!
	TObjArray sameHistos;
	bool good = true;
	int palette[100];
	int channel[100];
        int styles[100];
	int with_errors[100];
	for (int iFile = 0; iFile < list->GetEntriesFast(); iFile++) {
		file_info *info = (file_info*) (*list)[iFile];
		// cout << "Directory: " << info->_d->GetName() << endl;
		TH1 *h = (TH1*) info->_d->Get(obj->GetName());
		if (h) {
		if (info->_normalization > 0) {
			if (h->Integral() != 0) {
			h->Scale(info->_normalization/h->Integral());
			}
		} else {
			h->Add (h, info->_factor-1.0);
		}
		sameHistos.Add(h);
		// cout << " Added histo " << h->GetName() << endl;
		palette[iFile] = info->_color;
		channel[iFile] = info->_channel;
                styles[iFile] = info->_style;
		with_errors[iFile] = info->_with_errors;
		} else {
		good = false;
		cout << "Missing plot " << obj->GetName() << " in file " << info->GetName() << endl;
		}
	}// loop over files
	if (good) PlotHistoList (sameHistos, list, output_dir, params, palette, channel, styles, with_errors);
      } // key is TH1 and good name
  }//loop on keys

  ///
  /// Now do a TTree if given in the params file.
  /// TTrees are completely different from TH1's, they don't have TKeys, but TBranches. 
  /// It is assummed that all files contain the same TTree structure
  /// 
  TString tree_name = params->GetValue("Tree.TreeName","");
  if (tree_name == "") return;
  TString weight_for_leaves = params->GetValue("Tree.LeafWeight", "weight*lumiweight");
  TString cut_for_leaves = params->GetValue("Tree.Cut", "");
  if (cut_for_leaves != "") {
	weight_for_leaves = "(" + cut_for_leaves + ")*" + weight_for_leaves;
	cout << " This cut will be applied on the tree: " << weight_for_leaves << endl;
  }
  // We do for example: "(jetpt1>30&&jetpt2>30)*weight"
  TCanvas *c1 = 0; // We'll need this for drawing a Branch from a Tree into a histogram
  if (c1 == 0) c1 = new TCanvas ("c1");
  c1->Clear();

  master_file->_d->cd();
  TTree *master_tree = (TTree*) master_file->_d->Get(tree_name); 
  TIter nextbranch = master_tree->GetListOfBranches();
  TBranch *branch;
  while (( branch = (TBranch*) nextbranch() )) {
     TString brtitle = branch->GetName(); // brtitle is the name of the branch
     if (brtitle.Contains(match_only_histo)) {
	 TString brtype = branch->GetLeaf(branch->GetName())->GetTypeName();
	 if ( branch->IsFolder() ){
	    if ( brtype == "vector<TLorentzVector>" ) {
		cout << " Found folder branch: " << branch->GetName() << " is of type: " << brtype << endl; 
	    } else {
		continue;
	    }
	 }
	 TObjArray sameHistos;
	 bool good = true;
	 int palette[100];
	 int channel[100];
         int styles[100];
	 int with_errors[100];

	 int nbins=20;
         double xmin=0.;
	 double xmax=1.;
	 for (int iFile = 0; iFile < list->GetEntriesFast(); iFile++) {
		  file_info *info = (file_info*) (*list)[iFile];
		  // cout << "Directory: " << info->_d->GetName() << endl;
		  TTree *tree = (TTree*) info->_d->Get(tree_name);
                  //cout << tree->GetEntriesFast() << endl;
		  TH1* h;
		  if (iFile==0) {// Set histogram dimensions from the first source. 
		        if (brtype=="vector<TLorentzVector>") {
			    tree->Draw(TString(brtitle + "@.size()>> h0_"+brtitle),weight_for_leaves);
			} else {
			    tree->Draw(TString(brtitle + ">> h0_"+brtitle),weight_for_leaves);
			    //tree->Draw("etalep1>>h0_etalep1","weight"); Draws leaf in new histogram
			}
			h = (TH1F*) gDirectory->Get(TString("h0_"+brtitle)); // h points to the new histogram
			if (h){
			       nbins = h->GetNbinsX();
			       xmin = h->GetXaxis()->GetXmin();
			       xmax = h->GetXaxis()->GetXmax();
                        }
		  } else {
			h = new TH1F("h","h",nbins,xmin,xmax); // Important to use "h" for Title and Name
			if (brtype=="vector<TLorentzVector>") {
			    tree->Draw(TString(brtitle + "@.size()>> h"),weight_for_leaves);
			} else {
			    tree->Draw(TString(brtitle + ">>h"),weight_for_leaves); // That way, h is unequivocal
			}
		  }
		  if (h) {
		      if (info->_normalization > 0) {
			  if (h->Integral() != 0) {
			      h->Scale(info->_normalization/h->Integral());
			  }
		      } else {
			  h->Add (h, info->_factor-1.0);
		      }
                      h->SetName(brtitle);
		      h->SetTitle(brtitle);
		      if (brtype=="vector<TLorentzVector>") h->SetTitle(brtitle+"SizeFromBranch");
		      sameHistos.Add(h);
		      //cout << iFile << "  Added histo " << h->GetName() << " " << h->Integral() << endl;
		      palette[iFile] = info->_color;
		      channel[iFile] = info->_channel;
                      styles[iFile] = info->_style;
		      with_errors[iFile] = info->_with_errors;
		  } else {
		      good = false;
		      cout << "Missing branch " << brtitle << " in file " << info->GetName() << endl;
		  }
	}// loop over files
	if (good) PlotHistoList (sameHistos, list, output_dir, params, palette, channel, styles, with_errors);
     } // good name
  }//loop over branch names
}

void PlotHistoList (TObjArray &histograms, TObjArray *fileinfo, TDirectory *output_dir, TEnv *params, int *palette, int *channel, int *styles, int *with_errors)
{
  ///
  /// We'll just re-use a canvas to save with processing time
  ///

  TCanvas *c1 = 0;
  if (c1 == 0) c1 = new TCanvas ("c1");
  c1->Clear();

  ///
  /// Get the model histogram we'll be using.
  ///

  TH1 *first_guy = (TH1*) histograms[0];
  c1->SetName (first_guy->GetName());
  c1->SetTitle (first_guy->GetTitle());
  TString htitle (first_guy->GetTitle());
  TString xtitle (first_guy->GetXaxis()->GetTitle());
  TString hname (first_guy->GetName());

  ///
  /// Use the titles given to us so they show in the
  /// histogram legend
  ///

  for (int i = 0; i < histograms.GetEntriesFast(); i++) {
    TH1 *h = (TH1*) histograms[i];
    //file_info *f = (*fileinfo)[i];
    file_info *f = (file_info*)(*fileinfo)[i];
    h->SetName (f->_title);
    // h->SetStats(kFALSE);
  }

  ///
  /// Plot them all
  ///

  TString type = params->GetValue("Histo.Type", "Overlay");
  int goodNbins = params->GetValue("Histo.GoodNbins", -1);
  if (type == "OverlayLines" || type == "Legend") {
      if (histograms[0]->InheritsFrom(TH2::Class())) {
	  OverlayHistos2D (c1, histograms, htitle, "", "", palette);
      } else {
	  OverlayHistos("Legend",histograms, xtitle, "", "", palette, styles, with_errors, goodNbins);
      }
  } else if (type == "OverlayLinesWithStatBoxes" || type == "StatBox" || type == "Overlay") {
      if (histograms[0]->InheritsFrom(TH2::Class())) {
	  OverlayHistos2D (c1, histograms, htitle, "", "", palette);
      } else {
	  OverlayHistos("StatBox",histograms, htitle, "", "", palette, styles, with_errors, goodNbins);
      }
  } else if (type == "Add") {
      if (! histograms[0]->InheritsFrom(TH2::Class())) {
	  int nsig=1; // the number of signal histograms at the end of the array (they will be plotted as lines).
	  AddHistos(histograms, htitle, "", "", nsig, palette);
      }
  } else if (type == "Ratio") {
      if (! histograms[0]->InheritsFrom(TH2::Class())) {
	  Data_SumBkg_RatioHistos(histograms, htitle, "", "",palette);
      }
  } else if (type == "PrintYields") {// if we run it like this, then we can access also the TTrees.
      /// IMPORTANT:
      /// We need to select here the name of a histogram that has one entry per event.
      if (htitle == "metSign"){ // skip histos until we find the one we want.
          int nsig=1; // the number of signal histograms at the end of the array (they won't be added as bkg).
          PrintYields(histograms,nsig);
          // We want to exit the first time we call it, since we don't need to do this for every plot.
          return;
      }
  } else {
      cout << "Unknown type: " << type.Data() << ". Don't know what to do. " << endl;
  }
  
  ///
  /// Now, write it out to a file! Also remove some temp stuff,
  /// otherwise root will complain a lot!
  ///

  TDirectory *old = gDirectory;
  output_dir->cd();
  c1->Write (hname);
  old->cd();

  if (gDirectory->FindObject("frame")) {
    delete gDirectory->FindObject("frame");
  }
}

///
/// InitDirList()
///
///  Get our dirlist setup
///
void file_info::InitDirList (void)
{
  if (_dir_init) {
    return;
  }
  _dir_init = true;

  _d->cd();
  TIter nextKey (_d->GetListOfKeys());
  TKey *key;
  int index = 0;
  while (( key = (TKey*)nextKey() )) { 
    TObject *obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom("TDirectory")) {
      ///
      /// If we are beyond _startOffset, then add the offset to see where we should be...
      ///
	bool useful = true;
	if (index >= _startOffset) {
	    useful = index >= (_startOffset + _offset);
	}
	if (useful) {
	    _dirList.Add(obj);
	}
	index++;
    }
  }
}

///
/// GetNumberDirectories
///
///  How many directories do we have floating around in here?
///
int file_info::GetNumberDirectories (void)
{
  InitDirList();
  return _dirList.GetSize();
}

///
///  Strip the leading index off...
///
TString file_info::GetNormalizedName (void)
{
  TString dirname (_d->GetName());
  char basename[2000];
  int index;
  sscanf(dirname.Data(), "%d%s", &index, basename);
  return TString (basename);
}

///
/// Sometimes the names have to move indicies for some odd
/// reason. Deal with it here.
///
TString file_info::TranslateDirName (TString name)
{
  if (_offset == 0) {
    return name;
  }

  ///
  /// Get the info, see if it is relavent!
  ///

  char basename[2000];
  int index;
  sscanf(name.Data(), "%d%s", &index, basename);

  if (index < _startOffset) {
    return name;
  }

  char finalname[2000];
  sprintf (finalname, "%02d%s", index + _offset, basename);

  cout << "Looking at " << finalname << endl;

  return finalname;
}


///
/// Print_Yields: simple table of yields
///
void PrintYields(TObjArray histograms, Int_t nsig){
    const Int_t ntot = histograms.GetEntries();
    float bkg_sum=0.0;
    float sig_sum=0.0;
    float bkg_err_sum=0.0;
    float sig_err_sum=0.0;
    float data=0.0;
    
    // Prepare LaTex file:
    FILE *out; out = fopen("yield.tex","w");
    fprintf(out,"\\setlength{\\tabcolsep}{10pt}\n");
    fprintf(out,"\\begin{tabular}{lr@{ $\\pm$ }l} \\hline \n");
    // Loop over all files
    printf("%24s    %-15s \n","Source","Yield");    
    fprintf(out,"%24s & \\multicolumn{2}{c}{%s} \\\\ \\hline \n","Source","Yield");
    for(Int_t i = ntot-1; i>ntot-nsig-1; i--){//signals
      TH1F * h = (TH1F*) histograms[i];
      float yield = h->Integral();
      float err = 0.;
      if (h->GetEntries()>0) err = yield/TMath::Sqrt(h->GetEntries());
      printf("%24s    %6.2f +- %-6.2f \n",h->GetName(),yield,err);
      fprintf(out,"%24s    %6.2f +- %-6.2f \n",h->GetName(),yield,err);
      sig_sum+=yield;
      sig_err_sum+=err*err;
    }
    if (nsig>1){
      printf ("%24s  %6.2f +- %-6.2f \n","SignalSum",sig_sum,TMath::Sqrt(sig_err_sum));
      fprintf (out, "\\hline \n");
      fprintf (out,"%24s  %6.2f +- %-6.2f ","SignalSum",sig_sum,TMath::Sqrt(sig_err_sum));
      fprintf (out, "\\hline \n");
    }
    for(Int_t i = ntot-nsig-1; i>0; i--){// bkgs only
      TH1F * h = (TH1F*) histograms[i];
      float yield = h->Integral();
      float err = 0.;
      if (h->GetEntries()>0) err = yield/TMath::Sqrt(h->GetEntries());
      printf("%24s    %6.2f +- %-6.2f \n",h->GetName(),yield,err);
      fprintf(out,"%24s    %6.2f +- %-6.2f \n",h->GetName(),yield,err);
      bkg_sum+=yield;
      bkg_err_sum+=err*err;
    }
    if (ntot-nsig>0){
      printf ("%24s    %6.2f +- %-6.2f \n","BackgroundSum",bkg_sum,TMath::Sqrt(bkg_err_sum));
      fprintf (out, "\\hline \n");
      fprintf (out,"%24s  %6.2f +- %-6.2f ","BackgroundSum",bkg_sum,TMath::Sqrt(bkg_err_sum));
      fprintf (out, "\\hline \n");
    }
    // Finally, data:
    TH1F * h = (TH1F*) histograms[0];
    printf ("%24s    %-6f \n",h->GetName(),h->GetEntries()); /// Use GetEntries instead of Integral
    fprintf (out,"%24s  %6f ",h->GetName(),h->GetEntries());
    fprintf (out, "\\hline \n");
}

TString Make_LaTeX_Title(TString title){
    
    TString goodtitle;
    if (title.Contains("t#bar{t}"))                    {goodtitle = "$t\\bar{t}$";}
    else if (title.Contains("Z #rightarrow ee"))       {goodtitle = "$Z \\to$~ee";}
    else if (title.Contains("Z #rightarrow #tau#tau")) {goodtitle = "$Z \\to \\tau\\tau$";}
    else {goodtitle = title;}
    
    return goodtitle;
}

