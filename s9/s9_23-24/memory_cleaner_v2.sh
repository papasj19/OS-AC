#!/bin/bash

ME=`whoami`
MSG='Deleting'

IPCS_S=`ipcs -s | grep $ME | cut -f2 -d" "`
IPCS_M=`ipcs -m | grep $ME | cut -f2 -d" "`
IPCS_Q=`ipcs -q | grep $ME | cut -f2 -d" "`

echo 'Deleting...'

for id in $IPCS_M; 
do
 echo $MSG 'shared memory' $id;
 ipcrm -m $id;
done

for id in $IPCS_S; 
do
  echo $MSG 'semaphore' $id;
  ipcrm -s $id;
done

for id in $IPCS_Q; 
do
  echo $MSG 'queue' $id;
  ipcrm -q $id;
done

echo 'Done'
