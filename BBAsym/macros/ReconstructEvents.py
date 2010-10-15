#!/usr/bin/env python
#-------------------------------------------------------------------------------------------------
# Filename: ReconstructEvents.py
# Description: This is where we chose how to reconstruct each event of a tree, wether we match to 
#              the generator level, or some reconstruction variable.
#              We can also ApplyCuts over the tree before we reconstruct the event (we will also 
#              be able to make cuts on new variables in the reconstructed event).
# Created: 5-Oct-2010 Aran Garcia-Bellido (aran@mail.cern.ch)
#-------------------------------------------------------------------------------------------------
from AsymmetryUtils import *
from KinematicUtils import *
from EventClass import *
from CutStats import *
#
# From an input TTree utm/t identify the two b and bbar jets in the event and store some quantities
#


# Pass a list of cuts cuts=['njt>0','jtpt[0]>25'] 
# and the leaves you need to use for them: leaves_needed=['njt','jtpt']
# This will apply those cuts on the tree and return the tree without those events
def ApplyCuts(tree,cuts,leaves_needed):
	# Make sure the cuts use leaves that we have available: 
	for leaf in leaves_needed: tree.SetBranchStatus(leaf,1)
	
	nevents=tree.GetEntries()
	cumulative_cut='' # will contain the logical AND (&&) of all cuts 
	cut_nevents=nevents # inital counter = total starting events
	ncut=0; ncuts=len(cuts) # number of cuts
	for cut in cuts:
		input_events=cut_nevents # counter after previous cut
		if ncut==0: cumulative_cut=cut
		else: cumulative_cut=cumulative_cut + '&&' + cut
		# Create TEntryList called elist with the cuts we want:
		tree.Draw(">>elist", cumulative_cut, "entrylist");
		elist = gDirectory.Get("elist"); # declare the elist just defined in memory
		cut_nevents=elist.GetN() # counter after this cut
		stat.AddCut(cut,input_events,cut_nevents)
		# ok, last cut, let's finally tell the tree which events it should have:
		#if (ncut==ncuts-1): tree.SetEntryList(elist)
		elist.Delete()
		ncut=ncut+1
	##############
	## Warning! 
	## tree.GetEntries() returns the original number, instead use: tree.GetSelectedRows().
	##############
	# But it is better to make a copy of the old tree with the total selection:
	tree.SetBranchStatus('*',1) # First activate all branches
	# 
	output = TFile.Open("empty.root","RECREATE") # we'll delete it later
	newtree=tree.CopyTree(cumulative_cut) # make the copy
	#newtree.SetDirectory(0)
	newtree.SetBranchStatus('*',0) # and turn them off again for passing
	#tree.Delete()
	return newtree

#########################################################################################

