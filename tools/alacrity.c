#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <alacrity.h>
#include <ALUtil.h>

#define MIN_PART_SIZE 1

int encodeCommand(int argc, char **argv);
int decodeCommand(int argc, char **argv);
int showbinsCommand(int argc, char **argv);
int bidtovCommand(int argc, char **argv);
int showbinlensCommand(int argc, char **argv);

#define BREAKDOWN

//
// COMMAND LIST
//
typedef int (*command_func_t)(int, char **);
struct {
    const char *name;
    command_func_t func;
    const char *allowed_flags;
} static COMMANDS[] = {
        {"encode", encodeCommand, "piclesx"},
        {"decode", decodeCommand, "l"},
        {"showbins", showbinsCommand, "l"}, // show bin of each partiion
        {"bidtov", bidtovCommand, "l"},  // print bin id to value
        {"binlens", showbinlensCommand, "l"}, //  print bin whose length is less than pfd chunk size (32,64,128)
};
const int NUM_COMMANDS = sizeof(COMMANDS)/sizeof(COMMANDS[0]);

struct {
    uint64_t part_size_in_elem;

    _Bool index_form_set;
    ALIndexForm index_form;

    _Bool legacy_format;

    char significant_bits;
    ALDatatype datatype;
} static OPTIONS;

char *cmdstr;
void usage_and_exit() {
    fprintf(stderr, "Usage: alac encode [OPTIONS] IN_FILE OUT_FILEBASE\n"
                    "Usage: alac decode [OPTIONS] IN_FILEBASE OUT_FILE\n"
    				"Usage: alac showbins ALAC ENCODED VALUE PREFIX\n"
    				"Usage: alac bidtov ALAC ENCODED VALUE PREFIX\n"
                    "\n"
                    "alac encode\n"
                    "  Encodes IN_FILE (a file of raw values) into a set of files with filename\n"
                    "  prefix OUT_FILEBASE. OUT_FILEBASE may include a directory path and a\n"
                    "  prefix for the final filenames. For example, if OUT_FILENAME = /tmp/dataset-alac,\n"
                    "  then this command will produce the following files:\n"
                    "\n"
                    "    /tmp/dataset-alac-metadata.dat\n"
                    "    /tmp/dataset-alac-compressed_data.dat\n"
                    "    /tmp/dataset-alac-query_index.dat\n"
                    "    Possibly other similar files\n"
                    "\n"
                    "  Specific parameters for the encoding process are given via option flags (see below).\n"
                    "\n"
                    "Options:"
                    "\n"
                    "  -p SIZE          Specifies the partition size. SIZE is a positive integer,\n"
                    "                     optionally followed by a multipler suffix, optionally followed\n"
                    "                     by the element suffix:\n"
                    "                       Multipler suffixes:\n"
                    "                         K (10^3), Ki (2^10), M (10^6), Mi (2^20),\n"
                    "                         G (10^9), Gi (2^30), T (10^12), Ti (2^40)\n"
                    "                       Element suffix: if the suffix 'E' is appended, the partition size\n"
                    "                         is measured in elements (rather than in bytes, as default)\n"
                    " -e TYPE           Specifies the element type of the input (valid types are float, double)\n"
                    " -s NBITS          Specifies the number of significant bits for ALACRITY encoding\n"
                    "  -i, -c, -x       Produces an inverted, compression, or compressed inverted\n"
                    "                     (PForDelta) index, respectively. At most one of these flags\n"
                    "                     may be used at once (default is -i if none are specified).\n"
                    "  -l               Use the legacy ALACRITY format (does not work if -x is also\n"
                    "                     specified). This should not be used unless the produced index\n"
                    "                     must be used with old ALACRITY code, as the new format is more\n"
                    "                     space efficient.\n"
                    "\n"
                    "alac decode\n"
                    "  Decodes ALACRITY-encoded set of files with filename prefix IN_FILEBASE into a raw\n"
                    "  data file OUT_FILE. IN_FILEBASE may include a directory path and a\n"
                    "  prefix for the input filenames. For example, if IN_FILENAME = /tmp/dataset-alac,\n"
                    "  then this command will look for the following files:\n"
                    "\n"
                    "    /tmp/dataset-alac-metadata.dat\n"
                    "    /tmp/dataset-alac-compressed_data.dat\n"
                    "    /tmp/dataset-alac-query_index.dat\n"
                    "    Possibly other similar files\n"
                    "\n"
                    "  Specific parameters for the decoding process are given via option flags (see below).\n"
                    "\n"
                    "Options:"
                    "\n"
                    "  -l               Assume the input files are in the legacy ALACRITY format. This is only\n"
                    "                     the case if the data was encoded using the old ALACRITY indexing"
                    "                     code, or using the new encoder with the -l legacy format option\n"
    				"alac showbins \n"
    				"     showbins lists bin value, bin length(rid number), length percentages, and "
    		        "accumulated length percentages per partition\n"
    		        "alac bidtov  \n"
    				"     print out bin id and bin value per partition\n"
            );
    exit(1);
}

