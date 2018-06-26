#!/bin/bash

#######################################################################
#
# Copyright (C) 2018 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

if [ "$#" -ne 1 ]; then
    echo "USAGE: ./perf.sh numbers"
    exit;
fi
num=$1

capped() {
    num=$1
    lists=$2
    cores=$3

    ./perf $num $lists $cores > radix.single &
    pid=$!

    cmax=0
    first="true"
    while true
    do
        sleep 0.1

        cat /proc/$pid/status > radix.threads 2>/dev/null
        status=$?
        if [ $status -ne 0 ]
        then
            break
        fi

        grep "Generate time:" radix.single >/dev/null
        if [ $? -eq 0 ]
        then
            threads=`cat radix.threads | grep Threads | awk '{print $2'}`
            cmax=$(( threads > cmax ? threads : cmax ))
            #echo -n $threads
            echo -n .
        else
            echo -n -
        fi
    done

    echo ""
    echo -n "Capped $lists lists $cores cores: "
    fail=`grep FAIL radix.single | wc -l`
    pass=`grep PASS radix.single | wc -l`

    if [[ fail -gt 0 ]]
    then
        echo "FAIL ( functional )"
    else
        if [[ pass -gt 0 ]]
        then
            (( cmax -= 2 ))
            #echo "max $cmax cores $cores"

            if [ $cmax -le $cores ]
            then
                echo "PASS"
            else
                echo "FAIL ( too many cores: $cmax, should have been no more than $cores )"
            fi
        else
            echo "FAIL ( functional )"
        fi
    fi

    sleep 1;
}

for lists in 1 2 
do
    capped $num $lists 10
done

for lists in 1 2 
do
    capped $num $lists 15
done

for lists in 1 2  
do
    capped $num $lists 20
done

rm radix.single radix.quad radix.threads 2>/dev/null
