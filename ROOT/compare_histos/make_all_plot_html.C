// this macro generate an html file with all the plot contained in the root 
// file passed as argument. You have to create a directory where you wish to 
// save all the gif file created and then you have to pass this directory as
// argumenten to the macro. E.g. I have created a  directory named "figures"
// in root:
//.x make_all_plot_html.C("../work/schannel-wmunuHist.root", "figures")
// You have to move your html file produced and the directory with figure in 
// the same place. 
//
// Mods:
//   Now follows the more complex directory structure in the root files
//   Will create teh directory if it sin't already there.

void make_all_plot_html(TString file,TString path)
{
  ///
  /// Load up the macros to make sure we can interpret everything
  /// in there!
  ///

  //  gInterpreter->ExecuteMacro ("topd0root_analysis/compile.C");

  ///
  /// Setup
  ///

  TFile *f = new TFile(file);
  TEnv *params = new TEnv ("make_all_plot_html");
  //  params->ReadFile ("topd0root_analysis/macros/make_all_plot_html.params", kEnvChange);
  params->ReadFile ("make_all_plot_html.params", kEnvChange);

  gStyle->SetPalette(1);

  ///
  /// Ok -- we are ready to recursively process this stuff!
  ///

  html_a_directory (f, path, params);
}

///
/// Process a directory recursively.
///
void html_a_directory (TDirectory *f, TString &path, TEnv *params)
{
  const TCanvas *c_h1 = 0;
  if (c_h1 == 0) {
    int x = params->GetValue("H1.XSize", 150);
    int y = params->GetValue("H1.YSize", 100);
    c_h1 = new TCanvas ("c_h1", "1d plots", x, y);
  }

  ///
  /// Check how to make gif plots
  ///

  char command[512];
  sprintf(command, "which pstoimg &> /dev/null");
  bool UsePstoimg = ! system(command);

  ///
  /// Generate the output directory
  ///

  gSystem->MakeDirectory (path);

  ///
  /// Get the html started
  ///

  ofstream html (path + "/index.html");
  html << "<html><head><title>" << f->GetName() << "</title></head>" << endl;
  html << "<body>" << endl;
  html << "<h1>" << f->GetName() << "</h1>" << endl;
  cout << "Processing directory " << f->GetName() << endl;

  ///
  /// Now loop over all the keys in the directory
  ///

  f->cd();
  TList *objlist = f->GetListOfKeys();
  objlist->Sort(); // order alphabetically, instead of order in which they were written
  TIterator *itr = objlist->MakeIterator();
  TKey *key;
  while ((key = static_cast<TKey*>(itr->Next())) != 0) {
    TObject *obj = key->ReadObj();

    if (obj->IsA()->InheritsFrom("TDirectory")) {
      TDirectory *d = static_cast<TDirectory*>(obj);
      html << "<br><a href=\"" << d->GetName() << "/\">" << d->GetName() << "</a>" << endl;
      html_a_directory (d, path + "/" + d->GetName(), params);
    }

    else if (obj->IsA()->InheritsFrom("TObjString")) {
      TObjString *s = static_cast<TObjString*>(obj);
      html << "<p><h2>" << key->GetName() << "</h2>" << endl;
      html << "<blockquote><pre>" << static_cast<char*>(s->GetString())
	   << "</pre></blockquote></p>"
	   << endl;

    }

    //    else if (obj->IsA()->InheritsFrom("CutFlowTable")) {
    //      CutFlowTable *c = static_cast<CutFlowTable*> (obj);
    //      
    //      html << "<p><h2>" << key->GetName() << "</h2>" << endl;
    //
    //      CFTPrinterHTML txt (html);
    //      f->cd();
    //      c->PrintGlobal (txt, "All Events", "");
    //      html << "</p>" << endl;
    //    }

    else if (obj->IsA()->InheritsFrom("TCanvas")) {
      TCanvas *cnv = static_cast<TCanvas*>(obj);

      cnv->Draw();
      cnv->SaveAs(path + "/" + key->GetName() + ".eps");
      if (UsePstoimg) {
	sprintf(command, "pstoimg -type=gif %s &> /dev/null",(path + "/" + key->GetName() + ".eps").Data());
	if (system(command) != 0) {
	  cout<<"Could not convert to gif: "<<path + "/" + key->GetName() + ".eps"<<endl;
	  abort();
	}
      } else cnv->SaveAs(path + "/" + key->GetName() + ".gif");

      cnv->Close();

      html << "<p><a href=\"" << key->GetName() << ".eps\">";
      html << "<img src=\"" << key->GetName() << ".gif\">";
      html << "</a> <br> " << key->GetName() << ".gif </p>" << endl;								   
    }

    else if (obj->IsA()->InheritsFrom("TH1") && !(obj->IsA()->InheritsFrom("TH2"))) {
      TH1 *h = static_cast<TH1*> (obj);

      c_h1->cd();
      h->Draw();
      c_h1->SaveAs(path + "/" + key->GetName() + ".eps");
      if (UsePstoimg) {
	sprintf(command, "pstoimg -type=gif %s &> /dev/null",(path + "/" + key->GetName() + ".eps").Data());
	if (system(command) != 0) {
	  cout<<"Could not convert to gif: "<<path + "/" + key->GetName() + ".eps"<<endl;
	  abort();
	}
      } else c_h1->SaveAs(path + "/" + key->GetName() + ".gif");

      html << "<p><a href=\"" << key->GetName() << ".eps\">";
      html << "<img src=\"" << key->GetName() << ".gif\">";
      html << "</a> <br> " << key->GetName() << ".gif </p>" << endl;
    }
    f->cd();
  }

  ///
  /// Done!
  ///

  html << "</body></html>" << endl;
  html.close();
}