void init_options() {
    OPTIONS.part_size_in_elem = 0;
    OPTIONS.index_form = ALInvertedIndex;
    OPTIONS.index_form_set = false;
    OPTIONS.legacy_format = false;
    OPTIONS.datatype = DATATYPE_UNDEFINED;
    OPTIONS.significant_bits = 16;
}


double dclock(void) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}


double encode_time = 0, compress_time = 0, write_time = 0, total_time = 0;

int main(int argc, char **argv) {
    cmdstr = argv[0];

    init_options();

    _Bool is_part_size_in_elem = false;

    int c;
    while ((c = getopt(argc, argv, ":p:icle:s:xhkmr")) != -1) {
        switch (c) {
        case 'p':
        {
            uint64_t multi = 1;
            int arglen = strlen(optarg);
            _Bool part_size_suffix_power_2 = false;

            while (arglen > 0) {
                switch (optarg[arglen - 1]) {
                case 'e':
                case 'E':
                    // We must wait until we know the element size before we can scale
                    is_part_size_in_elem = true;
                    break;
                case 'i':
                case 'I':
                    part_size_suffix_power_2 = true;
                    break;

                    // NOTE: intentional fall-throughs
                case 't':
                case 'T':
                    multi *= part_size_suffix_power_2 ? 1024 : 1000;
                case 'g':
                case 'G':
                    multi *= part_size_suffix_power_2 ? 1024 : 1000;
                case 'm':
                case 'M':
                    multi *= part_size_suffix_power_2 ? 1024 : 1000;
                case 'k':
                case 'K':
                    multi *= part_size_suffix_power_2 ? 1024 : 1000;
                    break;

                default:
                    arglen = 0;
                    continue;
                }

                optarg[arglen-1] = 0;
                arglen--;
            }
            OPTIONS.part_size_in_elem = (uint64_t)atoll(optarg) * multi;
            if (OPTIONS.part_size_in_elem < MIN_PART_SIZE) {
                fprintf(stderr, "Error: partition size of %llu specified, but must be at least %llu\n", OPTIONS.part_size_in_elem, MIN_PART_SIZE);
                exit(1);
            }
//            printf("Using partition size of %llu(not accounting for element size, but %s later)\n", OPTIONS.part_size_in_elem, is_part_size_in_elem ? "WILL" : "WON'T");
            break;
        }

        case 'i':
        case 'c':
        case 'x':
        case 'h': //hybrid index = p4d + rle
        case 'k': //skipping index = p4d + skipping
        case 'm': //skipping hybrid index = p4d + rle + skipping
        case 'r': // expansion/relaxing method
            if (OPTIONS.index_form_set) {
                fprintf(stderr, "Error: options -i, -c and -x are mutually exclusive\n");
                usage_and_exit();
            }
            OPTIONS.index_form_set = true;
            if (c == 'i')        OPTIONS.index_form = ALInvertedIndex;
            else if (c == 'c')   OPTIONS.index_form = ALCompressionIndex;
            else if (c == 'h')   OPTIONS.index_form = ALCompressedHybridInvertedIndex; //rpfd
            else if (c == 'k')	 OPTIONS.index_form = ALCompressedSkipInvertedIndex;
            else if (c == 'm')	 OPTIONS.index_form = ALCompressedMixInvertedIndex;
            else if (c == 'r')	 OPTIONS.index_form = ALCompressedExpansionII; // epfd
            else                 OPTIONS.index_form = ALCompressedInvertedIndex; //x pfd
            break;

        case 'l':
            OPTIONS.legacy_format = true;
            break;

        case 'e':
        	if (strcasecmp(optarg, "float") == 0)
        		OPTIONS.datatype = DATATYPE_FLOAT32;
        	else if (strcasecmp(optarg, "double") == 0)
        		OPTIONS.datatype = DATATYPE_FLOAT64;
        	else {
                fprintf(stderr, "Error: element type must be one of { float | double }, but is %s\n", optarg);
                usage_and_exit();
            }
            break;

        case 's':
            OPTIONS.significant_bits = atoi(optarg);
            if (OPTIONS.significant_bits < 1 || OPTIONS.significant_bits > 32) {
                fprintf(stderr, "Error: significant byte count must be between 1 and 32, inclusive, but is %d\n", OPTIONS.significant_bits);
                usage_and_exit();
            }
            break;

        case ':':
            fprintf(stderr, "Option %c missing required argument\n", optopt);
            usage_and_exit();
            break;
        case '?':
        default:
            fprintf(stderr, "Unknown option %c\n", optopt);
            usage_and_exit();
            break;
        }
    }

    // Do some post-calculations based on all options
    if (!is_part_size_in_elem) {
        if (OPTIONS.datatype != DATATYPE_UNDEFINED)
        	OPTIONS.part_size_in_elem /= ALDatatypeGetSize(OPTIONS.datatype);
    }

    // Now that they've been parsed, skip over the options, to
    // leave only non-option args
    argc -= optind;
    argv += optind;

    // Make sure there's at least one argument for the command, then capture
    // it and advance past it
    if (argc < 1) usage_and_exit();
    const char *cmd = argv[0];
    argc--;
    argv++;

    // Find the matching command (if any), call it, and return the value it returns
    for (int i = 0; i < NUM_COMMANDS; i++)
        if (strcmp(COMMANDS[i].name, cmd) == 0)
            return COMMANDS[i].func(argc, argv);

    // If no command matches, print an error message
    fprintf(stderr, "Error: command %s unrecognized\n", cmd);
    usage_and_exit();

    // At the compiler's complaint...
    return 0;
}

