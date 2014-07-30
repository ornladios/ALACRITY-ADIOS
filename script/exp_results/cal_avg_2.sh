#!/bin/bash

FILES=$1
NUM=$2
for log_p in $FILES ; do

	cat $log_p |  awk -v avg=$NUM '
	BEGIN {
	   total=0;
	   line = 0;
	}
	{
	   total +=$1;
	   line +=1 ;
	   if (line % avg == 0) {
		   print total/line; 
		   total=0;
		   line = 0;
	   }
	}
	END {
	}
	' > $log_p"_avg"
done



