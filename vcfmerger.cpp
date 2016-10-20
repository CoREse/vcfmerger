/* File: vcfmerger.cpp
 * Author: CRE
 * Last Edited: Thu Oct 20 14:52:19 2016
 */

#include "crelib/crelib.h"
#include "vcfmerger.h"
#include <iostream>
#include <cstring>
#include <string>
#include <list>
#include <set>
#include <sstream>
#include <vector>
using namespace cre;
using namespace std;

static inline void getRootName(const char * FileName, char * RootName);
static inline void mergeHead(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const char * RRootName, uint & LSampleNum, uint & RSampleNum);
static inline void mergeSites(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const uint &LSampleNum, const uint &RSampleNum);

static inline void removeTailingN(char * str, uint Length=0)//to remove a tailing \n(if there is one).You can provide the Length of Buffer to avoid recalculating.

{
	if(Length==0) Length=strlen(str);
	if (Length>0&&str[--Length]=='\n')
	{
		str[Length]='\0';
	}
}

static inline int chromCompare(const char * A, const char * B)//return -1 if A<B, 0 if A=B, 1 if A>B
{
	if (strcmp(A,"MT")==0)
	{
		if (strcmp(A,B)==0) return 0;
		else return 1;
	}
	if (strcmp(B,"MT")==0)
	{
		if (strcmp(A,B)==0) return 0;
		else return -1;
	}
	return strcmp(A,B);
}

void merge2(const char * LFileName, const char * RFileName, const char * OutFileName)
{
	char LRootName[NAME_BUFFER_SIZE], RRootName[NAME_BUFFER_SIZE];
	getRootName(LFileName,LRootName);
	getRootName(RFileName,RRootName);
	FILE * LFile=fopen(LFileName, "r");
	if (LFile==NULL) die("Open left file failed!");
	FILE * RFile=fopen(RFileName, "r");
	FILE * OutFile=NULL;
	if (RFile==NULL) die("Open right file failed!");
	if (OutFileName[0]=='\0') OutFile=stdout;
	else
	{
		OutFile=fopen(OutFileName, "w");
		if (OutFile==NULL) die("Open out file failed!");
	}

	char *LBuffer=myalloc(BUFFER_SIZE,char);
	char *RBuffer=myalloc(BUFFER_SIZE,char);

	uint LSampleNum, RSampleNum;

	mergeHead(LFile,RFile,OutFile, LBuffer, RBuffer, RRootName, LSampleNum, RSampleNum);
	mergeSites(LFile,RFile,OutFile, LBuffer, RBuffer, LSampleNum, RSampleNum);

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

static inline void mergeHeader(char* LBuffer, char* RBuffer, const char * RRootName, FILE* OutFile, uint &LSampleNum, uint &RSampleNum);
static inline void mergeHead(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const char * RRootName, uint &LSampleNum, uint &RSampleNum)
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
	mergeHeader(LBuffer, RBuffer, RRootName, OutFile, LSampleNum, RSampleNum);
}

static inline void mergeHeader(char* LBuffer, char* RBuffer, const char* RRootName, FILE* OutFile, uint & LSampleNum, uint & RSampleNum)
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
	LSampleNum=LSamples.size();
	RSampleNum=RSamples.size();

	free(Buffer);
}

class Site
{
	Site& operator=(const Site&) {return *this;}
	Site(const Site&) {};
	public:
	uint Pos;
	char *Chrom;
	Site()
		:Pos(0)
	{
		Chrom=myalloc(NAME_BUFFER_SIZE, char);
	}
	~Site()
	{
		if (Chrom!=NULL) free(Chrom);
	}
	bool operator<(const Site&B) const
	{
		if (chromCompare(Chrom, B.Chrom)>0) return false;
		if (chromCompare(Chrom,B.Chrom)<0) return true;
		return Pos<B.Pos;
	}
	bool operator>(const Site&B) const
	{
		if (chromCompare(Chrom,B.Chrom)<0) return false;
		if (chromCompare(Chrom,B.Chrom)>0) return true;
		return Pos>B.Pos;
	}
	bool operator==(const Site&B) const
	{
		if (Pos==B.Pos&&chromCompare(Chrom,B.Chrom)==0) return true;
		return false;
	}
};

static inline void getSite(const char * Buffer, Site& TheSite)
{
	sscanf(Buffer,"%s %u",TheSite.Chrom, &(TheSite.Pos));
}

