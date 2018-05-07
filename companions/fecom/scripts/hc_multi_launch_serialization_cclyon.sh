#!/usr/bin/env bash
# hc_multi_launch_serialization_cclyon.sh

APP_NAME=hc_multi_launch_serialization_cclyon.sh
debug=1

# echo "Are you sure to process ? "
# echo "If yes, edit this file ('hc_multi_launch_serialization_cclyon.sh') and comment 'exit 1' at the begining"
# exit 1

function usage(){
echo "--------------"
echo "Goal : Multi launch serialization CCLyon with qsubs."
echo "Don't forget to adapt the script for your runs, RUN_MIN RUN_MAX and mapping calo & tracker"
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./${APP_NAME} [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-n  [ --number ]   set the number of events"
echo " "
echo "./${APP_NAME} -n number_of_events"
echo "Default value : number_of_events = 10"
echo " "
echo "--------------"
echo "Example : "
echo "./${APP_NAME} -n 100000"
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

SCRIPT_DIR=/sps/nemo/scratch/golivier/software/Falaise/trunk/companions/fecom/scripts/
SINGLE_SERIAL_SCRIPT=hc_launch_serialization_cclyon.sh

# Change Run min / max 
RUN_MIN=3
RUN_MAX=52


for (( i=${RUN_MIN}; i <= ${RUN_MAX}; ++i ))
do
    echo "RUN_NUMBER=" ${i}
    echo "Starting process..."

    INPUT_DIR="/sps/nemo/snemo/half_commissioning/"
    INPUT_RAW_DIR="${INPUT_DIR}/raw/Run_${i}/"
    INPUT_FILES=`ls ${INPUT_RAW_DIR}/*.dat*`
    if [ $? -ne 0 ];
    then
	echo "WARNING : No data files from FEB (Run_*.dat* type)  in your directory !"
	continue
    fi

    HC_CALO_MAPPING_CONFIG_FILE=${INPUT_RAW_DIR}/mapping_calo.csv
    stat ${HC_CALO_MAPPING_CONFIG_FILE} > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
	echo "WARNING : No ${HC_CALO_MAPPING_CONFIG_FILE} in your directory !"
	echo "WARNING : continue, Run_${i} was not processed into serialization !"
	continue
    fi

    HC_TRACKER_MAPPING_CONFIG_FILE=${INPUT_RAW_DIR}/mapping_tracker.csv
    stat ${HC_TRACKER_MAPPING_CONFIG_FILE} > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
	echo "WARNING : No ${HC_TRACKER_MAPPING_CONFIG_FILE} in your directory !"
	echo "WARNING : continue, Run_${i} was not processed into serialization !"
	continue
    fi

    echo "INPUT_DIR ${INPUT_DIR}"
    echo "INPUT_RAW_DIR ${INPUT_RAW_DIR}"

    OUTPUT_PATH=/sps/nemo/scratch/golivier/data_half_commissioning
    SERIALIZED_OUTPUT_PATH=${OUTPUT_PATH}/serialized/
    SERIALIZED_RUN_PATH=${SERIALIZED_OUTPUT_PATH}/Run_${i}

    SERIALIZED_DECODED_PATH=${SERIALIZED_RUN_PATH}/decoded
    SERIALIZED_BUILDED_PATH=${SERIALIZED_RUN_PATH}/builded
    SERIALIZED_SETTING_PATH=${SERIALIZED_RUN_PATH}/settings
    SERIALIZED_LOG_PATH=${SERIALIZED_RUN_PATH}/log_files.d
    LOG_FILE=${SERIALIZED_LOG_PATH}/multi_launch_serialization.log

    mkdir -p ${SERIALIZED_OUTPUT_PATH} ${SERIALIZED_RUN_PATH} ${SERIALIZED_DECODED_PATH} ${SERIALIZED_BUILDED_PATH} ${SERIALIZED_SETTING_PATH} ${SERIALIZED_LOG_PATH}   
    if [ $? -eq 1 ];
    then
	echo "ERROR :mkdir -p ${SERIALIZED_OUTPUT_PATH} ${SERIALIZED_RUN_PATH} ${SERIALIZED_DECODED_PATH} ${SERIALIZED_BUILDED_PATH} ${SERIALIZED_SETTING_PATH} ${SERIALIZED_LOG_PATH} FAILED  !" >> ${LOG_FILE}
	exit 0
    fi 

    echo "****** MULTI SERIALIZE GENERAL LOG FILE RUN_${i}" > ${LOG_FILE} 2>&1   

    echo "INPUT_DIR ${INPUT_DIR}" >> ${LOG_FILE}
    echo "INPUT_RAW_DIR ${INPUT_RAW_DIR}" >> ${LOG_FILE}

    echo "SERIALIZED_OUTPUT_PATH ${SERIALIZED_OUTPUT_PATH}" >> ${LOG_FILE}
    echo "SERIALIZED_RUN_OUTPUT_PATH ${SERIALIZED_RUN_PATH}" >> ${LOG_FILE}
    echo "SERIALIZED_DECODED_OUTPUT_PATH ${SERIALIZED_DECODED_PATH}" >> ${LOG_FILE}
    echo "SERIALIZED_BUILDED_OUTPUT_PATH ${SERIALIZED_BUILDED_PATH}" >> ${LOG_FILE}
    echo "SERIALIZED_SETTING_OUTPUT_PATH ${SERIALIZED_SETTING_PATH}" >> ${LOG_FILE}
    echo "SERIALIZED_LOG_PATH ${SERIALIZED_LOG_PATH}" >> ${LOG_FILE}

    cp ${HC_CALO_MAPPING_CONFIG_FILE}  ${SERIALIZED_SETTING_PATH}/
    if [ $? -eq 1 ];
    then    
	echo "ERROR : cannot copy ${HC_CALO_MAPPING_CONFIG_FILE} into ${SERIALIZED_SETTING_PATH} FAILED !" >> ${LOG_FILE}
	exit
    fi  

    cp ${HC_TRACKER_MAPPING_CONFIG_FILE}  ${SERIALIZED_SETTING_PATH}/
    if [ $? -eq 1 ];
    then    
	echo "ERROR : cannot copy ${HC_TRACKER_MAPPING_CONFIG_FILE} into ${SERIALIZED_SETTING_PATH} FAILED !" >> ${LOG_FILE}
	exit
    fi
    echo "HC_TRACKER_MAPPING_CONFIG_FILE ${HC_TRACKER_MAPPING_CONFIG_FILE}" >> ${LOG_FILE}
    echo "HC_CALO_MAPPING_CONFIG_FILE ${HC_CALO_MAPPING_CONFIG_FILE}" >> ${LOG_FILE}

    echo "QSUB command : qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=12000M,sps=1 -q huge@@C6320IImc -e ${LOG_DIR} -o ${LOG_DIR} ${SCRIPT_DIR}/${SINGLE_SERIAL_SCRIPT} -r ${i} -n ${nb_event} -C ${HC_CALO_MAPPING_CONFIG_FILE} -T ${HC_TRACKER_MAPPING_CONFIG_FILE}" >> ${LOG_FILE}

    qsub -V -P P_nemo -l h_cpu=12:00:00,fsize=12000M,sps=1 -q huge@@C6320IImc -e ${LOG_DIR} -o ${LOG_DIR} ${SCRIPT_DIR}/${SINGLE_SERIAL_SCRIPT} -r ${i} -n ${nb_event} -C ${HC_CALO_MAPPING_CONFIG_FILE} -T ${HC_TRACKER_MAPPING_CONFIG_FILE} >> ${LOG_FILE}
    if [ $? -eq 1 ];
    then    
	echo "ERROR : qsub FAILED !" >> ${LOG_FILE}
	exit
    else
	echo "qsub SUCCESS !" >> ${LOG_FILE}
    fi  
    echo "Next run..."

done