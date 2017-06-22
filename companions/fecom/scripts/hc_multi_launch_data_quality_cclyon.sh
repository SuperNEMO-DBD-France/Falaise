#!/usr/bin/env bash
# hc_multi_launch_data_quality_cclyon.sh

APP_NAME=hc_multi_launch_data_quality_cclyon.sh
debug=1

echo "Starting..." >&2

echo "Are you sure to process ? "
echo "If yes, edit this file and comment exit at the begining"
echo "Exiting" 
exit 1

function usage(){
    echo "--------------"
    echo "Goal : Multi Analyze half commissioning raw data with qsubs at CC Lyon."
    echo "Don't forget to adapt which run you want to process etc..."
    echo "--------------"
    echo "How to use it"
    echo " "
    echo "$ ./hc_multi_launch_data_quality_cclyon.sh [OPTIONS] [ARGUMENTS]"
    echo ""
    echo "Allowed options: "
    echo "-h  [ --help ]     produce help message"
    echo "-n  [ --number ]   set the number of events"
    echo " "
    echo "./hc_multi_launch_data_quality_cclyon.sh -n number_of_events"
    echo "Default value : number_of_events = 10"
    echo " "
    echo "--------------"
    echo "Example : "
    echo "./hc_multi_launch_data_quality_cclyon.sh -n 100000"
    echo " "
}


#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
nb_event=10

while [ -n "$1" ];
do
    arg="$1"
    arg_value="$2"
    if [ "x$arg" = "x-h" -o "x$arg" = "x--help" ]; then
        usage
        exit 0
    fi
    if [ "x$arg" = "x-n" ]; then
        nb_event=$arg_value
    fi
    shift 2
done

SCRIPT_DIR=/sps/nemo/scratch/golivier/software/Falaise/trunk/companions/fecom/scripts
SINGLE_QUALITY_SCRIPT=hc_launch_data_quality_cclyon.sh

# Change Run min / max 
RUN_MIN=3
RUN_MAX=52

