#!/bin/bash
# multi_produce_geiger_sd_modified_script_lyon.sh

debug=1

echo "Starting..." >&2

ANALYSIS_LOG_WORK_DIR=/sps/nemo/scratch/golivier/software/Analysis/logs/
echo "ANALYSIS_LOG_WORK_DIR=" ${ANALYSIS_LOG_WORK_DIR}

# Input raw SD
for i in {4..6}
do
    INPUT_SD_RUN=${i}
    # DB info : "/sps/nemo/scratch/golivier/SD_data_MCC_1/database.db"
    # run_0 : TL208, 1 : Bi214 source bulk, 2 : 0nu, 3 : 2nu, 4 : Bi214 source surface, 5 : Bi214 field surface, 6 : Bi214 field bulk
    
    # Input modified SD for same_cell_trigger and neighbourg_trigger
    # for j in {1..15}
    # do
    # 	INPUT_SD_MODIF_RUN=${j}
    # 	MODE="same_cell_trigger"
    # 	# qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=50000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} trigger_sd_modified_lyon.sh -m ${MODE} -s ${INPUT_SD_RUN} -r ${INPUT_SD_MODIF_RUN}

    # 	MODE="neighbourg_trigger"
    # 	# qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=50000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} trigger_sd_modified_lyon.sh -m ${MODE} -s ${INPUT_SD_RUN} -r ${INPUT_SD_MODIF_RUN}
    # 	#echo "INPUT_SD_MODIF_RUN_${INPUT_SD_MODIF_RUN} done !"
    # done

    # Only trigger efficiency (only 1 SD / run = input_run_${i}/run_0 atm)
    MODE="trigger_efficiency"
    qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=50000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} trigger_sd_modified_lyon.sh -m ${MODE} -s ${INPUT_SD_RUN}
    
    echo "INPUT_SD_RUN_${INPUT_SD_RUN} done !"
done