//
// BEGIN COMMAND IMPLEMENTATIONS
//

// Opens a FILE pointer for input and an ALFileStore for output. Also returns the length of the input file
int encodeCommandOpenFiles(int argc, char **argv, FILE **infile, uint64_t *infile_len, ALStore *outfile) {
    if (argc != 2) usage_and_exit();

    const char *infilename = argv[0];
    const char *outbasename = argv[1];

    struct stat st;
    _Bool infile_exists = (stat(infilename, &st) == 0);
    if (!infile_exists) {
        fprintf(stderr, "Error: encoder cannot find input file %s\n", infilename);
        return 1;
    }

    *infile_len = st.st_size;

    *infile = fopen(infilename, "r");
    if (*infile == NULL) {
        fprintf(stderr, "Error: encoder cannot open input file %s\n", infilename);
        return 1;
    }

//    printf(" Using legacy format %s \n", OPTIONS.legacy_format?"y":"n");
    ALStoreOpenPOSIX(outfile, outbasename, "w", OPTIONS.legacy_format);

    return 0;
}

// Encodes the data from infile into the ALFileStore outfile, closing both afterward
int encodeCommandEncode(FILE *infile, uint64_t infile_len, ALStore *outfile) {
    const int datatypeLen = ALDatatypeGetSize(OPTIONS.datatype);

	// Allocate the input buffer
    uint64_t buflen;
    if (infile_len > (OPTIONS.part_size_in_elem * datatypeLen))
        buflen = (OPTIONS.part_size_in_elem * datatypeLen);
    else
        buflen = infile_len;

    void *inbuf = malloc(buflen); // Input buffer

    // Prepare the output buffer (encoder)
    ALIndexForm index_form = OPTIONS.index_form;
    if (index_form >= ALCompressedInvertedIndex) // all other compressed index based on inverted index
        index_form = ALInvertedIndex;

    ALEncoderConfig econfig;
    ALEncoderConfigure(&econfig, OPTIONS.significant_bits, OPTIONS.datatype, index_form);
    ALPartitionData partdata;   // Output buffer

    size_t bytesread;
    int i = 0;
    double s  ;
    encode_time = 0;
    compress_time = 0;
    write_time = 0;
    total_time = 0;
    double ss = dclock();
    while (!feof(infile) && !ferror(infile) && (bytesread = fread(inbuf, 1, buflen, infile)) != 0) {
        i++;
        dbprintf("Encoding partition %d with size %llu...\n", i, bytesread);
        s  = dclock();
        ALEncode(&econfig, inbuf, bytesread / datatypeLen, &partdata);
        encode_time  = encode_time +  (dclock() - s);

        s = dclock();
        if (OPTIONS.index_form >= ALCompressedInvertedIndex) // all other compressed index based on inverted index
            ALConvertIndexForm(&partdata.metadata, &partdata.index, OPTIONS.index_form);
        compress_time  = compress_time +  (dclock() - s);

        s = dclock();
        if (ALStoreWritePartition(outfile, &partdata) != ALErrorNone) {
            fprintf(stderr, "Error appending ALACRITY partition to file, aborting\n");
            abort();
            return 1;
        }
        write_time  = write_time +  (dclock() - s);


        ALPartitionDataDestroy(&partdata); // TODO: make this automatic when encoding over an existing partition data
        dbprintf("Partition %d done!\n", i);
    }
    total_time = dclock() - ss;
    printf("[read: %9.3lf] [encode: %9.3lf] [compress: %9.3lf] [write: %9.3lf] [total: %9.3lf]\n", total_time - (encode_time + compress_time + write_time ),  encode_time, compress_time, write_time, encode_time + compress_time + write_time , total_time);
    if (ferror(infile)) {
        fprintf(stderr, "Error reading from input file, aborting\n");
        return 1;
    }

    // Cleanup
    free(inbuf);
    fclose(infile);

    printf("Encoding complete, %llu bytes of input data successfully encoded into %llu partitions\n", infile_len, outfile->cur_partition);

    dbprintf("Closing ALACRITY output file...\n");
    if (ALStoreClose(outfile) != ALErrorNone) {
        fprintf(stderr, "Error closing ALACRITY output file, aborting\n");
        return 1;
    }

    return 0;
}

