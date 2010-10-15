
class CutStats:
	def __init__(self):
		self.cutNames=[]
		self.cutEventsIn=[]
		self.cutEventsOut=[]
		#self.Ninitial=self.cutEventsIn[0]
		self.Ncurrent=0
		self.Ncuts=0
		print ' Instantiated CutStats'
		
	def AddCut(self,cutname,Nin,Nout): # The cut name and the number of events after the cut	
		self.cutNames.append(cutname)
		self.cutEventsOut.append(Nout)
		self.cutEventsIn.append(Nin)
		self.Ncurrent=Nout
		self.Ncuts=self.Ncuts+1
		return
	
	def AddWeight(self,weight):
		return

	def PrintSummary(self):
		if self.Ncuts==0: return
		print " %-16s | %6s | %6s | %8s  | %8s  |" % ('Cut applied','IN','OUT','rel.eff','tot.eff')
		for i in xrange(self.Ncuts):
			print " %-16s | %6i | %6i |  %6.2f%%  |  %6.2f%%  |" %        \
			(self.cutNames[i],self.cutEventsIn[i],self.cutEventsOut[i],   \
			100.*float(self.cutEventsOut[i])/float(self.cutEventsIn[i]),  \
			100.*float(self.cutEventsOut[i])/float(self.cutEventsIn[0]))  
		
	def Reset(self):
		self.cutNames=[]
		self.cutEventsIn=[]
		self.cutEventsOut=[]
		#self.Ninitial=self.cutEventsIn[0]
		self.Ncurrent=0
		self.Ncuts=0
		print ' Reset CutStats' 
		return

# This is how we make this stat variable a global between different modules.
# We instantiate it here and then just import CutStats whenever we want to use it! 
stat=CutStats()	