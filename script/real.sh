#! /bin/bash


basedir=/workspace/lrm

soft=$basedir/lrm-software/bwa-sw/bwa-0.6.2-gmon
utils=$basedir/lrm-software/utils
workspace=$basedir/workspace
database=/data/lrm/database

human=$database/hs_ref_GRCh37.p5/hs_ref_GRCh37.p5_split.fa

runtime_dir=$workspace/$(basename $PWD)

program="bwa-gmon"
generator="generate_queries_v4"
extract="extract"
seeds="seeds"
census="census_char_v1.1"
gprof="gprof"
prefix="hs"

export PATH=$soft:$utils:$PATH

function build_reference_index
{
  reference=$human
  index=$reference
  name=`basename $reference .fa`

  if [ -f $index.bwt ]; then echo "Index may exist"; exit 0; fi

  cd ${index%/*}
  echo -n "In "
  pwd

  echo "$program index -a bwtsw $reference"
  $program index -a bwtsw $reference 2>&1 | tee build_${name}_index-runtime.log &

  pid=`ps -e | grep $program | awk '{print $1}'`
  if [ ! -d /proc/$pid ]
  then
    echo "*** program not start ***"
  else
    echo "*** program id: $pid ***"
    while true
    do
	  if [ ! -f /proc/$pid/status ]; then break; fi
      cat /proc/$pid/status | grep '(sleeping)' > /dev/null
      if [ $? -eq 0 ]; then break; fi
      sleep 3
    done
    if [ -f /proc/$pid/status ]; then cat /proc/$pid/status > build_${name}_index-thread.log; fi
  fi

  if [ -f gmon.out ]; then mv gmon.out $name-gmon.out; fi
}

function gen_query
{
  length=$1
  amount=$2
  error=$3
	reference=$human
	name=`basename $reference .fa`
	sequence=${reference%/*}/${name}.ex

	if [ ! -f $sequence ]
	then
		$extract -d $reference -f $sequence
		if [ $? -ne 0 ]; then echo Extract fa file failed!; exit -1; fi
	fi

  cd $runtime_dir
  folder=$prefix-A$amount-L$length-E$error
 
  if [ -d $folder ]; then cd $folder; else mkdir $folder; cd $folder; fi
  if [ -f $folder.fa ]
  then
    echo "FA file exist!"
  else
    echo "$generator -a $amount -l $length -e $error -d $sequence -f $folder.fa"
    time $generator -a $amount -l $length -e $error -d $sequence -f $folder.fa
    if [ $? -eq 0 ] ; then echo "Generate OK" ; else echo "*** Query file generate Failed ***"; exit -2; fi
  fi
}

function lrm_multi_thread
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7
	index=$human

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq
 
  if [ "$file" == "$prefix-A-L-E-Z-S" ]; then echo "Error input"; exit 5; fi

  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else mkdir $folder; cd $folder; fi  
  pwd

  if [ -f $folder.fa ]
  then
    echo "FA file exist!"
  else
		gen_query $length $amount $error
  fi
  
  if [ -f gmon.out ]; then mv -v gmon.out gmon.out-bak; else echo "gmon.out not exist"; fi
  if [ -f $file.gmon.out -a -f $file.log -a -f $file.time ]
  then
    echo "Maybe already aligned"
    return 1;
  elif [ ! -f $folder.fa ]
  then
    echo "*** Qurey file not exist ***"
  else
    echo "Align $folder.fa:"
    if [ "$seq" == "454" ]
    then
      echo "{ time -p $program bwasw -z $z -s $s -t $thread $index $folder.fa 1>$file.sam 2>$file.log ; } 2>$file.time"
      { time -p $program bwasw -z $z -s $s -t $thread $index $folder.fa 1>$file.sam 2>$file.log ; } 2>$file.time
    else
      echo "{ time -p $program bwasw -b 5 -q 2 -r 1 -z $z -s $s -t $thread $index $folder.fa 1>$file.sam 2>$file.log ; } 2>$file.time"
      { time -p $program bwasw -b 5 -q 2 -r 1 -z $z -s $s -t $thread $index $folder.fa 1>$file.sam 2>$file.log ; } 2>$file.time
    fi
  fi 

  if [ -f gmon.out ]; then mv gmon.out $file.gmon.out; else echo "gmon.out not generate"; exit -1; fi
  
  echo "Generate bitwise"
  grep -v SQ $file.sam | awk '{print $1}' | grep [0-9] > $file.bitwise
  echo "Generate quality"
  grep -v SQ $file.sam | awk '{print $4}' | grep [0-9] > $file.quality
  echo -n "$census -b $file.bitwise -q $file.quality > $file.census"
  $census -b $file.bitwise -q $file.quality > $file.census
  echo  ".....$?"

  cd ..
}
  
function gen_census
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq
 
  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "$folder not exist"; exit -1; fi  
  echo "Generate bitwise"
  grep -v SQ $file.sam | awk '{print $1}' | grep [0-9] > $file.bitwise
  echo "Generate quality"
  grep -v SQ $file.sam | awk '{print $4}' | grep [0-9] > $file.quality
  echo -n "$census -b $file.bitwise -q $file.quality > $file.census"
  $census -b $file.bitwise -q $file.quality > $file.census
  echo  ".....$?"
  cd ..

	cd -
}
  
function speed
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  [ -z "$6" ] || thread=$6
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file
 
  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "No such folder"; fi
 
  if [ ! -f ${file}-mt${thread}.log ]
  then
    echo "File not generate yet!"; return 2;
  fi

  cost=`awk  '/CPU/{print $7}' ${file}-mt${thread}.log`
  echo -n -e "$cost\t"
  cd ..
}
 
function format_time
{
  tmp=0m0.0s 
  [ -z "$1" ] || tmp=$1

  echo $tmp | grep 'm'
  if [ $? -eq 0 ]
  then
    min=`echo $tmp | awk -Fm '{print $1}'`
    sec=`echo $tmp | awk -Fm '{print $2}'`
    sec=`echo $sec  | awk -Fs '{print $1}'`
    tmp=`echo "scale=1; $min * 60 + $sec" | bc`
  fi
}
 
 
function real_time
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  [ -z "$6" ] || thread=$6
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file
 
  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "No such folder"; fi
 
  if [ ! -f time-mt${thread}.log ]; then echo "File not generate yet!"; return 3; fi

  cost=`awk  '/real/{print $2}' time-mt${thread}.log`
  format_time $cost
  cost=$tmp
  echo -n -e "$cost\t"
  cd ..
}
 

function cpu_time
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  [ -z "$6" ] || thread=$6
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file

  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "No such folder"; fi

  if [ ! -f time-mt${thread}.log ]; then echo "File not generate yet!"; return 4; fi

  user_time=`awk  '/user/{print $2}' time-mt${thread}.log`
  format_time $user_time
  user_time=$tmp
  sys_time=`awk  '/sys/{print $2}' time-mt${thread}.log`
  format_time $sys_time
  sys_time=$tmp
  cost=`echo "scale=2; $user_time + $sys_time" | bc`
  echo -n -e "$cost\t"
  cd ..
}

function core_rate
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq
  
  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "No such folder"; fi

  if [ ! -f $file.gmon.out ]; then echo "File not generate yet!"; return 5; fi

  rate=`$gprof $soft/$program $file.gmon.out -p | awk '/aln_extend_core/{print $1}'`
  echo -n -e "$rate\t"
  cd ..
}

function max_rate
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  [ -z "$6" ] || thread=$6
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file

  cd $runtime_dir

  if [ -d $folder ]; then cd $folder; else echo "No such folder"; fi

  if [ ! -f gmon.out-mt$thread ]; then echo "File not generate yet!"; return 6; fi

  rate=`$gprof $soft/$program gmon.out-mt$thread -p | awk 'NR==6{print $1}'`
  echo -n -e "$rate\t"
  cd ..
}


function census_rate
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file
  cd $folder
  step1=0
  step2=0
  step3=0

  rate=`$gprof $soft/$program gmon.out -p | awk '/bwtl_seq2bwtl/{print $1}'`
  step1=`echo "scale=2;$step1 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/is_sa/{print $1}'`
  step1=`echo "scale=2;$step1 + $rate" | bc`

  rate=`$gprof $soft/$program gmon.out -p | awk '/bsw2_core/{print $1}'`
  step2=`echo "scale=2;$step2 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bwt_2occ4/{print $1}'`
  step2=`echo "scale=2;$step2 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bwtl_2occ4/{print $1}'`
  step2=`echo "scale=2;$step2 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bwt_2occ$/{print $1}'`
  step2=`echo "scale=2;$step2 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bsw2_connectivity/{print $1}'`
  step2=`echo "scale=2;$step2 + $rate" | bc`
  
  rate=`$gprof $soft/$program gmon.out -p | awk '/aln_extend_core/{print $1}'`
  step3=`echo "scale=2;$step3 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bsw2_extend_left/{print $1}'`
  step3=`echo "scale=2;$step3 + $rate" | bc`
  rate=`$gprof $soft/$program gmon.out -p | awk '/bsw2_extend_rght/{print $1}'`
  step3=`echo "scale=2;$step3 + $rate" | bc`

  echo  -e "$step1,$step2,$step3\t"
  cd ..
}


function Q20
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq
  
  cd $folder
  rate=`awk -F : '/Q20/{print $5}' $file.census`
  rate=`echo $rate | sed -e 's/\r//g'`
  echo -n -e "$rate\t"
  cd ..
}

function error
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq

  cd $folder
  rate=`awk  'NR==11{print $2}' $file.census`
  rate=`echo $rate | sed -e 's/\r//g'`
  echo -n -e "$rate\t"
  cd ..
}

function clean_log
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  [ -z "$6" ] || thread=$6

  file=$prefix-A$amount-L$length-E$error-Z$z-S$s
  folder=$file

  if [ -f $folder/$file-mt$thread.log ];    then rm "$folder/$file-mt$thread.log"; else echo "Missing $folder/$file-mt$thread.log"; fi
  if [ -f $folder/$file-mt$thread.census ]; then rm "$folder/$file-mt$thread.census"; else echo "Missing $folder/$file-mt$thread.census"; fi
  if [ -f $folder/$file-mt$thread.bitwise ];then rm "$folder/$file-mt$thread.bitwise"; else echo "Missing $folder/$file-mt$thread.bitwise"; fi
  if [ -f $folder/$file-mt$thread.quality ];then rm "$folder/$file-mt$thread.quality"; else echo "Missing $folder/$file-mt$thread.quality"; fi
  if [ -f $folder/gmon.out-mt$thread ];     then rm "$folder/gmon.out-mt$thread"; else echo "Missing $folder/gmon.out-mt$thread"; fi
  if [ -f $folder/time-mt$thread.log ];     then rm "$folder/time-mt$thread.log"; else echo "Missing $folder/time-mt$thread.log"; fi
}

function package_list
{
  length=$1
  amount=$2
  error=$3
  z=$4
  s=$5
  thread=1
  seq=454
  [ -z "$6" ] || thread=$6
  [ -z "$7" ] || seq=$7

  folder=$prefix-A$amount-L$length-E$error
  file=$prefix-A$amount-L$length-E$error-Z$z-S$s-$thread-$seq

  if [ -f $folder/$file.log ];    then echo "$folder/$file.log"    >> $tar_list; else echo "Missing $folder/$file.log"; fi
  if [ -f $folder/$file.census ]; then echo "$folder/$file.census" >> $tar_list; else echo "Missing $folder/$file.census"; fi
  if [ -f $folder/$file.bitwise ];then echo "$folder/$file.bitwise">> $tar_list; else echo "Missing $folder/$file.bitwise"; fi
  if [ -f $folder/$file.quality ];then echo "$folder/$file.quality">> $tar_list; else echo "Missing $folder/$file.quality"; fi
  if [ -f $folder/$file.gmon.out ];then echo "$folder/$file.gmon.out" >> $tar_list; else echo "Missing $folder/$file.gmon.out"; fi
  if [ -f $folder/$file.time ];   then echo "$folder/$file.time" >> $tar_list; else echo "Missing $folder/$file.time"; fi
}

function package
{
  instant=$1

  ver=`date +%Y-%m-%d-%H-%M`
  tar_files=$program-$prefix-$ver

  tar_file=$tar_files.tar
  tar_list=$tar_files.list

  if [ ! -f $tar_list ]; then touch $tar_list; fi
  
	$instant package_list

  list=`cat $tar_list`
  tar cvf $tar_file $list
  if [ $? -ne 0 ]; then exit 255; else rm $tar_list; fi

}


function default
{
  operation=$1
	mt=1
	s=3
	z=1

  for z in 1
  do
    for s in 3 # 5 7 9 11
    do
      $operation 100 100000 2  $z $s
      $operation 100 100000 5  $z $s

      $operation 200 50000 2   $z $s
      $operation 200 50000 5   $z $s

      $operation 500 20000 2   $z $s
      $operation 500 20000 5   $z $s

      $operation 1000 10000 2   $z $s
      $operation 1000 10000 5   $z $s

      $operation 2000 5000 2   $z $s
      $operation 2000 5000 5   $z $s

      $operation 4000 2500 2   $z $s
      $operation 4000 2500 5   $z $s

      $operation 100 100000 10  $z $s $mt pacbio  
      $operation 200 50000  10  $z $s $mt pacbio
      $operation 500 20000  10  $z $s $mt pacbio
      $operation 1000 10000 10  $z $s $mt pacbio
      $operation 2000 5000  10  $z $s $mt pacbio
      $operation 4000 2500  10  $z $s $mt pacbio
    done
  done
}


function run_mt
{
  operation=$1
  z=1
  s=3
  mt=1

  for z in 1 # 3 5
  do
		#for mt in 1 2 3 4 5
    for mt in 1 # 3 5 7 9 11 13 15
    do 
      $operation 100 100000 2  $z $s $mt
      $operation 200 50000  2  $z $s $mt
      $operation 500 20000  2  $z $s $mt
      $operation 1000 10000 2  $z $s $mt
      $operation 2000 5000  2  $z $s $mt
      $operation 4000 2500  2  $z $s $mt
      echo ""
    done

		#for mt in 1 2 3 4 5
    for mt in 1 # 3 5 7 9 11 13 15
    do 
      $operation 100 100000 5  $z $s $mt
      $operation 200 50000  5  $z $s $mt
      $operation 500 20000  5  $z $s $mt
      $operation 1000 10000 5  $z $s $mt
      $operation 2000 5000  5  $z $s $mt
      $operation 4000 2500  5  $z $s $mt
      echo ""
    done
  done

  # -b5 -q2 -r1 -z10, PacBio
  for z in 10 #12 14
  do
		#for mt in 1 2 3 4 5
    for mt in 1 # 3 5 7 9 11 13 15
    do 
      $operation 100 100000 10  $z $s $mt pacbio  
      $operation 200 50000  10  $z $s $mt pacbio
      $operation 500 20000  10  $z $s $mt pacbio
      $operation 1000 10000 10  $z $s $mt pacbio
      $operation 2000 5000  10  $z $s $mt pacbio
      $operation 4000 2500  10  $z $s $mt pacbio
      echo ""
    done
  done
}

function test
{
  operation=$1
  z=1
  s=3
  mt=1
  $operation 100 100000 2 $z $s $mt
}

operations="core_rate lrm_multi_thread Q20 error gen_census package build_reference_index gen_query"
operation=lrm_multi_thread
instants="default run_mt test"
instant=run_mt
check=0

onces="build_reference_index gen_cal"
once=0

if [ $# == 0 ]
then
  echo -e "Usage: $0 [$operations] [$instants]";
  exit 0; 
fi 

for i in $operations
do
  if [ "$1" == "$i" ]; then operation=$i; check=1; else continue; fi
done

for i in $onces
do
  if [ "$1" == "$i" ]; then once=1; else continue; fi
done

for i in $instants
do
  if [ "$2" == "$i" ]; then instant=$i; else continue; fi
done

if [ $check -eq 0 ]; then echo -e "Usage: $0 [$operations] [$instants]"; exit 0; fi
if [ $once  -eq 0 ]; then $instant $operation; else $operation $instant $2 $3 $4; fi



