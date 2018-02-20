#!/bin/bash
# multi_produce_geiger_sd_modified_script_lyon.sh

debug=1

echo "Starting..." >&2

which bc
if [ $? -ne 0 ];
then
    echo '[ERROR] bc does not exist!!!!'
    exit 1
else 
    echo 'bc exist'
fi

ANALYSIS_LOG_WORK_DIR=/sps/nemo/scratch/golivier/software/Analysis/logs/
echo "ANALYSIS_LOG_WORK_DIR=" ${ANALYSIS_LOG_WORK_DIR}

for i in {1..15}
do 
    echo "i = $i"
    UNIQUE_ID=${i}
    TRIGGER_PROBABILITY=`echo "${i} * 0.01" | bc -l `
    INPUT_SD_RUN=3
    # run_0 : TL208, 1 : Bi214, 2 : 0nu, 3 : 2nu
    NUMBER_OF_FILES=4

    echo "INPUT_SD_RUN=" ${INPUT_SD_RUN}
    echo "UNIQUE_ID=" ${UNIQUE_ID}
    echo "TRIGGER_PROBABILITY=" ${TRIGGER_PROBABILITY}
    echo "NUMBER_OF_FILES=" ${NUMBER_OF_FILES}

    qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=50000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} produce_geiger_sd_modified_script_lyon.sh -r ${INPUT_SD_RUN} -n ${NUMBER_OF_FILES} -m "same_cell_trigger" -p ${TRIGGER_PROBABILITY} -u ${UNIQUE_ID}
    qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=50000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} produce_geiger_sd_modified_script_lyon.sh -r ${INPUT_SD_RUN} -n ${NUMBER_OF_FILES} -m "neighbourg_trigger" -p ${TRIGGER_PROBABILITY} -u ${UNIQUE_ID}
done
