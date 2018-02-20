#!/usr/bin/env bash
# trigger_sd_modified_lyon.sh

debug=1

echo "Starting..." >&2


function usage(){
echo "--------------"
echo "Goal : Produce self trigger hits and pack them into events @ 1600 ns."
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./trigger_sd_modified_lyon [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-m  [--mode] the SD production mode you want to pass through the trigger system "
echo "-s  [--sd-run-number] set the input raw SD run number"
echo "-r  [--run-number] set the run number SD modified"
echo " "
echo "./trigger_sd_modified_lyon"
echo " "
echo "--------------"
echo "Example : "
echo "./trigger_sd_modified_lyon -m same_cell_trigger -s 0-r 1"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
MODE=UNDEFINED # Modes are : "same_cell_trigger", "neighbourg_trigger", "trigger_efficiency"
INPUT_SD_RUN_NUMBER=UNDEFINED
INPUT_RUN_NUMBER=UNDEFINED

while [ -n "$1" ];
do
    arg="$1"
    arg_value="$2"
    if [ "x$arg" = "x-h" -o "x$arg" = "x--help" ]; then
        usage
        exit 0
    fi
    if [ "x$arg" = "x-m" ]; then
	MODE=$arg_value
    fi    
    if [ "x$arg" = "x-s" ]; then
	INPUT_SD_RUN_NUMBER=$arg_value
    fi    
    if [ "x$arg" = "x-r" ]; then
	INPUT_RUN_NUMBER=$arg_value
    fi    
    shift 2
done
if [ ${MODE} = "UNDEFINED" ];
then
    echo "ERROR : ${MODE} is not defined : FAILED and EXIT !"
    exit 1
fi
if [ ${INPUT_SD_RUN_NUMBER} = "UNDEFINED" ];
then
    echo "ERROR : ${INPUT_SD_RUN_NUMBER} is not defined : FAILED and EXIT !"
    exit 1
fi
echo "MODE=               " ${MODE}
echo "INPUT_SD_RUN_NUMBER=" ${INPUT_SD_RUN_NUMBER}

# Construct input paths and list of input SD
ANALYSIS_PATH=/sps/nemo/scratch/golivier/software/Analysis/
INPUT_RUN_PATH=""
LIST_OF_SD_FILES=""
OUTPUT_PATH=""

if [ "$MODE" == "trigger_efficiency" ];
then
    INPUT_RUN_PATH=/sps/nemo/scratch/golivier/SD_data_MCC_1/run_${INPUT_SD_RUN_NUMBER}/output_files.d/
    LIST_OF_SD_FILES=`ls ${INPUT_RUN_PATH}/*.brio`
    OUTPUT_PATH=${ANALYSIS_PATH}/${MODE}/input_run_${INPUT_SD_RUN_NUMBER}/run_0/trigger_output/
else
    echo "INPUT_RUN_NUMBER=   " ${INPUT_RUN_NUMBER}
    if [ ${INPUT_RUN_NUMBER} = "UNDEFINED" ];
    then
	echo "ERROR : ${INPUT_RUN_NUMBER} is not defined : FAILED and EXIT !"
	exit 1
    fi
    INPUT_RUN_PATH=${ANALYSIS_PATH}/${MODE}/input_run_${INPUT_SD_RUN_NUMBER}/run_${INPUT_RUN_NUMBER}/
    INPUT_RUN_SD_PATH=${INPUT_RUN_PATH}/SD_brio/
    LIST_OF_SD_FILES=`ls ${INPUT_RUN_SD_PATH}/*.brio`
    OUTPUT_PATH="${INPUT_RUN_PATH}/trigger_output/" 
fi

echo "INPUT_RUN_PATH=   " ${INPUT_RUN_PATH}
echo "OUTPUT_PATH=      " ${OUTPUT_PATH}
mkdir -p ${OUTPUT_PATH}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${OUTPUT_PATH} FAILED !"
    exit 0
fi

WORKER_OUTPUT_DIR=${TMPDIR}/
TRIGGER_CONFIG_FILE="${FALAISE_DIGITIZATION_DIR}/resources/config/snemo/common/1.0/trigger_parameters.conf"
scp ${TRIGGER_CONFIG_FILE} ${TMPDIR}/
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${TRIGGER_CONFIG_FILE} into ${TMPDIR} FAILED !"
    exit 0
fi

NUMBER_OF_SD_FILES=`ls ${LIST_OF_SD_FILES} | wc -l`
NUMBER_OF_EVENTS=`echo "250000*${NUMBER_OF_SD_FILES}" | bc -l`

SW_PATH="${SW_WORK_DIR}/Falaise/build/BuildProducts/bin/"
SW_NAME="falaisedigitizationplugin-trigger_program"


echo "SW_NAME=            " ${SW_PATH}/${SW_NAME}
echo "LIST_OF_SD_FILES=   " ${LIST_OF_SD_FILES}
echo "NUMBER_OF_SD_FILES= " ${NUMBER_OF_SD_FILES}
echo "NUMBER_OF_EVENTS=   " ${NUMBER_OF_EVENTS}
echo "TRIGGER_CONFIG_FILE=" ${TRIGGER_CONFIG_FILE}
echo "OUTPUT_PATH=        " ${OUTPUT_PATH}
echo "WORKER_OUTPUT_DIR=  " ${WORKER_OUTPUT_DIR}

${SW_PATH}/${SW_NAME} -i ${LIST_OF_SD_FILES} -c ${TRIGGER_CONFIG_FILE} -o ${WORKER_OUTPUT_DIR} -n ${NUMBER_OF_EVENTS}

echo "Begin copy from worker to /sps/"
echo "OUTPUT_PATH=        " ${OUTPUT_PATH}

scp -r ${WORKER_OUTPUT_DIR}/* ${OUTPUT_PATH}
if  [ $? -eq 1 ];
then
    echo "ERROR : scp ${WORKER_OUTPUT_DIR} ${OUTPUT_PATH} failed !"
fi 