// encode <input file> <output basename>
int encodeCommand(int argc, char **argv) {
    FILE *infile;
    uint64_t infile_len;
    ALStore outfile;

    int ret;
    ret = encodeCommandOpenFiles(argc, argv, &infile, &infile_len, &outfile);
    if (ret != 0) return ret;

    ret = encodeCommandEncode(infile, infile_len, &outfile);
    if (ret != 0) return ret;

    return 0;
}

// DECODE

// Opens a FILE pointer for output and an ALFileStore for input
int decodeCommandOpenFiles(int argc, char **argv, ALStore *infile, FILE **outfile) {
    if (argc != 2) usage_and_exit();

    const char *inbasename = argv[0];
    const char *outfilename = argv[1];

//    printf("Using legacy format? %s \n", OPTIONS.legacy_format?"y":"n");
    ALError err = ALStoreOpenPOSIX(infile, inbasename, "r", OPTIONS.legacy_format);
    if (err != ALErrorNone) {
        fprintf(stderr, "Error: decoder cannot open input files with basename %s\n", inbasename);
        return 1;
    }

    *outfile = fopen(outfilename, "w");
    if (*outfile == NULL) {
        fprintf(stderr, "Error: decoder cannot open output file %s\n", outfilename);
        return 1;
    }

    return 0;
}

// Encodes the data from infile into the ALFileStore outfile, closing both afterward
int decodeCommandDecode(ALStore *infile, FILE *outfile) {
    ALPartitionData partdata;   // Output buffer
    void *outbuf;
    uint64_t outbufCount;
    uint64_t totalElemsWritten = 0;
    ALError err;
    size_t writelen;

    while (!ALStoreEOF(infile)) {
        err = ALStoreReadPartition(infile, &partdata);
        if (err != ALErrorNone) {
            fprintf(stderr, "Error reading the next partition from the input files\n");
            return 1;
        }

        outbuf = malloc(ALGetDecodeLength(&partdata));
        if (!outbuf) {
            fprintf(stderr, "Error allocating %llu btyes of temporary memory for decoding\n", ALGetDecodeLength(&partdata));
            return 1;
        }

        err = ALDecode(&partdata, outbuf, &outbufCount);
        if (err != ALErrorNone) {
            fprintf(stderr, "Error decoding partition from input files\n");
            return 1;
        }

        writelen = fwrite(outbuf, partdata.metadata.elementSize, outbufCount, outfile);
        if (writelen != outbufCount) {
            fprintf(stderr, "Error writing decoded elements to file (wrote %llu, expected to write %llu)\n",
                    (uint64_t)writelen, (uint64_t)partdata.metadata.elementSize);
            return 1;
        }

        totalElemsWritten += outbufCount;

        free(outbuf);
        ALPartitionDataDestroy(&partdata);
    }

    // Cleanup
    fclose(outfile);

    printf("Decoding complete, wrote out %llu elements\n", totalElemsWritten);

    dbprintf("Closing ALACRITY input files...\n");
    if (ALStoreClose(infile) != ALErrorNone) {
        fprintf(stderr, "Error closing ALACRITY input file, aborting (output should still be correct)\n");
        return 1;
    }

    return 0;
}

