alac_for_adios_query
====================

===============Install ALACRITY===========================
DEPENDENCES:
===gcc 4.8.2
===automake 1.11.1 autoconf 2.69, and libtool 2.6.2b
You can download them from links below
http://ftp.gnu.org/gnu/automake/
http://ftp.gnu.org/gnu/autoconf/
https://ftp.gnu.org/gnu/libtool/

1. Build timer package   [git@github.com:ncsu-samatova/Timer.git]
   1.1 cd [TIMER SOURCE CODE]
   1.2 ./configure --prefix=[TIMER INSTALL PATH]
   1.3 make ; make install 
2. Build PForDelta package [git@github.com:ncsu-samatova/PForDelta.git]
   1.1 cd [PForDelta PATH] 
   1.2 make libridcompress.a    // this will produce libridcompress.a in current folder
3. Build ALACRITY [git@github.com:ncsu-samatova/ALACRITY.git]
   1.1 cd [ALACRITY PATH]
   1.2 Copy ./runconf to ./myrunconf
   1.3 Edit ./myrunconf, setting the paths for timer and indexcompression, and your output build path
   1.4 Run ./myrunconf

==============Index & Query ============================
1. Build Index: 
   1.1 cd [alalrity install path]
   1.2 The executable file 'alac' is the command we need. It has following arguments ( type ./build/bin/alac, we can see the detail usage): 
         'encode': indicates to indexing the data
         -p      : partition size, we can specify total size or total element number
         -i | -x : the format of index, -i means the inverted index, but -x means the compressed inverted index
         -s[Number]   :  the number of bits it will use
         [input data path]
         [output data path] : this path require a prefix of the files the alarity will produce
    For example, if we want to build compressed inverted indexes for 2GB double `temp` precision data  by binning 16 significant bits
    csc$ ./build/bin/alac encode -p268435456E -x -edouble -s16 ~/sigmod_2012_2GB/temp ~/data/ii_index/s3d/1part_2G/pfd/temp 
    Using partition size of 268435456(not accounting for element size, but WILL later)
    Using legacy format n 
    Encoding complete, 2147483648 bytes of input data successfully encoded into 1 partitions

2. Perform Query: 
   2.1 cd [alalrity install path]
   2.2 The executable file 'uniquery' is the command we need. It has following arguments ( type ./build/bin/uniquery, we can see the detail usage): 
	[input data base path]: the path is the output path of the indexing stage
        [low boundary value]
        [high boundary value]
     For example, if we want to query the `temp` data  is between 35.65  and 663.6, we do:
        csc$ ./build/bin/uniquery ~/data/ii_index/s3d/1part_2G/pfd/temp 35.65 663.6
        Total results: 11041898 (4.1134% selectivity, 1/1 partitions had results)
        uniquery avg. time 0.261354 

===============ASSUMPTIONS=========================================
1. ALACRITY now only takes at most 2GB partition data size. 
2. The output of ALACRITY query does not output all RIDs. 

