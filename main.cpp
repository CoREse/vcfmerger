/* File: main.cpp
 * Author: CRE
 * Last Edited: Mon Jan 16 13:00:43 2017
 */

#include "vcfmerger.h"
#include <vector>
#include <set>
#include <string>
#include <string.h>
#include "crelib/crelib.h"
#include <omp.h>
using namespace std;
using namespace cre;

static inline void printHelp()
{
	die("Usage:\n"\
			"\tvcfmerger vcf1 vcf2 ... > merged.vcf\n"\
			"or\tvcfmerger vcf1 vcf2 ... -o merged.vcf\n"\
			"and\t-t n for multithreading\n"\
			"Warning: This program may generate or overwrite several temporary files(less than input files) in this directory, they are %s* and %s*. If the program ends normal, they would be deleted as well. Although I don't think you have those files in your directory, just be aware of it.", TEMP_FILE_NAME1, TEMP_FILE_NAME2);
}

set<string> AllocatedTempFiles;

void removeTempFiles()
{
	for (set<string>::iterator i=AllocatedTempFiles.begin();i!=AllocatedTempFiles.end();++i) remove(i->c_str());
}

omp_lock_t NameLock;
string getTempFileName(const char * Prefix, int n)
{
	string Name(Prefix);
	Name+=to_string(n);
	
	omp_set_lock(&NameLock);
	AllocatedTempFiles.insert(Name);
	omp_unset_lock(&NameLock);
	return Name;
}

uint Threads=1;

int main (int argc, char ** argv)
{
	if (argc<=1) printHelp();

	atexit(removeTempFiles);

	vector<string> InFileNames;
	char OutFile[NAME_BUFFER_SIZE];
	OutFile[0]='\0';
	for (int i=1;i<argc;++i)
	{
		if (argv[i][0]=='-')
		{
			if (argv[i][1]=='o')
			{
				if (i+1>=argc) printHelp();
				strcpy(OutFile, argv[++i]);
			}
			else if (argv[i][1]=='t'&& argv[i][2]=='\0')
			{
				if (i+1>=argc) printHelp();
				Threads=atoi(argv[++i]);
			}
		}
		else
		{
			InFileNames.push_back(argv[i]);
		}
	}
	if (InFileNames.size()<2) printHelp();
	uint FilesToMerge=InFileNames.size();
	const char *Prefix=TEMP_FILE_NAME1, *LPrefix=TEMP_FILE_NAME2;
	while(FilesToMerge>1)
	{
		if (FilesToMerge==2)
		{
			merge2(InFileNames[0].c_str(),InFileNames[1].c_str(),OutFile);
			break;
		}
		omp_set_num_threads(Threads);
		omp_init_lock(&NameLock);
		#pragma omp parallel for
		for (int i=0;i<FilesToMerge/2;++i)
		{
			string TempFileName=getTempFileName(Prefix,i);
			merge2(InFileNames[i].c_str(), InFileNames[FilesToMerge-1-i].c_str(), TempFileName.c_str());
			InFileNames[i]=TempFileName;
		}
		if (FilesToMerge%2) FilesToMerge=FilesToMerge/2+1;
		else FilesToMerge/=2;
		const char * Temp=Prefix;
		Prefix=LPrefix;
		LPrefix=Temp;
	}
	return 0;
}
