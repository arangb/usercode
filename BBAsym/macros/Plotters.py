from ROOT import AddressOf, TFile, TCanvas, gDirectory, TTree, TH1F, TH1I, TLegend, TGraphErrors, gStyle, gInterpreter, TCut, TLorentzVector
from array import array
from AsymmetryUtils import *

def MakeHisto(var,mcweight,name,title,xtitle,nbin,xmin,xmax):
	# Takes a list of numbers and makes a histo
	# The title is important for legends, in case there are more than one histo
	hist=TH1F(name,title,nbin,xmin,xmax)
	hist.SetXTitle(xtitle)
	hist.SetYTitle('Yield')
	hist.SetLineWidth(2)
	#hist.SetStats(0)
	for i in xrange(len(var)):
		hist.Fill(var[i],mcweight[i])
	
	# Make sure we count the number of entries			
	entries=hist.GetEntries()				
	# include the overflow/underflow bins:
	numbins = hist.GetNbinsX() #this is the last bin plotted
	hicontent = hist.GetBinContent(numbins)
	overflow  = hist.GetBinContent(numbins+1) # this bin contains the overflow
	locontent = hist.GetBinContent(1) # this is the first bin plotted
	underflow = hist.GetBinContent(0) # this bin contains the underflow
	if (underflow>0 or overflow>0):
		#print " %-10s numbins=%4i hicontent=%4.2f over=%4.2f locontent=%4.2f underflow=%4.2f" % (name,numbins,hicontent,overflow,locontent,underflow)
		hist.SetBinContent(numbins,hicontent+overflow)
		#hist.SetBinContent(1,locontent+underflow) # Be CAREFUL some variables are initialized at -10 and will show up here. 
		# Restore the number of entries, it adds +1 if there is overflow !!!???
		# http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=6764
		hist.SetEntries(entries)	
	return hist

def PlotHisto(h,filename):
	# Takes a list of numbers and makes a histo	
	gInterpreter.ExecuteMacro("~aran/public/rootlogon.C")
	c1 = TCanvas ("c1")
	h.SetTitle('')
	h.Draw()
    	c1.SaveAs(filename)
	
	return

# Takes a list of variables (each is a list of numbers) and overlays them in TH1
def PlotOverlayHistos(hlist,xtitle,filename):
	palette=[4,2,1,3,5,6,7]
	if len(palette) < len(hlist):
		print 'Error in PlotOverlayHistos: specify more colors in palette!'
		return 
	
	gInterpreter.ExecuteMacro("~aran/public/rootlogon.C")
	c1 = TCanvas ("c1")
	leg=TLegend(0.7,0.50,0.90,0.70)
	leg.SetBorderSize(0); leg.SetTextSize(0.04); leg.SetTextFont(62); leg.SetFillColor(0)
	i=0
	for h in hlist:
		h.SetLineColor(palette[i])
		h.SetLineStyle(i+1)
		h.SetStats(0)
		leg.AddEntry(h,h.GetTitle(),'L')
		if i==0: 
			h.SetXTitle(xtitle)
			h.SetTitle('')
			h.Draw()
		else: h.Draw("SAME")
		i=i+1
		
	leg.Draw("SAME")	
    	c1.SaveAs(filename)
	return
	
#
# Plots Afb vs a user-defined variable in nbins 
# Afb is defined as Nt(dy>0)-Ntbar(dy<0) / Nt(dy>0)+Ntbar(dy<0)
#
# event is an instance of the EventClass
# varname is the variable we want in the x-axis: bb_dy, or bjet.Pt, or alljets.M for example
#
def PlotAfbVsVar(event,varname,xtitle,nbins,xmin,xmax,outfilename):	
	if '.' in varname: # For example 'bjet.Pt'
		objname,sep,method=varname.partition('.') # bjet,.,Pt
		var=event.Get(objname,method)
	else:	# normal list variable:
		var=event.getattr(varname)
	# Determine bin ranges given the input nbins,xmin,xmax:	
	# IDEALLY, WE COULD HAVE A FUNCTION THAT CALCULATES IRREGULAR BINS 
	# BASED ON HOW MANY EVENTS WE HAVE IN EACH, SO THAT EACH BIN HAS THE SAME ERRORS
	# It's complicated: http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=10855
	xbins=[]
	for n in xrange(nbins):
        	diff=int(float(xmax-xmin)/nbins)
        	xbins.append(xmin+n*diff)
	#print xbins
        xbins.append(xmax) #Now len(xbins)=nbins+1	

	#OptimizeBinSizesForEqualEntriesPerBin(var,nbins,xbins)

	# Now loop over entries and count deltay>0 and <0 as a function of bin position
	entries=event.N
	npos=nbins*[0]
	nneg=nbins*[0]
	for i in xrange(entries):
		by = event.bjet[i].Rapidity()
		bbary = event.Bjet[i].Rapidity()
		deltay=by-bbary
		for n in xrange(nbins):
			if (var[i]>xbins[n] and var[i]<xbins[n+1]):
				if deltay>0: npos[n]=npos[n]+1
				if deltay<0: nneg[n]=nneg[n]+1			
		# end loop over bins 
	# end loop over events
	print ' Npos=',npos,'Nneg=',nneg
	PlotAsymInBins(npos,nneg,nbins,xbins,xtitle,outfilename)
	
