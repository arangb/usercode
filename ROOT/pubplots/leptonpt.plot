#
# Config file for compare_histos.C macro
#

Output.File: leptonpt.eps

#
# Type can be Legend, StatBox, OverlayLines
# Add: to do the stacking
Histo.Type: PublicationHisto
Histo.Name: h_lepton_pt

#Histo.Xmin:   0.000000
#Histo.Xmax:   10.000000
#Histo.Ymin:   0.000000
#Histo.Ymax:   70.000000
Histo.XNdivisions:  510
Histo.YNdivisions:  504
Histo.TextSize: 0.06
#Histo.XTitle: Electron p_{T} [GeV]
Histo.YTitle: Yield [Events]
Histo.AddText.1: a)
Histo.AddText.1.X0: 0.20
Histo.AddText.1.Y0: 0.85
#Histo.AddText.2: pre-tag
#Histo.AddText.3: 2 jets
#Histo.AddText.3.X0: 0.20
#Histo.AddText.3.Y0: 0.90
# ShowLegend can be no/right/left
Histo.ShowLegend: right

Histo.ShowSignalSeparately: 1
Histo.Signal.Title.1: m_{H}=100
#Histo.Signal.Title.2: m_{H}=90

#
# Files for input. 
# - Numbering MUST start at 1.
# - DATA should ALWAYS be first. 
# - Then the backgrounds, and finally all the signal points we want to plot
# 
Files.Number: 8

Files.1.Name: /prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/data_zero_plots.root
Files.1.Title: Data
Files.1.Factor: 1.000
Files.1.Color: 1

Files.2.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/qcd_zero_plots.root
Files.2.Title: Multijets
Files.2.Color: 9
# Files.2.FillStyle: 2045 
# can also do filled areas

Files.3.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/ztt_zero_plots.root
Files.3.Title: Z #rightarrow #tau#tau
Files.3.Color: 2

Files.4.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/zee_zero_plots.root
Files.4.Title: Z #rightarrow ee
Files.4.Color:  4

Files.5.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/wjets_zero_plots.root
Files.5.Title: W+jets
Files.5.Color: 5

Files.6.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/diboson_zero_plots.root
Files.6.Title: Diboson
Files.6.Color: 25

Files.7.Name:/prj_root/3014/top_write/aran/lhoods/36invfb_newnlo/ttbar_zero_plots.root
Files.7.Title: t#bar{t}
Files.7.Color:  3

#Files.8.Name:/prj_root/3014/top_write/aran/signal_nlo/37ifb/mH_100_btag_nominal_plots.root
Files.8.Name: /prj_root/3014/top_write/aran/signal_nlo/37ifb/mH_100_btag_nominal_plots.root
Files.8.Title: m_{H}=100
Files.8.Factor: 5.000
Files.8.Color:  6
Files.8.LineStyle: 1