for (( i=${RUN_MIN}; i <= ${RUN_MAX}; ++i ))
do
    echo "RUN_NUMBER=" ${i}

    INPUT_DIR="/sps/nemo/scratch/golivier/data_half_commissioning/"
    INPUT_SERIALIZED_DIR="${INPUT_DIR}/serialized/Run_${i}/builded/"
    INPUT_FILES=`ls -v ${INPUT_SERIALIZED_DIR}/*.brio`

    if [ $? -ne 0 ];
    then
    	echo "WARNING : No serialized brio data files in ${INPUT_SERIALIZED_DIR} !"
        continue
    fi
    INPUT_CONF_DIR="${INPUT_DIR}/serialized/Run_${i}/settings/"

    ANALYZED_OUTPUT_PATH="/sps/nemo/scratch/golivier/data_half_commissioning/analyzed/Run_${i}/"
    ANALYZED_ROOT_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/root_files
    ANALYZED_BRIO_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/brio_files
    ANALYZED_STAT_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/stat_files
    LOG_DIR=${ANALYZED_OUTPUT_PATH}/log_files.d
    SERIALIZED_ROOT_DATA_FORMAT_PATH="/sps/nemo/scratch/golivier/data_half_commissioning/root/Run_${i}/"
    OUTPUT_CONF_DIR="${ANALYZED_OUTPUT_PATH}/settings/"
    LOG_FILE=${LOG_DIR}/multi_launch_quality.log

    mkdir -p  ${ANALYZED_OUTPUT_PATH} ${ANALYZED_ROOT_OUTPUT_PATH} ${ANALYZED_BRIO_OUTPUT_PATH} ${ANALYZED_STAT_OUTPUT_PATH} ${LOG_DIR} ${OUTPUT_CONF_DIR} ${SERIALIZED_ROOT_DATA_FORMAT_PATH}
    if [ $? -eq 1 ];
    then
	echo "ERROR : mkdir -p  ${ANALYZED_OUTPUT_PATH} or ${ANALYZED_ROOT_OUTPUT_PATH} or ${ANALYZED_BRIO_OUTPUT_PATH} or ${ANALYZED_STAT_OUTPUT_PATH} or ${LOG_DIR} or ${OUTPUT_CONF_DIR} or ${SERIALIZED_ROOT_DATA_FORMAT_PATH} FAILED  !" 
	exit 0
    fi 

    echo "****** MULTI ANALYZE GENERAL LOG FILE RUN_${i}" > ${LOG_FILE} 2>&1

    echo "ANALYZED_OUTPUT_PATH ${ANALYZED_OUTPUT_PATH}" >> ${LOG_FILE}
    echo "ANALYZED_ROOT_OUTPUT_PATH ${ANALYZED_ROOT_OUTPUT_PATH}" >> ${LOG_FILE}
    echo "ANALYZED_BRIO_OUTPUT_PATH ${ANALYZED_BRIO_OUTPUT_PATH}" >> ${LOG_FILE}
    echo "ANALYZED_STAT_OUTPUT_PATH ${ANALYZED_STAT_OUTPUT_PATH}" >> ${LOG_FILE}
    echo "LOG_DIR ${LOG_DIR}" >> ${LOG_FILE}
    echo "OUTPUT_CONF_DIR ${OUTPUT_CONF_DIR}" >> ${LOG_FILE}
    echo "SERIALIZED_ROOT_DATA_FORMAT_PATH ${SERIALIZED_ROOT_DATA_FORMAT_PATH}" >> ${LOG_FILE}

    LOG_FILE=${LOG_DIR}/hc_data_quality.log
    echo "LOG_FILE ${LOG_FILE}" >> ${LOG_FILE}

    HC_CALO_MAPPING_CONFIG_FILE=${INPUT_CONF_DIR}/mapping_calo.csv
    stat ${HC_CALO_MAPPING_CONFIG_FILE} > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
	echo "WARNING : No ${HC_CALO_MAPPING_CONFIG_FILE} in your directory !"
	echo "WARNING : continue, Run_${i} was not processed into serialization !"
	continue
    fi
    HC_TRACKER_MAPPING_CONFIG_FILE=${INPUT_CONF_DIR}/mapping_tracker.csv
    if [ $? -ne 0 ];
    then
	echo "WARNING : No ${HC_TRACKER_MAPPING_CONFIG_FILE} in your directory !"
	echo "WARNING : continue, Run_${i} was not processed into serialization !"
	continue
    fi
    echo "HC_CALO_MAPPING_CONFIG_FILE    =" ${HC_CALO_MAPPING_CONFIG_FILE} >> ${LOG_FILE}
    echo "HC_TRACKER_MAPPING_CONFIG_FILE =" ${HC_TRACKER_MAPPING_CONFIG_FILE} >> ${LOG_FILE}

    cp ${HC_CALO_MAPPING_CONFIG_FILE} ${OUTPUT_CONF_DIR}/
    if [ $? -eq 1 ];
    then    
	echo "ERROR : cannot copy ${HC_CALO_MAPPING_CONFIG_FILE} into ${OUTPUT_CONF_DIR} FAILED !" >> ${LOG_FILE}
	exit 0
    fi

    cp ${HC_TRACKER_MAPPING_CONFIG_FILE} ${OUTPUT_CONF_DIR}/
    if [ $? -eq 1 ];
    then    
	echo "ERROR : cannot copy ${HC_TRACKER_MAPPING_CONFIG_FILE} into ${OUTPUT_CONF_DIR} FAILED !" >> ${LOG_FILE}
	exit 0
    fi

    echo "LAUNCH JOB TO THE MOON !"
    qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 -e ${LOG_DIR} -o ${LOG_DIR} ${SCRIPT_DIR}/${SINGLE_QUALITY_SCRIPT} -r ${i} -n ${nb_event} -C ${HC_CALO_MAPPING_CONFIG_FILE} -T ${HC_TRACKER_MAPPING_CONFIG_FILE} >> ${LOG_FILE}
    
    echo "Next run..."
done
