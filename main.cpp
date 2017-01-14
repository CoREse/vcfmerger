/* File: main.cpp
 * Author: CRE
 * Last Edited: Sat Jan 14 14:37:18 2017
 */

#include "vcfmerger.h"
#include <vector>
#include <set>
#include <string>
#include <string.h>
#include "crelib/crelib.h"
using namespace std;
using namespace cre;

static inline void printHelp()
{
	die("Usage:\n"\
			"\tvcfmerger vcf1 vcf2 ... > merged.vcf\n"\
			"or\tvcfmerger vcf1 vcf2 ... -o merged.vcf\n"\
			"Warning: This program may generate or overwrite several temporary files(less than input files) in this directory, they are %s* and %s*. If the program ends normal, they would be deleted as well. Although I don't think you have those files in your directory, just be aware of it.", TEMP_FILE_NAME1, TEMP_FILE_NAME2);
}

set<string> AllocatedTempFiles;

void removeTempFiles()
{
	for (set<string>::iterator i=AllocatedTempFiles.begin();i!=AllocatedTempFiles.end();++i) remove(i->c_str());
}

string getTempFileName(const char * Prefix, int n)
{
	string Name(Prefix);
	Name+=to_string(n);
	AllocatedTempFiles.insert(Name);
	return Name;
}

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
		}
		else
		{
			InFileNames.push_back(argv[i]);
		}
	}
	if (InFileNames.size()<2) printHelp();
	uint FilesToMerge=InFileNames.size();
	const char *Prefix=TEMP_FILE_NAME1, *LPrefix=TEMP_FILE_NAME2;
	string TempFileName;
	while(FilesToMerge>1)
	{
		if (FilesToMerge==2)
		{
			merge2(InFileNames[0].c_str(),InFileNames[1].c_str(),OutFile);
			break;
		}
		for (uint i=0;i<FilesToMerge/2;++i)
		{
			TempFileName=getTempFileName(Prefix,i);
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
