data=test.log
tmpdata=$data".tmp"
num=$(awk -F"," 'NR==1 { print NF }' $data)
print $num

i=1
while (( $i <= $num ))
do
newline=''
for val in $(cut -d"," -f$i $data)
do
newline=$newline$val","
done
nline=`print ${newline%?}`
print $nline >> $tmpdata
(( i = i + 1 ))
done
