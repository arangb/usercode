#!/usr/bin/env python

import sys
import os
from CutStats import *
from AsymmetryUtils import *
from ReconstructEvents import *
from Plotters import *

def main():
	if len(sys.argv) < 2:
        	print "Usage: AsymmetryAnalysis.py <list_of_input_root_files>"
	
	input_filelist=sys.argv[1:]
	for infile in input_filelist:
		print " Calculating asymmetry for ",infile
		stat.Reset()
		tree=GetTreeFromFile(infile)
		#for event in tree:
		#	print event.njt
		cut_tree=ApplyCuts(tree,['njt>1','jtpt[0]>15'],['njt','jtpt'])		
		
		# The copying of trees into memory gives warnings, so delete this spurious file:
		if (os.path.exists('empty.root')): os.remove('empty.root') 
		RecoGenEvt=RecoGenBBEvents(cut_tree,'bbar_signal',['mcweight','njt','jtgenid','jtpt','jtphi','jteta','jty','jte','jtmuptrel','met'])
		RecoGenEvt.CutEvents('bb_dy','>',0.0)
		#RecoGenEvt.CutEvents('bb.M','>',10.0)
		#RecoGenEvt.SaveTree(cut_tree,'caca.root')
		
		bbM=RecoGenEvt.Get('bb','M')
		#bbdy=RecoGenEvt.bb_dy
		#print 'bjet pT: ', bpt		
		#print 'bb_dy: ', bbdy
		
		#PlotHisto(MakeHisto(bbM,RecoGenEvt.mcweight,'hbb_M','bb_M',' M_{b#bar{b}} [GeV]',20,0.,400.),'bb_M.eps')
		PlotHisto(MakeHisto(RecoGenEvt.Get('alljets','M'),RecoGenEvt.mcweight,'hallj_M','allj_M',' M_{alljets} [GeV]',40,0.,800.),'alljets_M.png')
		#h1=MakeHisto(RecoGenEvt.Get('bjet','Pt'),RecoGenEvt.mcweight,'h_bpt','b jet','b jet candidate p_{T} [GeV]',20,0.,50.)
		#h2=MakeHisto(RecoGenEvt.Get('Bjet','Pt'),RecoGenEvt.mcweight,'h_Bpt','#bar{b} jet','B jet candidate p_{T} [GeV]',20,0.,50.)
		#PlotOverlayHistos([h1,h2],'Jet p_{T} [GeV]','b_B_pt.eps')
		#PlotHisto(MakeHisto(RecoGenEvt.bb_dy,RecoGenEvt.mcweight,'hbb_dy','bb_dy',' #Deltay=y_{b}-y_{#bar{b}}',20,-5,5),'bb_dy.eps')
		#PlotHisto(MakeHisto(RecoGenEvt.bjtmuptrel,RecoGenEvt.mcweight,'h_bmuptrel','b jet','b jet muon p_{T}^{rel} [GeV]',20,0.,5.),'bmuptrel.eps')
	
		#PlotAsymVsVar(RecoGenEvt,'bb.M','M_{b#bar{b}} [GeV]',4,20.,100.,'Afb_vs_bbM.png')
		
		stat.PrintSummary()
main()