// encode <input basename> <output file>
int decodeCommand(int argc, char **argv) {
    ALStore infile;
    FILE *outfile;

    int ret;
    ret = decodeCommandOpenFiles(argc, argv, &infile, &outfile);
    if (ret != 0) return ret;

    ret = decodeCommandDecode(&infile, outfile);
    if (ret != 0) return ret;

    return 0;
}


// SHOW BINS

// Opens an ALFileStore for input
int commandOpenFiles(int argc, char **argv, ALStore *infile) {
    if (argc != 1) usage_and_exit();

    const char *inbasename = argv[0];

    ALError err = ALStoreOpenPOSIX(infile, inbasename, "r", OPTIONS.legacy_format);
    if (err != ALErrorNone) {
        fprintf(stderr, "Error: decoder cannot open input files with basename %s\n", inbasename);
        return 1;
    }

    return 0;
}

// Lists the bins and their boundaries in the given ALStore, partition by partition
int showbinsCommandShowBins(ALStore *infile) {
    ALPartitionStore pstore;
    ALMetadata meta;
    ALError err;

    bin_id_t i;
    double *d = malloc(sizeof(double));
    uint64_t zero = 0;
    while (!ALStoreEOF(infile)) {
        ALStoreOpenPartition(infile, &pstore, true);
        ALPartitionStoreReadMetadata(&pstore, &meta);

        assert(meta.datatype == DATATYPE_FLOAT64);

        printf("Partition %llu has %lu bins and %llu RIDs\n", pstore.partition_num, meta.binLayout.numBins, meta.partitionLength);
        for (i = 0; i < meta.binLayout.numBins; i++) {
            REJOIN_DATUM_BITS(d, 8, meta.significantBits, meta.binLayout.binValues[i], 0);
            //*(uint64_t*)&d = meta.binLayout.binValues[i] << (64 - meta.significantBits);
            printf("%+.6e[%10lu] = %10lu (frac. %.6e) (cum.frac. %.6e)\n",
                    *d, i, meta.binLayout.binStartOffsets[i+1] - meta.binLayout.binStartOffsets[i],
                    (double)(meta.binLayout.binStartOffsets[i+1] - meta.binLayout.binStartOffsets[i]) / meta.partitionLength,
                    (double)(meta.binLayout.binStartOffsets[i+1] - meta.binLayout.binStartOffsets[0]) / meta.partitionLength);
        }

        ALPartitionStoreClose(&pstore);
        free(meta.binLayout.binStartOffsets);
        free(meta.binLayout.binValues);
        if (meta.indexMeta.indexForm == ALCompressedInvertedIndex)
            free(meta.indexMeta.u.ciim.indexBinStartOffsets);
    }

    free(d);

    dbprintf("Closing ALACRITY input files...\n");
    if (ALStoreClose(infile) != ALErrorNone) {
        fprintf(stderr, "Error closing ALACRITY input file, aborting (output should still be correct)\n");
        return 1;
    }

    return 0;
}

int showbinsCommand(int argc, char **argv) {
    ALStore infile;

    int ret;
    ret = commandOpenFiles(argc, argv, &infile);
    if (ret != 0) return ret;

    ret = showbinsCommandShowBins(&infile);
    if (ret != 0) return ret;

    return 0;
}



