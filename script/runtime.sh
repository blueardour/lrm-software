# !/ bin/bash

if [ $# -lt 1 ]; then echo Command is needed; exit -1; fi

program=`basename $1`

$1 2>&1 1>runtime.log &

pid=`ps -e | grep $program | awk '{print $1}'`

if [ ! -d /proc/$pid ]
then
  echo "*** program not start ***"
else
  echo "*** program id: $pid ***"
  while true
  do
    sleep 4
    if [ ! -f /proc/$pid/status ]; then echo "Unexpected: no status file"; break; fi
    cat /proc/$pid/status | grep '(sleeping)' # > /dev/null
    if [ $? -eq 0 ]; then cat /proc/$pid/status > thread.log; break; fi
  done
fi


