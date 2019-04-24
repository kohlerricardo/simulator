#!/bin/bash

if [ -z "$1" ]
then
    echo "Basename folder empty"
    exit
else
    bash exec_bundle_ricardo.sh ${1}
    bash exec_bundle_milad.sh ${1}
fi