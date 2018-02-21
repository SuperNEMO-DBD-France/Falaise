#!/usr/bin/env bash
# multi_self_trigger_script_lyon.sh

debug=1
START_DATE=`date "+%Y-%m-%d"`

echo "Starting..." >&2

ANALYSIS_LOG_WORK_DIR=/sps/nemo/scratch/golivier/software/Analysis/logs/
echo "ANALYSIS_LOG_WORK_DIR=" ${ANALYSIS_LOG_WORK_DIR}

ST_OUTPUT="/sps/nemo/scratch/golivier/software/Analysis/self_trigger/"
mkdir -p ${ST_OUTPUT}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${ST_OUTPUT} FAILED !"
    exit 0
fi

SELF_TRIGGER_DB="/sps/nemo/scratch/golivier/software/Analysis/self_trigger/database.db"
touch ${SELF_TRIGGER_DB}
TIME_INTERVAL=1 # seconds

# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 1 -s ${TIME_INTERVAL} -c 1 -t 0.1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 2 -s ${TIME_INTERVAL} -c 10 -t 0.1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 3 -s ${TIME_INTERVAL} -c 100 -t 0.1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 4 -s ${TIME_INTERVAL} -c 1000 -t 0.1

# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 5 -s ${TIME_INTERVAL} -c 1 -t 1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 6 -s ${TIME_INTERVAL} -c 10 -t 1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 7 -s ${TIME_INTERVAL} -c 100 -t 1
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 8 -s ${TIME_INTERVAL} -c 1000 -t 1

# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 9 -s ${TIME_INTERVAL} -c 1 -t 10
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 10 -s ${TIME_INTERVAL} -c 10 -t 10
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 11 -s ${TIME_INTERVAL} -c 100 -t 10
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 12 -s ${TIME_INTERVAL} -c 1000 -t 10

# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 13 -s ${TIME_INTERVAL} -c 1 -t 100
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 14 -s ${TIME_INTERVAL} -c 10 -t 100
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 15 -s ${TIME_INTERVAL} -c 100 -t 100
# qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 16 -s ${TIME_INTERVAL} -c 1000 -t 100

qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 17 -s ${TIME_INTERVAL} -c 1 -t 500
qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 18 -s ${TIME_INTERVAL} -c 10 -t 500
qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 19 -s ${TIME_INTERVAL} -c 100 -t 500
qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=8000M,sps=1 -q huge@@C6320IImc -e ${ANALYSIS_LOG_WORK_DIR} -o ${ANALYSIS_LOG_WORK_DIR} self_trigger_script.sh -r 20 -s ${TIME_INTERVAL} -c 1000 -t 500


# echo "run_1 ${TIME_INTERVAL} 1 0.1" >> ${SELF_TRIGGER_DB}
# echo "run_2 ${TIME_INTERVAL} 10 0.1" >> ${SELF_TRIGGER_DB}
# echo "run_3 ${TIME_INTERVAL} 100 0.1" >> ${SELF_TRIGGER_DB}
# echo "run_4 ${TIME_INTERVAL} 1000 0.1" >> ${SELF_TRIGGER_DB}

# echo "run_5 ${TIME_INTERVAL} 1 1" >> ${SELF_TRIGGER_DB}
# echo "run_6 ${TIME_INTERVAL} 10 1" >> ${SELF_TRIGGER_DB}
# echo "run_7 ${TIME_INTERVAL} 100 1" >> ${SELF_TRIGGER_DB}
# echo "run_8 ${TIME_INTERVAL} 1000 1" >> ${SELF_TRIGGER_DB}

# echo "run_9 ${TIME_INTERVAL} 1 10" >> ${SELF_TRIGGER_DB}
# echo "run_10 ${TIME_INTERVAL} 10 10" >> ${SELF_TRIGGER_DB}
# echo "run_11 ${TIME_INTERVAL} 100 10" >> ${SELF_TRIGGER_DB}
# echo "run_12 ${TIME_INTERVAL} 1000 10" >> ${SELF_TRIGGER_DB}

# echo "run_13 ${TIME_INTERVAL} 1 100" >> ${SELF_TRIGGER_DB}
# echo "run_14 ${TIME_INTERVAL} 10 100" >> ${SELF_TRIGGER_DB}
# echo "run_15 ${TIME_INTERVAL} 100 100" >> ${SELF_TRIGGER_DB}
# echo "run_16 ${TIME_INTERVAL} 1000 100" >> ${SELF_TRIGGER_DB}

# echo "run_17 ${TIME_INTERVAL} 1 500" >> ${SELF_TRIGGER_DB}
# echo "run_18 ${TIME_INTERVAL} 10 500" >> ${SELF_TRIGGER_DB}
# echo "run_19 ${TIME_INTERVAL} 100 500" >> ${SELF_TRIGGER_DB}
# echo "run_20 ${TIME_INTERVAL} 1000 500" >> ${SELF_TRIGGER_DB}


