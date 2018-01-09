#!/usr/bin/env bash
# produce_geiger_sd_modified_script_lyon.sh

debug=1

echo "Starting..." >&2

function usage(){
echo "--------------"
echo "Goal : Produce geiger SD modified to test trigger robustness"
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./produce_geiger_sd_modified_script_lyon [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-r  [--run-number] set the run number"
echo " "
echo "./produce_geiger_sd_modified_script_lyon"
echo " "
echo "--------------"
echo "Example : "
echo "./produce_geiger_sd_modified_script_lyon -r 0 -n 1"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
RUN_NUMBER=UNDEFINED
NUMBER_OF_INPUT_FILES=UNDEFINED
UNIQUE_OUTPUT_ID=UNDEFINED
MODIFICATION_MODE=UNDEFINED
# "same_cell_trigger" or "neighbourg_trigger"
TRIGGER_PROBABILITY=UNDEFINED

while [ -n "$1" ];
do
    arg="$1"
    arg_value="$2"
    if [ "x$arg" = "x-h" -o "x$arg" = "x--help" ]; then
        usage
        exit 0
    fi
    if [ "x$arg" = "x-r" ]; then
	RUN_NUMBER=$arg_value
    fi    
    if [ "x$arg" = "x-n" ]; then
	NUMBER_OF_INPUT_FILES=$arg_value
    fi    
    if [ "x$arg" = "x-u" ]; then
	UNIQUE_OUTPUT_ID=$arg_value
    fi    
    if [ "x$arg" = "x-m" ]; then
	MODIFICATION_MODE=$arg_value
    fi    
    if [ "x$arg" = "x-p" ]; then
	TRIGGER_PROBABILITY=$arg_value
    fi    
    shift 2
done

if [ ${RUN_NUMBER} = "UNDEFINED" ];
then
    echo "ERROR : ${RUN_NUMBER} is not defined : FAILED and EXIT !"
    exit 1
fi
echo "RUN_NUMBER=" ${RUN_NUMBER}
echo "NUMBER_OF_INPUT_FILES=" ${NUMBER_OF_INPUT_FILES}
echo "UNIQUE_OUTPUT_ID=" ${UNIQUE_OUTPUT_ID}
echo "MODIFICATION_MODE=" ${MODIFICATION_MODE}
echo "TRIGGER_PROBABILITY=" ${TRIGGER_PROBABILITY}

SW_PATH="${SW_WORK_DIR}/Falaise/build/BuildProducts/bin/"

##### Produce SD modified part #####
SW_NAME="falaisedigitizationplugin-produce_geiger_sd_modified"

INPUT_PATH="/sps/nemo/scratch/golivier/SD_data_MCC_1/"
INPUT_DIR="${INPUT_PATH}/run_${RUN_NUMBER}/"

OUTPUT_PATH="${SW_WORK_DIR}/Analysis/${MODIFICATION_MODE}/run_${UNIQUE_OUTPUT_ID}/"
mkdir -p ${OUTPUT_PATH}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${OUTPUT_PATH} FAILED !"
    exit 0
fi

DATABASE_FILE="${SW_WORK_DIR}/Analysis/database.db"

# Fill database :
echo "${START_DATE} run_${RUN_NUMBER} ${NUMBER_OF_INPUT_FILES} ${MODIFICATION_MODE} ${UNIQUE_OUTPUT_ID} ${TRIGGER_PROBABILITY}" >> ${DATABASE_FILE}

LIST_OF_INPUT_FILES=UNDEFINED
NUMBER_OF_EVENTS=UNDEFINED

if [ ${NUMBER_OF_INPUT_FILES} -eq 1 ];
then
    LIST_OF_INPUT_FILES=`ls ${INPUT_DIR}/output_files.d/file_0.brio`
    NUMBER_OF_EVENTS=250000
elif [ ${NUMBER_OF_INPUT_FILES} -eq 2 ];
then 
    LIST_OF_INPUT_FILES=`ls ${INPUT_DIR}/output_files.d/file_0.brio ${INPUT_DIR}/output_files.d/file_1.brio`
    NUMBER_OF_EVENTS=500000
elif [ ${NUMBER_OF_INPUT_FILES} -eq 3 ];
then
    LIST_OF_INPUT_FILES=`ls ${INPUT_DIR}/output_files.d/file_0.brio ${INPUT_DIR}/output_files.d/file_1.brio ${INPUT_DIR}/output_files.d/file_2.brio` 
    NUMBER_OF_EVENTS=750000
elif [ ${NUMBER_OF_INPUT_FILES} -eq 4 ];
then
    LIST_OF_INPUT_FILES=`ls ${INPUT_DIR}/output_files.d/file_0.brio ${INPUT_DIR}/output_files.d/file_1.brio ${INPUT_DIR}/output_files.d/file_2.brio ${INPUT_DIR}/output_files.d/file_3.brio`
    NUMBER_OF_EVENTS=1000000
else
    echo "ERROR : Invalid file number, abort script !"
    exit 0
fi

echo "INPUT_DIR=" ${INPUT_DIR}
echo "LIST_OF_INPUT_FILES=" ${LIST_OF_INPUT_FILES}
echo "NUMBER_OF_EVENTS=" ${NUMBER_OF_EVENTS}
echo "SW=" ${SW_PATH}/${SW_NAME}
echo "OUTPUT_PATH=" ${OUTPUT_PATH}

OUTPUT_FILENAME="modified_SD.brio"
OUTPUT_FILE=${OUTPUT_PATH}/${OUTPUT_FILENAME}

echo "${SW_PATH}/${SW_NAME} -i ${LIST_OF_INPUT_FILES} -o ${OUTPUT_PATH} -n ${NUMBER_OF_EVENTS} -m ${MODIFICATION_MODE} -p ${TRIGGER_PROBABILITY}"
echo ""

echo "***********"
echo "WORKER ARCHITECTURE LOG"
echo "***********"
uname -a
getconf LONG_BIT
grep flags /proc/cpuinfo
echo ""


echo "***********"
echo "LDD" 
echo "***********"
ldd ${SW_PATH}/${SW_NAME}
echo ""
echo ""


${SW_PATH}/${SW_NAME} -i ${LIST_OF_INPUT_FILES} -o ${OUTPUT_PATH} -n ${NUMBER_OF_EVENTS} -m ${MODIFICATION_MODE} -p ${TRIGGER_PROBABILITY}

