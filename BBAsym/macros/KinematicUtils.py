# From http://www-d0.fnal.gov/D0Code/source/kinem_util/kinem_util/AnglesUtil.hpp
# And: http://www-d0.fnal.gov/D0Code/source/top_cafe/src/TopTopologicalVariables.cpp
import math
from ROOT import TLorentzVector

# Ternary in C++: a > b ? x : y;
# Ternary in Python: x if a>b else y
def delta_phi(phi1,phi2):
	PHI=abs(phi1-phi2);
	# In C++: (PHI<=math.pi)? PHI : 2.*math.pi-PHI;
	return PHI if (PHI<=math.pi) else (2.*math.pi-PHI)

def delta_R(eta1,phi1,eta2,phi2):
	deta = eta1-eta2
	dphi = delta_phi(phi1,phi2)
	return math.sqrt(deta*deta + dphi*dphi)

def MasslessPartonsMt(pt1,phi1,pt2,phi2):
	mt=-1
	dphi=delta_phi(phi1,phi2)
	return math.sqrt(2.0*pt1*pt2*(1.0-math.cos(dphi)))

# objects is a list of TLorentzVectors
def Mt(objects):
    if(len(objects)<2): return -1.0
    # initialize
    Mt,sum_pT,sum_px,sum_py = 0.0,0.0,0.0,0.0
    # loop over objects
    for p in objects:
    	sum_pT += p.Pt()
    	sum_px += p.Px()
    	sum_py += p.Py()

    Mt = sum_pT*sum_pT - sum_px*sum_px - sum_py*sum_py

    if ( Mt >= 0.0 ):
      Mt = math.sqrt(Mt)
    else:
    	if (abs(Mt)<1.0e-6):
        	Mt = 0.0  # Square of Mt is negative, but small
    	else:
        	return -1.0
    	# square of Mt is negative and large
    #square of Mt is negative

    return Mt

def MakeParticle(E,pt,phi,eta):
	px=pt*math.cos(phi)
	py=pt*math.sin(phi)
	pz=pt*math.sinh(eta)
	particle=TLorentzVector(px,py,pz,E)
	return particle