def RecoMuChargeBBEvents(tree,sourcename,leaflist):
	# Only read those leaves that we need:
	Leafarray={} # Dictionary to contain each leaf
	for leaf in leaflist:
		tree.SetBranchStatus(leaf,1)
		# Create holders for each leaf:
		Leafarray[leaf]=tree.FindLeaf(leaf)
	
	entries=tree.GetEntries()
	countbb=0
	#
	bjet=[] # list of TLorentzVectors (one for each event)
	Bjet=[] # list of TLorentzVectors (one for each event)
	bb_Mt,bb_dR,bb_dy=[],[],[]
	bjtmuptrel,Bjtmuptrel,bjtmuchg,Bjtmuchg=[],[],[],[]
	njt=[]
	met=[]
	alljets=[] # list of TLorentzVectors of the sum of alljets
	mcweight=[] 
	# Make an empty entrylist
	#tree.Draw(">>elist", "", "entrylist")
   	#elist=gDirectory.Get("elist")
	elist=TEntryList('elist','elist')
	for ientry in xrange(entries):
		tree.GetEntry(ientry)
		# Ensure we have at least two jets in the event
		if (Leafarray['njt'].GetValue()>1 ):
			b_index=get_index_for_pdgid(1,Leafarray['jtmuchg'])
			bbar_index=get_index_for_pdgid(-1,Leafarray['jtmuchg'])
			# Only accept events with b and bbar:
			if (b_index<0 and bbar_index<0): continue
			#if (b_index>0 
			# ok, now we have a b and bbar:
			bE=Leafarray['jte'].GetValue(b_index)
			bpt=Leafarray['jtpt'].GetValue(b_index)
			bphi=Leafarray['jtphi'].GetValue(b_index)
			beta=Leafarray['jteta'].GetValue(b_index)
			thisbjet=MakeParticle(bE,bpt,bphi,beta)
			bjet.append(thisbjet)
			#
			BE=Leafarray['jte'].GetValue(bbar_index)
			Bpt=Leafarray['jtpt'].GetValue(bbar_index)
			Bphi=Leafarray['jtphi'].GetValue(bbar_index)
			Beta=Leafarray['jteta'].GetValue(bbar_index)
			thisBjet=MakeParticle(BE,Bpt,Bphi,Beta)
			Bjet.append(thisBjet)
			#
			# Fill in muon vars:
			bjtmuptrel.append(Leafarray['jtmuptrel'].GetValue(b_index))
			Bjtmuptrel.append(Leafarray['jtmuptrel'].GetValue(bbar_index))
			#bjtmuchg.append(Leafarray['jtmuchg'].GetValue(b_index))
			#Bjtmuchg.append(Leafarray['jtmuchg'].GetValue(bbar_index))
			# Other event variables:
			mcweight.append(Leafarray['mcweight'].GetValue())
			njt.append(Leafarray['njt'].GetValue())
			met.append(Leafarray['met'].GetValue())
			# alljets object (sum of 4-vectors of all jets):
			allj=TLorentzVector(0.,0.,0.,0.)
			for j in xrange(int(Leafarray['njt'].GetValue())):
				jE=Leafarray['jte'].GetValue(j)
				jpt=Leafarray['jtpt'].GetValue(j)
				jphi=Leafarray['jtphi'].GetValue(j)
				jeta=Leafarray['jteta'].GetValue(j)
				allj=allj+MakeParticle(jE,jpt,jphi,jeta)
			alljets.append(allj)
			#
			elist.Enter(ientry) # we want this event from this tree.
			#
			countbb=countbb+1
		# end of two jets
	#end of loop over events
	stat.AddCut('MakeEvt: bbmuchg',entries,countbb)
	# Check elist:
	#elist=gDirectory.Get("elist")
	if (elist.GetN() != countbb): print 'ERROR in RecoGenBBEvents: elist has incorrect length!',elist.GetN(), countbb
	# Instantiate our event:
	RecoMuChargeBB=BBbarEvent(bjet,Bjet,elist,sourcename)
	RecoMuCharge.fillvariable('bjtmuptrel',bjtmuptrel)
	RecoMuCharge.fillvariable('Bjtmuptrel',Bjtmuptrel)
	RecoMuCharge.fillvariable('bjtmuchg',bjtmuchg)
	RecoMuCharge.fillvariable('Bjtmuchg',Bjtmuchg)
	RecoMuCharge.fillvariable('mcweight',mcweight)
	RecoMuCharge.fillvariable('njt',njt)
	RecoMuCharge.fillvariable('met',met)
	RecoMuCharge.fillvariable('alljets',alljets)
	#
	return RecoMuChargeBB
	
	
	
#########################################################################################

## EXPLORE pt=getattr(mytree,"BranchName")
##         for i in xrange(pt):
##		print pt[i]

