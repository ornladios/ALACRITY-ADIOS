#!/bin/bash

TMP1="t1.log"
TMP2="t2.log"

for log in `ls *.log_*` ; do 
#log="epd_bmap.log_SEP"  
	#delete qsub output
	sed '/^qsub.*$/d' $log  > $TMP1
	sed '/^total.*$/d' $TMP1  > $TMP2
	sed '/^final.*$/d' $TMP2  > $TMP1
	#delete /tmp/work/..
	sed '/^\/tmp\/work.*$/d' $TMP1 > $TMP2
	#delete md5 output , '-r' is reuqired because the {} repetition is enabled in RE
	sed -r '{/^[a-z0-9]{32}/d}' $TMP2 > $TMP1
	##########ONLY this command work for deleting ^M special character 
        ###########tried `sed -e 's/^M//g' input > output
	tr -d "\015" < $TMP1 > $TMP2
	F_OUT=$log"_p" # final output has the "_p" as suffix at end of file name	
	#delete empty line 
	sed '/^$/d' $TMP2 > $F_OUT


done



