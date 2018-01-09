#!/usr/bin/env bash
# produce_geiger_sd_modified_script_lyon.sh
debug=1

echo "Starting..." >&2

for i in {1..10}
do 
    echo "i = $i"
    UNIQUE_ID=${i}
    TRIGGER_PROBABILITY=`echo "${i} * 0.01" | bc -l `
    echo "UNIQUE_ID=" $UNIQUE_ID 
    echo "TRIGGER_PROBABILITY=" $TRIGGER_PROBABILITY
    INPUT_SD_RUN=0
    NUMBER_OF_FILES=1
    qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 produce_geiger_sd_modified_script_lyon.sh -r ${INPUT_SD_RUN} -n ${NUMBER_OF_FILES} -m same_cell_trigger -p ${TRIGGER_PROBABILITY} -u ${UNIQUE_ID}
    
done