def RecoGenBBEvents(tree,sourcename,leaflist):	
	# Only read those leaves that we need:
	Leafarray={} # Dictionary to contain each leaf
	for leaf in leaflist:
		tree.SetBranchStatus(leaf,1)
		# Create holders for each leaf:
		Leafarray[leaf]=tree.FindLeaf(leaf)
	# So now, when we want to access the value of a given leaf, we can call:
	# Leafarray['jtpt'].Getvalue(2)  
	# instead of defining one by one: jtptLeaf=tree.FindLeaf('jtpt') ; jtptLeaf.GetValue(2)
		
	entries=tree.GetEntries()
	countbb=0
	#
	bjet=[] # list of TLorentzVectors (one for each event)
	Bjet=[] # list of TLorentzVectors (one for each event)
	bb_Mt,bb_dR,bb_dy=[],[],[]
	bjtmuptrel,Bjtmuptrel,bjtmuchg,Bjtmuchg=[],[],[],[]
	njt=[]
	met=[]
	alljets=[] # list of TLorentzVectors of the sum of alljets
	mcweight=[] 
	# Make an empty entrylist
	#tree.Draw(">>elist", "", "entrylist")
   	#elist=gDirectory.Get("elist")
	elist=TEntryList('elist','elist')
	for ientry in xrange(entries):
		tree.GetEntry(ientry)
		# Ensure we have at least two jets in the event
		if (Leafarray['jtgenid'].GetLen()>1 ):
			b_index=get_index_for_pdgid(5,Leafarray['jtgenid'])
			bbar_index=get_index_for_pdgid(-5,Leafarray['jtgenid'])
			# Only accept events with b and bbar:
			if (b_index<0 or bbar_index<0): continue
			# ok, now we have a b and bbar:
			bE=Leafarray['jte'].GetValue(b_index)
			bpt=Leafarray['jtpt'].GetValue(b_index)
			bphi=Leafarray['jtphi'].GetValue(b_index)
			beta=Leafarray['jteta'].GetValue(b_index)
			thisbjet=MakeParticle(bE,bpt,bphi,beta)
			bjet.append(thisbjet)
			#
			BE=Leafarray['jte'].GetValue(bbar_index)
			Bpt=Leafarray['jtpt'].GetValue(bbar_index)
			Bphi=Leafarray['jtphi'].GetValue(bbar_index)
			Beta=Leafarray['jteta'].GetValue(bbar_index)
			thisBjet=MakeParticle(BE,Bpt,Bphi,Beta)
			Bjet.append(thisBjet)
			#
			# Fill in muon vars:
			bjtmuptrel.append(Leafarray['jtmuptrel'].GetValue(b_index))
			Bjtmuptrel.append(Leafarray['jtmuptrel'].GetValue(bbar_index))
			#bjtmuchg.append(Leafarray['jtmuchg'].GetValue(b_index))
			#Bjtmuchg.append(Leafarray['jtmuchg'].GetValue(bbar_index))
			# Other event variables:
			mcweight.append(Leafarray['mcweight'].GetValue())
			njt.append(Leafarray['njt'].GetValue())
			met.append(Leafarray['met'].GetValue())
			# alljets object (sum of 4-vectors of all jets):
			allj=TLorentzVector(0.,0.,0.,0.)
			for j in xrange(int(Leafarray['njt'].GetValue())):
				jE=Leafarray['jte'].GetValue(j)
				jpt=Leafarray['jtpt'].GetValue(j)
				jphi=Leafarray['jtphi'].GetValue(j)
				jeta=Leafarray['jteta'].GetValue(j)
				allj=allj+MakeParticle(jE,jpt,jphi,jeta)
			alljets.append(allj)
			#
			elist.Enter(ientry) # we want this event from this tree.
			#
			countbb=countbb+1
		# end of two jets
	#end of loop over events
	stat.AddCut('RecoEvents bbgen',entries,countbb)
	# Check elist:
	#elist=gDirectory.Get("elist")
	if (elist.GetN() != countbb): print 'ERROR in RecoGenBBEvents: elist has incorrect length!',elist.GetN(), countbb
	# Instantiate our event:
	RecoGenBB=BBbarEvent(bjet,Bjet,elist,sourcename)
	RecoGenBB.fillvariable('bjtmuptrel',bjtmuptrel)
	RecoGenBB.fillvariable('Bjtmuptrel',Bjtmuptrel)
	#RecoGenBB.fillvariable('bjtmuchg',bjtmuchg)
	#RecoGenBB.fillvariable('Bjtmuchg',Bjtmuchg)
	RecoGenBB.fillvariable('mcweight',mcweight)
	RecoGenBB.fillvariable('njt',njt)
	RecoGenBB.fillvariable('met',met)
	RecoGenBB.fillvariable('alljets',alljets)
	#
	return RecoGenBB	
#