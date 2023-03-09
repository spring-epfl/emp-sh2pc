#!/bin/bash

exec 2>&1

N=${1}
IP=${2}
RUNS=${3}

log="emp_x_server.log"

cd '/psi/emp'

echo "N:" ${N} >>${log}

i=0
while [ $i -lt $RUNS ]
do
    port=$(expr 20000 + ${N} '*' 10 + ${i})
    { /usr/bin/time -f '%e, %U, %S' ./bin/test_ms_x 1 ${port} ${N} ${IP} >>${log} ; } 2>>${log}
    i=$[$i+1]
    sleep 1
done
