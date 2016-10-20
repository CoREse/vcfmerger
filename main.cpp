/* File: main.cpp
 * Author: CRE
 * Last Edited: Thu Oct 20 15:53:41 2016
 */

#include "vcfmerger.h"
#include <vector>
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
			"Warning: This program may generate or overwrite two temporary files in this directory, they are %s and %s. If the program ends normal, they would be deleted as well. Although I don't think you have those files in your directory, just be aware of it.", TEMP_FILE_NAME1, TEMP_FILE_NAME2);
}

void removeTempFiles()
{
	remove(TEMP_FILE_NAME1);
	remove(TEMP_FILE_NAME2);
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
	char *LeftFileName, *RightFileName;
	if (InFileNames.size()==2) merge2(InFileNames[0].c_str(), InFileNames[1].c_str(), OutFile);
	else
	{
		merge2(InFileNames[0].c_str(), InFileNames[1].c_str(), TEMP_FILE_NAME1);
		LeftFileName=TEMP_FILE_NAME1;
		RightFileName=TEMP_FILE_NAME2;
		for (uint i=2;i<InFileNames.size()-1;++i)
		{
			merge2(LeftFileName, InFileNames[i].c_str(), RightFileName);
			char * Temp=LeftFileName;
			LeftFileName=RightFileName;
			RightFileName=Temp;
		}
		merge2(LeftFileName, InFileNames[InFileNames.size()-1].c_str(), OutFile);
	}
	return 0;
}
