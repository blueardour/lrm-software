# ! /bin/bash

dir=~/workspace/lrm/database/human_g1k_v37
reference=reference.fa
query=read.fa

index=index-2

Length=3000

cd $dir
if [ -d $Length ]; then cd $Length; else mkdir $Length; cd $Length; fi

if [ ! -f $reference ]; then cp ../demo/$reference .; fi

# generate read.fa
if [ ! -f $query ]
then
	wgsim -N 1000 -1 $Length -d0 -S11 -e0 -r0.02 $reference $query /dev/null
	fastqToFa read.fq $query
fi

# generate index
if [ ! -d $index ]; then mkdir $index; fi
rm -rf $index/*

index -l $Length -i 8 -b 10 -d $index -r $reference -v 3
if [ $? -ne 0 ]; then echo "Index Error"; exit -1; fi

#sort -u index/${reference}.uspt  -v 1
#if [ $? -ne 0 ]; then echo "Sort Error"; exit -1; fi


