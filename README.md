# VCF Merger
## What is vcfmerger?
Like its name, it merges multiple vcf files into one. It does things similar to vcf-merge in vcf-tools.

Although vcf-merge might do the same thing, it is considerably slow, and for big size of data, I don't even dare to try.

This tool is written in C++, and aims to merge numerous (say, 1 million) human genome vcfs togather, and to do it efficiently.

## Notice
1. It only works for human genome which have 22 + X + Y chromesomes and MT.
2. It only works for GT type of vcf.

## PBWT version
When two files have different alts at same position, it simply merge them into two lines of vcf file, for speed and simplicity. I know it violents vcf specifications, but for pbwt, it doesn't matter, as pbwt will divide multi alts into different sites.

## Make and Installation
```
git clone https://github.com/CoREse/vcfmerger
cd vcfmerger
make
sudo make install
```

or you can do

```
make PREFIX=/some/prefix install
```

to indicate your own prefix(instead of the default /usr/bin).

If you want to remove the program, just

```
make remove
```

or

```
make PREFIX=/some/prefix remove
```

## Usage
You can run vcfmerger without arguments to see help.