// Lists the percentage of bins  whose length is less than PFD chunk size (32,64,or 128)  in the given ALStore, partition by partition
int showbinlensCommandShowBins(ALStore *infile) {
    ALPartitionStore pstore;
    ALMetadata meta;
    ALError err;
    int PFD_chunksizes[3] = {32,64,128};
    bin_id_t bin_lens_counts[3] = {0}; // the number of bins whose length is less than 32, 64, or 128
    bin_id_t i;
    double *d = malloc(sizeof(double));
    uint64_t zero = 0;
    while (!ALStoreEOF(infile)) {
        ALStoreOpenPartition(infile, &pstore, true);
        ALPartitionStoreReadMetadata(&pstore, &meta);

        for(int k = 0; k < 3 ; k ++) bin_lens_counts[k]=0;

        assert(meta.datatype == DATATYPE_FLOAT64);
        bin_id_t total_bins = meta.binLayout.numBins;
        for (i = 0; i < total_bins ; i++) {
            REJOIN_DATUM_BITS(d, 8, meta.significantBits, meta.binLayout.binValues[i], 0);

            bin_offset_t bin_len = meta.binLayout.binStartOffsets[i+1] - meta.binLayout.binStartOffsets[i];
            for (int k = 0 ; k < 3 ; k++){
            	if (bin_len < PFD_chunksizes[k]){
            		bin_lens_counts[k] ++;
            	}
            }
        }
        printf("Partition %llu has %lu bins and %llu RIDs, # of bins whose length < 32,64,128 ="
        		"{[%lu, %lu, %lu],[%4.2lf,%4.2lf,%4.2lf]}\n", pstore.partition_num, total_bins , meta.partitionLength
        		, bin_lens_counts[0], bin_lens_counts[1], bin_lens_counts[2]
        		, bin_lens_counts[0]*100.0/total_bins, bin_lens_counts[1]*100.0/total_bins, bin_lens_counts[2]*100.0/total_bins);

        ALPartitionStoreClose(&pstore);
        free(meta.binLayout.binStartOffsets);
        free(meta.binLayout.binValues);
        if (meta.indexMeta.indexForm == ALCompressedInvertedIndex)
            free(meta.indexMeta.u.ciim.indexBinStartOffsets);
    }

    free(d);

    dbprintf("Closing ALACRITY input files...\n");
    if (ALStoreClose(infile) != ALErrorNone) {
        fprintf(stderr, "Error closing ALACRITY input file, aborting (output should still be correct)\n");
        return 1;
    }

    return 0;
}



int showbinlensCommand(int argc, char **argv) {
    ALStore infile;

    int ret;
    ret = commandOpenFiles(argc, argv, &infile);
    if (ret != 0) return ret;

    ret = showbinlensCommandShowBins(&infile);
    if (ret != 0) return ret;

    return 0;
}


// Lists the bins and their boundaries in the given ALStore, partition by partition
int bidtovCommandConvert(ALStore *infile) {
    ALPartitionStore pstore;
    ALMetadata meta;
    ALError err;

    bin_id_t i;
    double *d = calloc(1, sizeof(double));
    uint64_t zero = 0;
    while (!ALStoreEOF(infile)) {
        ALStoreOpenPartition(infile, &pstore, true);
        ALPartitionStoreReadMetadata(&pstore, &meta);

        assert(meta.datatype == DATATYPE_FLOAT64);

        const int insigbits = (meta.elementSize<<3) - meta.significantBits;
        const uint64_t lomask = (1ULL << (insigbits - 2));

        printf("Partition %llu has %lu bins and %llu RIDs\n", pstore.partition_num, meta.binLayout.numBins, meta.partitionLength);
        for (i = 0; i < meta.binLayout.numBins; i++) {
            REJOIN_DATUM_BITS(d, 8, meta.significantBits, meta.binLayout.binValues[i], lomask);
            //*(uint64_t*)&d = meta.binLayout.binValues[i] << (64 - meta.significantBits);
            printf("%10lu -> %+.10e\n", i, *d);
        }

        ALPartitionStoreClose(&pstore);
        free(meta.binLayout.binStartOffsets);
        free(meta.binLayout.binValues);
        if (meta.indexMeta.indexForm == ALCompressedInvertedIndex)
            free(meta.indexMeta.u.ciim.indexBinStartOffsets);
    }

    free(d);

    dbprintf("Closing ALACRITY input files...\n");
    if (ALStoreClose(infile) != ALErrorNone) {
        fprintf(stderr, "Error closing ALACRITY input file, aborting (output should still be correct)\n");
        return 1;
    }

    return 0;
}

int bidtovCommand(int argc, char **argv) {
    ALStore infile;

    int ret;
    ret = commandOpenFiles(argc, argv, &infile);
    if (ret != 0) return ret;

    ret = bidtovCommandConvert(&infile);
    if (ret != 0) return ret;

    return 0;
}

