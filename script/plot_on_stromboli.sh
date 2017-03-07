#!/bin/bash

echo "plot_on_stromboli.sh starts with argument: " $@

echo "-- Synchronizing with local repo"
./synchronize.sh stromboli

echo "-- Loading python 2.7"
source activate py27

echo "-- Looping over listed input JSONFiles"
for JSONFile in "$@"
do
    prefix=$(python parse_json.py $JSONFile prefix)

    scpt stromboli ../input/inputJSON/${prefix}.json /home/tauzin/Workspace/lbm_solver/input/inputJSON/

    sshp stromboli "screen -dm bash -c 'cd Workspace/lbm_solver/script && ./plot.sh ../input/inputJSON/${prefix}.json &> plot_${prefix}.out'"

done

echo "plot_on_stromboli.sh ends"
