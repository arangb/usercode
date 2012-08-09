#!/usr/bin/env bash
usage() {
    echo "Usage: ./`basename $0` <arguments> [options]"
    echo " <arguments>:"
    echo "  -file=<first file>[,first_title[,error]]    First file (title,error optional)"
    echo "  -file=<second file>[,second_title[,error]]  Second file (title,error optional)"
    echo "  -file=..."
    echo " [options]:"
    echo "  -outdir=<dirname>               Output directory"
    echo "  -force                          Overwrite existing output"
    echo "  -errors                         Show all histograms with error bars"
    echo "  -nonorm                         Do not normalize histograms"
    echo "  -nohtml                         Do not produce an html comparison page"
    echo "  -overlay=<overlay option>       Choose overlay type (StatBox or Legend)"
    echo "  -config=<config file>           Read config from file instead of building it"
    echo "  -goodnbins=<nbins>              Optimal number of bins, applying only to"
    echo "                                   histograms with at least nbins bins"
    echo "  -help,-h                        This help"
    echo
    echo " The script has to be run in the same directory as compare_histos.C is."
    echo " It will plot all the histograms it finds, and, if specified, can also plot the branches in a tree."
    echo " To get error bars, one can specify"
    echo "1 or true as the error parameter in the -file option for a specific file, or"
    echo "use the -errors option to apply it to all histograms. Keep in mind that errors"
    echo "will only be meaningful if your histograms were created with Sumw2()."
    echo " In title specifications, all underscores _ are replaced by a whitespace."
    echo " The configuration used is written in compare_files.from_script"
    echo
    exit 1
}

if [ ! -f compare_histos.C ]; then
    echo
    echo "*** Missing compare_histos.C file"
    echo
    usage
fi

FORCE=0
NORM=0
NFILES=0
OVERLAY=Overlay
HTML=1
ERRORS=0
GOODNBINS=-1

for var in $*; do
    if [ `echo "$var"| grep -c "\-file="` != 0 ]; then
        FILE=`echo "$var"| sed 's/\-file=//'`
	NFILES=$((NFILES+1))
	FILES[$NFILES]=`echo $FILE | cut -d, -f1`
	TITLES[$NFILES]=file$NFILES
	PLOTERRORS[$NFILES]=0
        if [ `echo $FILE |grep -c ,` -eq 1 ]; then
	    TITLE=`echo $FILE | cut -d, -f2`
	    TITLES[$NFILES]=${TITLE//_/ }   # replace _ with space
	    if [ `echo $FILE | tr -cd , |wc -c` -eq 2 ]; then
		PLOTERROR=`echo $FILE | cut -d, -f3 | tr '[A-Z]' '[a-z]'`
		[ x$PLOTERROR = "x1" -o x$PLOTERROR = "xtrue" ] && PLOTERRORS[$NFILES]=1
	    fi
	fi
    elif [ `echo "$var"| grep -c "\-outdir="` != 0 ]; then
        OUTPUT=`echo "$var"| sed 's/\-outdir=//'`
    elif [ `echo "$var"| grep -c "\-overlay="` != 0 ]; then
        OVERLAY=`echo "$var"| sed 's/\-overlay=//'`
    elif [ `echo "$var"| grep -c "\-config="` != 0 ]; then
        CONFIGFILE=`echo "$var"| sed 's/\-config=//'`
    elif [ `echo "$var"| grep -c "\-goodnbins="` != 0 ]; then
        GOODNBINS=`echo "$var"| sed 's/\-goodnbins=//'`
    elif [ `echo "$var"| grep -c "\-force"` != 0 ]; then
	FORCE=1
    elif [ `echo "$var"| grep -c "\-nonorm"` != 0 ]; then
	NORM=0
    elif [ `echo "$var"| grep -c "\-nohtml"` != 0 ]; then
	HTML=0
    elif [ `echo "$var"| grep -c "\-errors"` != 0 ]; then
	ERRORS=1
    elif [ `echo "$var"| grep -c "\-help"` != 0 ]; then
	usage
    else
	usage
    fi
done

[ $NFILES -eq 0 -a x$CONFIGFILE = "x" ] && usage

if [ $ERRORS -eq 1 ]; then
    for i in `seq 1 $NFILES`; do
	PLOTERRORS[$i]=1
    done
fi

OUTPUT=${OUTPUT:-overlay_plots}
OUTDIR=.
#if [ ${OUTPUT:$((${#OUTPUT}-5))}x != ".rootx" ]; then
    OUTDIR=${OUTPUT}
    mkdir -p ${OUTDIR}
    OUTPUT=${OUTPUT}/`basename $OUTPUT`.root
#fi
if [ -f $OUTPUT -a $FORCE = 0 ]; then
    echo "Output exists: $OUTPUT"
    echo "Exiting... You can force it with -force"
    exit 1
fi

cat >  compare_files.from_script <<EOF
Output.File: $OUTPUT
Histo.Type: $OVERLAY
Histo.GoodNbins: $GOODNBINS
EOF

if [ $NORM = 1 ]; then
cat >>  compare_files.from_script <<EOF
Global.Normalization: 100
EOF
fi

if [ x$CONFIGFILE = "x" ]; then
    cat >>  compare_files.from_script <<EOF
Files.Number: $NFILES
EOF
    for i in `seq 1 $NFILES`
    do
	cat >> compare_files.from_script <<EOF
Files.$i.Name: ${FILES[$i]}
Files.$i.Title: ${TITLES[$i]}
Files.$i.BaseDir:
Files.$i.DirOffset: 0
Files.$i.StartOffset: 0
Files.$i.Color: $((100+i))
Files.$i.PlotErrors: ${PLOTERRORS[$i]}
EOF
    done
else
cat $CONFIGFILE >> compare_files.from_script
fi

# This is to produce the plots:
root -q -b -x -l 'compare_histos.C("compare_files.from_script")'
# This will produce the html:
if [ $HTML -eq 1 ]; then
    cmd="root -b -q -x -l make_all_plot_html.C'(\"$OUTPUT\",\"$OUTDIR\")'"
    eval $cmd |grep -v Info
    cp compare_files.from_script ${OUTDIR}	
fi

# Get the yield:  
#cat compare_files.from_script | sed 's/Histo.Type: .*/Histo.Type: PrintYields_hb/g' > yields.list
#root -q -b -x -l 'compare_histos.C("yields.list")' > ${OUTDIR}/yield.txt
#cp yield.tex ${OUTDIR}/
#rm -f yields.list
#rm -f yield.tex
# Add the yield (verbatim mode) at the end of the html, if any:
#if [ -r ${OUTDIR}/index.html ]; then 
#    echo "<pre>" >> ${OUTDIR}/index.html
#    cat ${OUTDIR}/yield.txt >> ${OUTDIR}/index.html
#    echo "<\pre>" >> ${OUTDIR}/index.html
#fi

exit 0
