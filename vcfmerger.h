#ifndef VCF_MERGER_H
#define VCF_MERGER_H

#define BUFFER_SIZE 320000000
#define NAME_BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 1024
#define ALT_BUFFER_SIZE 20480000

void merge2(const char * LFileName, const char * RFileName, const char * OutFileName);//OutFileName="" refers to stdout

#define TEMP_FILE_NAME1 ".VCF_MERGER_TEMP_FILE.1.tmp"
#define TEMP_FILE_NAME2 ".VCF_MERGER_TEMP_FILE.2.tmp"

#endif
