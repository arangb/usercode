import FWCore.ParameterSet.Config as cms

source = cms.Source("LHESource",
                            fileNames = cms.untracked.vstring('file:/afs/cern.ch/user/a/aran/scratch0/CMSSW_3_8_4/src/bbar_10k_LHAPDF.lhe'), 
                            #skipEvents=cms.untracked.uint32(2000)
)

generator = cms.EDFilter("Herwig6HadronizerFilter",
	comEnergy = cms.double(7000.0),
	#crossSection = cms.untracked.double(0.523),
	doMPInteraction = cms.bool(True),
	emulatePythiaStatusCodes = cms.untracked.bool(True),
	filterEfficiency = cms.untracked.double(1.0),
	herwigHepMCVerbosity = cms.untracked.bool(False),
	herwigVerbosity = cms.untracked.int32(1),
	lhapdfSetPath = cms.untracked.string(''),
	maxEventsToPrint = cms.untracked.int32(0),
	printCards = cms.untracked.bool(True),
	useJimmy = cms.bool(True),

	HerwigParameters = cms.PSet(
		herwigUEsettings = cms.vstring(
			'JMUEO     = 1       ! multiparton interaction model',
			'PTJIM     = 4.449   ! 2.8x(sqrt(s)/1.8TeV)^0.27 @ 10 TeV',
			'JMRAD(73) = 1.8     ! inverse proton radius squared',
			'PRSOF     = 0.0     ! prob. of a soft underlying event',
			'MAXER     = 1000000 ! max error'
		),
                herwigMcatnlo = cms.vstring(
#			'IPROC      = 1705   ! QCD 2->2 processes',
		   	'PTMIN      = 10.    ! minimum pt in hadronic jet',
                        'YJMAX      = +4.    ! max jet rapidity',
                        'YJMIN      = -4.    ! min jet rapidity'
		),
		parameterSets = cms.vstring('herwigUEsettings',
                                            'herwigMcatnlo')
	)
)

mufilter = cms.EDFilter("MCSingleParticleFilter",
    # MC particle status code: 1=stable, 2=shower, 3=hard scattering; 0=all are accepted
    Status = cms.untracked.vint32(1,1),
    MinPt = cms.untracked.vdouble(3.,3.),
    # vector of accepted particle ID (logical OR)
    ParticleID = cms.untracked.vint32(13,-13) 
)
		
bfilter = cms.EDFilter("MCSingleParticleFilter",
    # MC particle status code: 1=stable, 2=shower, 3=hard scattering;
    Status = cms.untracked.vint32(3, 3),
    MinPt = cms.untracked.vdouble(5., 5.),
    # vector of accepted particle ID (logical OR)
    ParticleID = cms.untracked.vint32(5, -5) 
)	  

bbfilter = cms.EDFilter("MCParticlePairFilter",
      # vector of accepted particle ID for particle 1 (logical OR)(abslolute values of the ID's)
      ParticleID1 = cms.untracked.vint32(5),
      # vector of accepted particle ID for particle 2 (logical OR)(abslolute values of the ID's)
      ParticleID2 = cms.untracked.vint32(5),
      # accepted particle pair charge: -1 = Opposite Sign, +1 = Same Sign, 0 = both, default: 0)
      ParticleCharge = cms.untracked.int32(-1),
      # vector of min pt values corresponding to above particles -- if absent values are 0
      MinPt = cms.untracked.vdouble(10.),
      # vector of min pt values corresponding to above particles -- if absent values are -5
      MinEta = cms.untracked.vdouble(-3.0),
      # vector of min pt values corresponding to above particles -- if absent values are +5
      MaxEta = cms.untracked.vdouble(3.0),
      # vector of status codes corresponding to above particles -- if absent, all are accepted
      Status = cms.untracked.vint32(3, 3),
      # minimum invariant mass of the pair
      #MinInvMass = cms.untracked.double(40),
      # maximum invariant mass of the pair
      #MaxInvMass = cms.untracked.double(1000),
      # minimum delta phi (angle in transverse plain) between the pair (in radians)
      #MinDeltaPhi = cms.untracked.double(0.),
      # maximum delta phi (angle in transverse plain) between the pair (in radians)
      #MaxDeltaPhi = cms.untracked.double(6.29)
)
		
ProductionFilterSequence = cms.Sequence(generator*mufilter*bbfilter)

# Add this in the output of cmsDriver to print a long summary at the end.  
# process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )
   