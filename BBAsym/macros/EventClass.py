#-------------------------------------------------------------------------------------------------
# Filename: EventClass.py
# Description: This should be a very generic bag where to put events once they have been reconstructed.
#              So at this point we should know everything about it: which is the b and which is the bbar. 
#              Then we just add attributes (one for each variable we want to have, for example bb_dR and B_pT). 
#              The size of these attributes should be the number of events we select.
#              The attributes can also be TLorentzVectors, so that we don't have to store many of them (see MakeParticle)
#              So if you want to obtain a list of the bb_dR for each event: bbdR=self.bb_dR, 
#              or for the bjet.Pt: bpt=self.Get('bjet','Pt'), where Pt is any method accessible for a TLorentzVector
#              It allows to add attributes on the fly (see fillvariable).
#              It allows to make cuts on both direct attributes (like bb_dR) or on TLorentz objects: (like bjet.Pt)
#              It keeps track of the number of events through a TEntryList, such that we can go back to the original tree
#              save it with only the events we want. 
# Created: 5-Oct-2010 Aran Garcia-Bellido (aran@mail.cern.ch)
#-------------------------------------------------------------------------------------------------
from KinematicUtils import MasslessPartonsMt
from ROOT import TFile, TTree, TEntryList, gDirectory
from CutStats import *
#
class BBbarEvent:
	# Defined with two lists of TLorentzVectors
	def __init__(self,bjet,Bjet,entrylist,sourcename):
		if (len(bjet) != len(Bjet)):
			print ' BBbarEvent ',sourcename,' ERROR: the two input arrays don\'t have the same lenght!'
			return
		
		self.N=len(bjet)
		self.bjet=bjet
		self.Bjet=Bjet
		self.elist=entrylist
		self.name=sourcename
		# Create the bb pair candidate, it's the sum of two LorentzVectors:
		bbcand=[bjet[i]+Bjet[i] for i in xrange(self.N)]
		self.bb=bbcand
		#
		bb_dR,bb_dy,bb_Mtptphi=[],[],[]
		for i in xrange(self.N):
			bb_dR.append(bjet[i].DeltaR(Bjet[i]))
			bb_dy.append(bjet[i].Rapidity()-Bjet[i].Rapidity())
			bb_Mtptphi.append(MasslessPartonsMt(bjet[i].Pt(),bjet[i].Phi(),Bjet[i].Pt(),Bjet[i].Phi()))
		
		self.bb_dR=bb_dR
		self.bb_dy=bb_dy
		self.bb_Mt=bb_Mtptphi
		
		
		print ' BBbarEvent instantiated for ', sourcename
	
	# Read http://www.tutorialspoint.com/python/python_classes_objects.htm
	# This class is like defining a member function in the __init__ section:
	# self.bjtpt=[15.,13.,45.]
	# But instead you can do it on the fly with this method:
	# x=ReconstructedEvent
	# bjtpt=[15.,13.,45.]
	# x.fillvariable('bjtpt',bjtpt)
	# print x.bjtpt[2]  --> this will print 45.
	# Awesome! 
    	def fillvariable(self,name,values):
		setattr(self,name,values)
			
	# Example: bpt=MyBBEvent.Get('bjet','Pt')
	# will return an array with all the bjet's Pts
	# Much better than having to loop: i_pt=MyBBEvent.bjet[i].Pt()
	def Get(self,objname,methodname):
		# objname: bjet, Bjet, or bbcand 
		var=[]
		particles=getattr(self,objname)
		for particle in particles:
			# this is analog to: value=particle.methodname()
			# but accessing the object and method by name!
			value=getattr(particle,methodname)()
			var.append(value)
		return var
		
	def SaveTree(self,intree,outfilename):
		intree.SetBranchStatus('*',1)
		intree.SetEntryList(self.elist)
		f=TFile(outfilename,'recreate')
		newtree=intree.CopyTree('')
		newtree.Write()
		f.Close()
		print ' BBbarEvent: Saved tree (',self.N,' events) in file ',outfilename
	
	def CutEvents(self,varname,cuttype,cutvalue):		
		#elist,newelist=[],[] # These are the same as TEntryList but in python arrays
		newentrylist=TEntryList() # The new TEntryList with selected events
		indeces_to_keep=[] # list of indeces (not entries) to keep
		nevents=self.N
		# Special case if we want to look at the bjet.Pt() or the bb.M(), 
		# which are methods of TLorentzVector, instead of values of methods, like bb_dy or njt:
		if '.' in varname: # For example 'bjet.Pt'
			objname,sep,method=varname.partition('.') # bjet,.,Pt
			var=self.Get(objname,method)
		else:	# normal list variable:
			var=getattr(self,varname)
		# ok, now we have the list with values for the variable we want to cut on:	
		for i in xrange(self.N):
			entrynum=self.elist.GetEntry(i)
			#elist.append(int(entrynum)) # fill list with items from entrylist
			#print 'evtlist:',i,entrynum, var[i]
			if   (cuttype == '>'  and var[i] <= cutvalue): continue
			elif (cuttype == '>=' and var[i] <  cutvalue): continue
			elif (cuttype == '<'  and var[i] >= cutvalue): continue
			elif (cuttype == '<=' and var[i] >  cutvalue): continue
			elif (cuttype == '==' and var[i] != cutvalue): continue
			elif (cuttype == '!=' and var[i] == cutvalue): continue
			#else: print 'ERROR IN CutEvents!'
			#print ' goodevent ', i, entrynum, var[i] 
			newentrylist.Enter(entrynum)
			#newelist.append(int(entrynum))
			indeces_to_keep.append(i)
			
		#print 'old list:',elist
		#print 'new list:',newelist	
		#print 'indeces :',indeces_to_keep
		# Now loop over all attributes of this class:
		for attr, values in self.__dict__.iteritems():
			#print attr,values
			if (attr=='N' or attr=='Ninitial' or attr=='name' or attr=='elist'): continue
			newvalues=[values[i] for i in xrange(len(values)) if (i in indeces_to_keep)]
			#print ' %10s ' % attr,newvalues
			#if len(indeces_to_keep)==0: newvalues=[]
			setattr(self,attr,newvalues)
		
		self.elist=newentrylist
		self.N=newentrylist.GetN()
		cut_nevents=self.N
		
		stat.AddCut(varname+cuttype+str(cutvalue),nevents,cut_nevents)
		#print " %-16s | %6s | %6s |  %6.2f%%  | %8s  |"  % (varname+cuttype+str(cutvalue),nevents,cut_nevents,100.*float(cut_nevents)/float(nevents),'---- ')
		#print " %-16s | %6s | %6s |  %6.2f%%  | %6.2f%% |"  % (varname+cuttype+str(cutvalue),nevents,cut_nevents,100.*float(cut_nevents)/float(nevents),100.*float(cut_nevents)/float(self.Ninitial))
		
		
		
		return
#