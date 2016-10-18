# VCF Merger (WIP)
<h1 style="color:red"> Warning: This is a project that is working in progress(WIP), it is NOT fully functional! </h1>
## What is vcfmerger?
Like its name, it merges multiple vcf files into one. It does things similar to vcf-merge in vcf-tools.

Although vcf-merge might do the same thing, it is considerably slow, and for big size of data, I don't even dare to try.

This tool is written in C++, and aims to merge numerous (say, 1 million) human genome vcfs togather, and to do it efficiently.
