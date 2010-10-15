#!/usr/bin/env python

##
## Collection of utilities
##
from ROOT import TFile, TTree, TEntryList, gDirectory
import math

def GetTreeFromFile(inputfilename):
	# In case you want to access a file in castor, we need RFIO:
	if inputfilename.startswith('/castor/'):
        	inputfilename = 'rfio:' + inputfilename
		
	f = TFile.Open(inputfilename,'read')
	mytree=gDirectory.Get('utm/t')
	if not mytree:
		print "Error in GetTreeFromFile: No tree named utm/t in this file ",inputfilename
		exit(0)
	mytree.SetBranchStatus("*", 0) # Speed up reading! 
	return mytree	
#
# 
#
def RemakeTree(intree,outfilename):
	intree.SetBranchStatus('*',1)
	f=TFile(outfilename,'recreate')
	newtree=intree.CopyTree('')
	newtree.Write()
	f.Close()
	print ' RemakeTree: Saved tree (',newtree.GetEntries(),' events) in file ',outfilename
	return
#
#
#	
def CalculateAsymmetryWithError(Npos,Nneg):
	# F=x-y/(x+y) 
	# dF**2 = (dF/dx*dx)**2 + (dF/dy*dy)**2
	Asym=CalculateAsymmetry(Npos,Nneg)
	# Simple Poisson error:
	dNpos=math.sqrt(Npos)
	dNneg=math.sqrt(Nneg)
	dAdpos=2.0*float(Nneg)/(float(Npos+Nneg)**2)
	dAdneg=-2.0*float(Npos)/(float(Npos+Nneg)**2)
	AsymErr=math.sqrt(  (dAdpos*dNpos)**2 + (dAdneg*dNpos)**2  )
	return Asym,AsymErr 
	
def CalculateAsymmetry(Npos,Nneg):
	return float(Npos-Nneg)/float(Npos+Nneg)


#
# Input the jtgenid leaf for this event and the PDGID you want to match.
# If particle is not found, return -1
# This is like the general: list.index(i) which returns the first index where an item of the list matches i. 
def get_index_for_pdgid(pdgid,jtgenidLeaf):
	first_index=-1
	for i in xrange(jtgenidLeaf.GetLen()):
		if jtgenidLeaf.GetValue(i) == pdgid:
			first_index=i
			break
	return first_index
	
# From http://personal.denison.edu/~havill/algorithmics/python/search.py
# To find the index of an item in a list 
# BinarySearch([5.,10.,15.,20.],15.)=2 
# BinarySearch([5.,10.,15.,20.],13.)=-1 
def BinarySearch(L, item):
	first = 0
	last = len(L) - 1
	found = False
	while (first <= last) and not found:
		mid = (first + last) / 2;
		if item < L[mid]:
			last = mid - 1
		elif item > L[mid]:
			first = mid + 1
		else:
			found = True
			
	if found:
		return mid
	else:
		return -1


