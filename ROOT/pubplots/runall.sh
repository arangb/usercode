#!/bin/bash
cd $SRT_PRIVATE_CONTEXT/hb_tautaub/macros/pubplots
for file in `ls *plot`; do
  #sed -i 's?Files.2.Title: QCD?Files.2.Title: Multijets?' $file
  echo 'root -l -q -b -x PublicationHisto.C\(\"'${file}'\"\)'
  root -l -q -b -x PublicationHisto.C\(\"${file}\"\)
done
