# ! /bin/bash

dir=/data/lrm/database/human_g1k_v37/demo
reference=reference.fa
query=read.fa

Findex=fingerprint-index

PATH=/workspace/lrm/lrm-software/public-tools:/workspace/lrm/lrm-software/fingerprint:/workspace/lrm/lrm-software/bwa-sw/bwa-0.6.2:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games


cd $dir

if [ ! -d $Findex ]; then mkdir $Findex; fi
rm -rf $Findex/*
index -l 1000 -i 10 -b 20 -r $reference -v 3
if [ $? -ne 0 ]; then echo "Index Error"; exit -1; fi
mv $reference.* $Findex

#sort -u $Findex/$reference.uspt -p ACGT -v 3
#if [ $? -ne 0 ]; then echo "Sort Error"; exit -1; fi




