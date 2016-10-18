/* File: vcfmerger.cpp
 * Author: CRE
 * Last Edited: Tue Oct 18 14:42:54 2016
 */

#include "crelib/crelib.h"
#include <iostream>
#include <cstring>
#include <string>
#include <list>
#include <set>
#include <sstream>
#include <vector>
using namespace cre;
using namespace std;

#define BUFFER_SIZE 320000000
#define NAME_BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 1024

static inline void getRootName(const char * FileName, char * RootName);
static inline void mergeHead(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const char * RRootName);
static inline void mergeSites(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer);

void merge2(const char * LFileName, const char * RFileName, const char * OutFileName)
{
	char LRootName[NAME_BUFFER_SIZE], RRootName[NAME_BUFFER_SIZE];
	getRootName(LFileName,LRootName);
	getRootName(RFileName,RRootName);
	FILE * LFile=fopen(LFileName, "r");
	if (LFile==NULL) die("Open left file failed!");
	FILE * RFile=fopen(RFileName, "r");
	if (RFile==NULL) die("Open right file failed!");
	FILE * OutFile=fopen(OutFileName, "w");
	if (OutFile==NULL) die("Open out file failed!");

	char *LBuffer=myalloc(BUFFER_SIZE,char);
	char *RBuffer=myalloc(BUFFER_SIZE,char);

	mergeHead(LFile,RFile,OutFile, LBuffer, RBuffer, RRootName);
	mergeSites(LFile,RFile,OutFile, LBuffer, RBuffer);

	free(LBuffer);
	free(RBuffer);

	fclose(LFile);
	fclose(RFile);
	fclose(OutFile);
}

static inline void getRootName(const char * FileName, char * RootName)
{
	strcpy(RootName,FileName);
	if (strlen(FileName)>4)
	{
		if (strcmp(FileName+strlen(FileName)-4, ".vcf")==0)
		{
			RootName[strlen(FileName)-4]='\0';
		}
	}
}

static inline void mergeHeader(char* LBuffer, char* RBuffer, const char * RRootName, FILE* OutFile);
static inline void mergeHead(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const char * RRootName)
{
	list<string> LHeads, RHeads;
	//get L heads
	while(fgets(LBuffer, BUFFER_SIZE, LFile))
	{
		uint Length=strlen(LBuffer);
		if (Length==0) die("Read heads error");
		if (Length>=2&&LBuffer[0]=='#'&&LBuffer[1]=='#')
		{
			if (Length==BUFFER_SIZE-1) die("Buffer may not be enough!");
			if (Length>0&&LBuffer[Length-1]=='\n')
			{
				LBuffer[--Length]='\0';
			}
			LHeads.push_back(LBuffer);
		}
		else break;
	}
	//get R heads
	while(fgets(RBuffer, BUFFER_SIZE, RFile))
	{
		uint Length=strlen(RBuffer);
		if (Length==0) die("Read heads error");
		if (Length>=2&&RBuffer[0]=='#'&&RBuffer[1]=='#')
		{
			if (Length==BUFFER_SIZE-1) die("Buffer may not be enough!");
			if (Length>0&&RBuffer[Length-1]=='\n')
			{
				RBuffer[--Length]='\0';
			}
			RHeads.push_back(RBuffer);
		}
		else break;
	}
	//remove duplicated heads and output LHeads
	for (list<string>::iterator i=LHeads.begin();i!=LHeads.end();++i)
	{
		fprintf(OutFile,"%s\n",i->c_str());
		for (list<string>::iterator j=RHeads.begin();j!=RHeads.end();++j)
		{
			if (*i==*j)
			{
				j=RHeads.erase(j);
				if (j!=RHeads.begin()) --j;
			}
		}
	}
	//output RHeads
	for (list<string>::iterator j=RHeads.begin();j!=RHeads.end();++j)
	{
		fprintf(OutFile,"%s\n",j->c_str());
	}
	mergeHeader(LBuffer, RBuffer, RRootName, OutFile);
}

static inline void mergeHeader(char* LBuffer, char* RBuffer, const char* RRootName, FILE* OutFile)
{
	if (LBuffer[0]!='#') die("Can't read left's header!");
	if (RBuffer[0]!='#') die("Can't read right's header!");
	char * Buffer = myalloc(SMALL_BUFFER_SIZE, char);
	stringstream SS;
	set<string> LSamplesSet, RSamplesSet;
	vector<string> LSamples, RSamples;
	SS.str(LBuffer);
	SS>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer;//They are #CHROM POS ID REF ALT QUAL FILTER INFO FORMAT
	//get L samples
	while(SS>>Buffer)
	{
		if(SS.eof()) break;
		if (LSamplesSet.count(Buffer)!=0) die("Left file has duplicate samplenames!");
		LSamplesSet.insert(Buffer);
		LSamples.push_back(Buffer);
	}
	SS.clear();
	SS.str(RBuffer);
	SS>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer>>Buffer;//They are #CHROM POS ID REF ALT QUAL FILTER INFO FORMAT
	//get R samples
	while(SS>>Buffer)
	{
		if(SS.eof()) break;
		if (LSamplesSet.count(Buffer)!=0)//to add a root name and a _ as prefix
		{
			uint RRootLength=strlen(RRootName);
			uint BufferLength=strlen(Buffer);
			if (RRootLength+BufferLength+1>=SMALL_BUFFER_SIZE) die("Small buffer is too small!");
			memmove(Buffer+RRootLength+1, Buffer, BufferLength+1);
			memcpy(Buffer, RRootName, RRootLength);
			Buffer[RRootLength]='_';
		}
		if (RSamplesSet.count(Buffer)!=0) die("Left file has duplicate samplenames!");
		RSamplesSet.insert(Buffer);
		RSamples.push_back(Buffer);
	}
	uint Length=strlen(LBuffer);
	if (Length>0&&LBuffer[--Length]=='\n')
	{
		LBuffer[Length]='\0';
	}
	fprintf(OutFile, "%s", LBuffer);
	for (uint i=0;i<RSamples.size();++i)
	{
		fprintf(OutFile, "\t%s", RSamples[i].c_str());
	}

	free(Buffer);
}

static inline void mergeSites(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer)
{}

int main ()
{
	merge2("a.vcf", "b.vcf", "c.vcf");
	return 0;
}