#
# Plots Ac vs a user-defined variable in nbins 
# Ac is defined as Nt(|y|<yC)-Ntbar(|y|<yC) / Nt(|y|<yC)+Ntbar(|y|<yC)
# See hep-ph/1007.4328
# 
# event is an instance of the EventClass
# varname is the variable we want in the x-axis: bb_dy, or bjet.Pt, or bb.M for example
#
def PlotAcVsVar(event,varname,xtitle,nbins,xmin,xmax,outfilename):
	YCUT=0.7	
	if '.' in varname: # For example 'bjet.Pt'
		objname,sep,method=varname.partition('.') # bjet,.,Pt
		var=event.Get(objname,method)
	else:	# normal list variable:
		var=event.getattr(varname)
	# Determine bin ranges given the input nbins,xmin,xmax:	
	# IDEALLY, WE COULD HAVE A FUNCTION THAT CALCULATES IRREGULAR BINS 
	# BASED ON HOW MANY EVENTS WE HAVE IN EACH, SO THAT EACH BIN HAS THE SAME ERRORS
	# It's complicated: http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=10855
	xbins=[]
	for n in xrange(nbins):
        	diff=int(float(xmax-xmin)/nbins)
        	xbins.append(xmin+n*diff)
	#print xbins
        xbins.append(xmax) #Now len(xbins)=nbins+1	

	# Now loop over entries and count Npos and Nneg as a function of bin position
	entries=event.N
	npos=nbins*[0]
	nneg=nbins*[0]
	for i in xrange(entries):
		by = event.bjet[i].Rapidity()
		bbary = event.Bjet[i].Rapidity()		
		for n in xrange(nbins):
			if (var[i]>xbins[n] and var[i]<xbins[n+1]):
				if abs(by)<YCUT: npos[n]=npos[n]+1
				if abs(bbary)<YCUT: nneg[n]=nneg[n]+1	
		# end loop over bins 
	# end loop over events
	print ' Npos=',npos,'Nneg=',nneg
	PlotAsymInBins(npos,nneg,nbins,xbins,xtitle,outfilename)
	

#
# Plot the Asymmetry N(+) - N(-) / N(+) + N(-) as a function of one variable
# Inputs: npos[nbins]=count of N+ events in each bin
#         nneg[nbins]=count of N- events in each bin
#         xbins[nbins+1]=bin edges
#
def PlotAsymInBins(npos,nneg,nbins,xbins,xtitle,outfilename):
	# Now make plot (need npos,neg,nbins,xbins)
	gInterpreter.ExecuteMacro("~aran/public/rootlogon.C")
	c1 = TCanvas ("c1")
	c1.SetGrid(1,1)
	gStyle.SetGridColor(17)
	
	g_Afb = array('f')
    	g_var = array('f')
    	g_Afberr = array('f')
	g_varerr = array('f')
	for n in xrange(nbins):
		Afb,Afberr=CalculateAsymmetryWithError(npos[n],nneg[n])
		g_Afb.append(Afb)
		g_Afberr.append(Afberr)
		bin_center=(xbins[n+1]-xbins[n])/2.0
		g_var.append(bin_center+xbins[n])
                g_varerr.append((xbins[n+1]-xbins[n])/2.0)
		print " Bin %2i (%3.1f,%3.1f): Afb = %4.3f +- %4.3f" % (n,xbins[n],xbins[n+1],Afb,Afberr)

	gr = TGraphErrors(nbins,g_var,g_Afb,g_varerr,g_Afberr);
    	gr.SetMarkerStyle(20)
    	gr.SetMarkerSize(1.0)
    	gr.SetMarkerColor(1)
    	gr.SetLineColor(1)
    	gr.SetLineWidth(2)
    	#gr.SetMaximum(1.0)
    	gr.GetXaxis().SetTitle(xtitle)
    	gr.GetYaxis().SetTitle("A_{fb}")
	gr.SetTitle("A_{fb}=#frac{N(#Deltay>0)-N(#Deltay<0)}{N(#Deltay>0)+N(#Deltay<0)}")
    	#gr.GetYaxis().SetNdivisions(504)
    	gr.Draw("AP")
    	c1.SaveAs(outfilename)
	
		