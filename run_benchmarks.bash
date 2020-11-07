#!/bin/bash

# sample raw output from hyperfine
# | command                 |   mean | stddev | median |    user | system |    min |     max |
# |-------------------------+--------+--------+--------+---------+--------+--------+---------|
# | xsv-search+select+sort  | 91.339 |  5.375 | 91.606 | 126.303 | 12.542 | 83.054 | 102.054 |
# | csvm-select+cols+sort   | 62.334 |  3.791 | 61.661 |  81.697 | 11.041 | 56.091 |  73.056 |
# | csvm-select+cols+sort*2 | 53.898 |  3.934 | 53.534 |  90.992 | 14.448 | 47.680 |  67.617 |


function hfcsv2org {
    sed 1d | while read line; do
	echo $line | awk -F , '{printf "|%s|%0.03f|%0.03f (%3d%%)|%0.03f (%3d%%)|\n", $1, $2, $5, 100*$5/$2, $6, 100*$6/$2 }'

	# with ± stddev
	#echo $line | awk -F , '{printf "|%s|%0.03f ± %0.03f|%0.03f (%d%%)|%0.03f (%d%%)|\n", $1, $2*1000, $3*1000, $5*1000, 100*$5/$2, $6*1000, 100*$6/$2 }'

	# just print
	# echo $line | awk -F , '{printf "|%s|", $1; for(ii=2;ii<=NF;ii++) printf "%0.03f|", $ii*1000; printf "\n"}'
    done
    echo "|-"
}

export -f hfcsv2org

largeFile="100mb.csv"
veryLargeFile="1000mb.csv"
bmarkOut="benchmark-gen.org"

echo -e "|command|avg [s]| cputime(user) [s] | cputime(system) [s]|\n|<l>|<r>|<r>|<r>|\n|-" > $bmarkOut

hyperfine --warmup 3 --export-csv >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:100mb-filter rows'        "bin/csvm     -f $largeFile 'select(type==\"q\");'" \
	-n 'xsv :100mb-filter rows'         "xsv search -s type q $largeFile" \
	-n 'csvm:100mb-filter rows*4'      "bin/csvm -n4 -f $largeFile 'select(type==\"q\");'"

hyperfine --warmup 3 --export-csv >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:100mb-filter columns'        "bin/csvm     -f $largeFile 'cols(date,arrTm,bidPx,askPx)'" \
	-n 'xsv :100mb-filter columns'	 "xsv select date,arrTm,bidPx,askPx $largeFile" \
	-n 'csvm:100mb-filter columns*4'      "bin/csvm -n4 -f $largeFile 'cols(date,arrTm,bidPx,askPx)'"

hyperfine --warmup 3 --export-csv >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:100mb-sort'        "bin/csvm     -f $largeFile 'sort(askPx,arrTm)'" \
	-n 'xsv :100mb-sort'	 "xsv sort -s askPx,arrTm $largeFile" \
	-n 'csvm:100mb-sort*2'      "bin/csvm -n2 -f $largeFile 'sort(askPx,arrTm)'"

hyperfine --warmup 3 --export-csv >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:100mb-filter rows+columns'      "bin/csvm     -f $largeFile 'select(type==\"q\"); cols(date,arrTm,bidPx,askPx)'" \
	-n 'xsv :100mb-filter rows+columns'	 "xsv search -s type q $largeFile | xsv select date,arrTm,bidPx,askPx" \
	-n 'csvm:100mb-filter rows+columns*4'    "bin/csvm -n4 -f $largeFile 'select(type==\"q\"); cols(date,arrTm,bidPx,askPx)'"

hyperfine --warmup 3 --export-csv >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:1000mb-filter rows+columns'        "bin/csvm     -f $veryLargeFile 'select(type==\"q\"); cols(date,arrTm,bidPx,askPx)'" \
	-n 'xsv :1000mb-filter rows+columns'	 "xsv search -s type q $veryLargeFile | xsv select date,arrTm,bidPx,askPx" \
	-n 'csvm:1000mb-filter rows+columns*4'      "bin/csvm -n4 -f $veryLargeFile 'select(type==\"q\"); cols(date,arrTm,bidPx,askPx)'"

hyperfine --warmup 3 --export-csv  >(hfcsv2org >> $bmarkOut) \
	-n 'csvm:100mb:filter rows+columns+sort'	 "bin/csvm     -f $largeFile 'select(type==\"q\");cols(date,arrTm,bidPx,askPx);sort(askPx,arrTm);'" \
	-n 'xsv :100mb:filter rows+columns+sort'	 "xsv search -s type q $largeFile | xsv select date,arrTm,bidPx,askPx | xsv sort -s askPx,arrTm" \
	-n 'csvm:100mb:filter rows+columns+sort*2'	 "bin/csvm -n2 -f $largeFile 'select(type==\"q\");cols(date,arrTm,bidPx,askPx);sort(askPx,arrTm);'"