static inline void getRefNAlt(const char * Buffer, char * Ref, char * Alt)
{
	sscanf (Buffer, "%s %s %s %s %s", Ref,Ref,Ref, Ref, Alt);
}

static inline uint findTheDataOutset(const char * Buffer, uint Length=0)//return the index of the first varriant call, you can give the Length of Buffer to avoid recalculating.
{
	if (Length==0) Length=strlen(Buffer);
	uint TableCount=0;//there are 9 tabs before data
	for (uint i=0;i<Length;++i)
	{
		if (Buffer[i]=='\t') ++TableCount;
		if (TableCount==9)
		{
			return i+1;
		}
	}
	die("Something wrong with the data(not enough \\t before data!).");
	return 0;
}

static inline void mergeSites(FILE* LFile, FILE* RFile, FILE* OutFile, char* LBuffer, char* RBuffer, const uint &LSampleNum, const uint &RSampleNum)
{
	uint LLength, RLength, RDIndex;
	Site LSite, RSite;
	char* LRef=myalloc(ALT_BUFFER_SIZE, char);
	char* RRef=myalloc(ALT_BUFFER_SIZE, char);
	char* LAlt=myalloc(ALT_BUFFER_SIZE, char);
	char* RAlt=myalloc(ALT_BUFFER_SIZE, char);
	bool LReside=false, RReside=false;
	do
	{
		if (!LReside) LBuffer[0]='\0';
		if (!RReside) RBuffer[0]='\0';
		if (!feof(LFile)&&!LReside) fgets(LBuffer, BUFFER_SIZE, LFile);
		if (!feof(RFile)&&!RReside) fgets(RBuffer, BUFFER_SIZE, RFile);
		LReside=false;
		RReside=false;
		LLength=strlen(LBuffer);
		RLength=strlen(RBuffer);
		if (LLength==0&&RLength==0) break;
		if (LLength==0)
		{
			removeTailingN(RBuffer,RLength);
			RDIndex=findTheDataOutset(RBuffer,RLength);
			fprintf(OutFile,"\n");
			fwrite(RBuffer, 1, RDIndex-1, OutFile);//doesn't include the last \t
			for (uint i=0;i<LSampleNum;++i)
			{
				fprintf(OutFile,"\t.|.");
			}
			if (RSampleNum!=0) fprintf(OutFile,"\t%s",RBuffer+RDIndex);
		}
		else if (RLength==0)
		{
			removeTailingN(LBuffer,LLength);
			fprintf(OutFile,"\n%s", LBuffer);
			for (uint i=0;i<RSampleNum;++i) fprintf(OutFile,"\t.|.");
		}
		else
		{
			getSite(LBuffer,LSite);
			getSite(RBuffer,RSite);
			if (LSite<RSite)
			{
				RReside=true;
				removeTailingN(LBuffer,LLength);
				fprintf(OutFile,"\n%s", LBuffer);
				for (uint i=0;i<RSampleNum;++i) fprintf(OutFile,"\t.|.");
			}
			else if (LSite>RSite)
			{
				LReside=true;
				RDIndex=findTheDataOutset(RBuffer,RLength);
				fprintf(OutFile,"\n");
				fwrite(RBuffer, 1, RDIndex-1, OutFile);//doesn't include the last \t
				for (uint i=0;i<LSampleNum;++i)
				{
					fprintf(OutFile,"\t.|.");
				}
				removeTailingN(RBuffer,RLength);
				if (RSampleNum!=0) fprintf(OutFile,"\t%s",RBuffer+RDIndex);
			}
			else
			{
				getRefNAlt(LBuffer,LRef, LAlt);
				getRefNAlt(RBuffer,RRef, RAlt);

				if (strcmp(LRef, RRef)==0&&strcmp(LAlt,RAlt)==0)
				{
					removeTailingN(LBuffer,LLength);
					removeTailingN(RBuffer,RLength);
					RDIndex=findTheDataOutset(RBuffer,RLength);
					fprintf(OutFile,"\n%s", LBuffer);
					if (RSampleNum!=0) fprintf(OutFile,"\t%s",RBuffer+RDIndex);
				}
				else
				{
					RReside=true;
					removeTailingN(LBuffer,LLength);
					fprintf(OutFile,"\n%s", LBuffer);
					for (uint i=0;i<RSampleNum;++i) fprintf(OutFile,"\t.|.");
				}
			}
		}
	}
	while(!(feof(LFile)&&feof(RFile)));
	free(LRef);
	free(RRef);
	free(LAlt);
	free(RAlt);
}
