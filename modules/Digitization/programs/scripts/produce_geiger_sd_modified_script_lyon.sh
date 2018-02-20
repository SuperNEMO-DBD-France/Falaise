#!/bin/bash
# produce_geiger_sd_modified_script_lyon.sh

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
debug=1
echo "Starting..." >&2

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

OUTPUT_RUN_PATH="${SW_WORK_DIR}/Analysis/${MODIFICATION_MODE}/input_run_${RUN_NUMBER}/"
stat ${OUTPUT_RUN_PATH}
if [ $? -eq 1 ];
then
    echo "INFO : ${OUTPUT_RUN_PATH} does not exist !"
    echo "INFO : mkdir -p ${OUTPUT_RUN_PATH} !"
    mkdir -p ${OUTPUT_RUN_PATH}
    if [ $? -eq 1 ];
    then
	echo "ERROR : mkdir ${OUTPUT_RUN_PATH} FAILED !"
	exit 0
    fi
fi

OUTPUT_PATH="${OUTPUT_RUN_PATH}/run_${UNIQUE_OUTPUT_ID}/SD_brio"
stat ${OUTPUT_PATH}
if [ $? -eq 1 ];
then
    echo "INFO : ${OUTPUT_PATH} does not exist !"
    echo "INFO : mkdir ${OUTPUT_PATH} !"
    mkdir -p ${OUTPUT_PATH}
    if [ $? -eq 1 ];
    then
	echo "ERROR : mkdir ${OUTPUT_PATH} FAILED !"
	exit 0
    fi
fi

DATABASE_FILE="${SW_WORK_DIR}/Analysis/database.db"
stat $DATABASE_FILE
if  [ $? -eq 1 ];
then
    echo "INFO :  ${DATABASE_FILE} does not exist !"
    echo "INFO : touch ${DATABASE_FILE} !"
    touch ${DATABASE_FILE}
    if [ $? -eq 1 ];
    then
	echo "ERROR : touch ${DATABASE} FAILED !"
	exit 0
    fi
fi

# Fill database :
echo "${START_DATE} input_run_${RUN_NUMBER} ${NUMBER_OF_INPUT_FILES} ${MODIFICATION_MODE} ${UNIQUE_OUTPUT_ID} ${TRIGGER_PROBABILITY}" >> ${DATABASE_FILE}

LIST_OF_INPUT_FILES=UNDEFINED
NUMBER_OF_EVENTS=UNDEFINED

LIST_OF_INPUT_FILES=`ls ${INPUT_DIR}/output_files.d/*.brio | grep ".brio" -m ${NUMBER_OF_INPUT_FILES}`
echo "[INFO] : list of files : $LIST_OF_INPUT_FILES"
NUMBER_OF_EVENTS=`echo "250000*${NUMBER_OF_INPUT_FILES}" | bc -l`
echo "[INFO] : nb of event : $NUMBER_OF_EVENTS"

WORKER_OUTPUT_DIR=${TMPDIR}/

echo "INPUT_DIR=" ${INPUT_DIR}
echo "LIST_OF_INPUT_FILES=" ${LIST_OF_INPUT_FILES}
echo "NUMBER_OF_EVENTS=" ${NUMBER_OF_EVENTS}
echo "SW=" ${SW_PATH}/${SW_NAME}
echo "OUTPUT_PATH=" ${OUTPUT_PATH}
echo "WORKER_OUTPUT_DIR=" ${WORKER_OUTPUT_DIR}


# echo "***********"
# echo "WORKER ARCHITECTURE LOG"
# echo "***********"
# uname -a
# getconf LONG_BIT
# grep flags /proc/cpuinfo
# echo ""


# echo "***********"
# echo "LDD" 
# echo "***********"
# ldd ${SW_PATH}/${SW_NAME}
# echo ""
# echo ""

echo "[CMD] : ${SW_PATH}/${SW_NAME} -i ${LIST_OF_INPUT_FILES} -o ${WORKER_OUTPUT_DIR} -n ${NUMBER_OF_EVENTS} -m ${MODIFICATION_MODE} -p ${TRIGGER_PROBABILITY}"

${SW_PATH}/${SW_NAME} -i ${LIST_OF_INPUT_FILES} -o ${WORKER_OUTPUT_DIR} -n ${NUMBER_OF_EVENTS} -m ${MODIFICATION_MODE} -p ${TRIGGER_PROBABILITY}


WORKER_LIST_OF_FILES=`ls ${WORKER_OUTPUT_DIR}/*.brio`

# Copy file from worker to /sps/ 
for file in ${WORKER_LIST_OF_FILES}
do
    echo "File in dir ${WORKER_OUTPUT_DIR}   : " ${file}
    scp ${file} ${OUTPUT_PATH}
    if  [ $? -eq 1 ];
    then
	echo "ERROR : scp ${file} ${OUTPUT_PATH} failed !"
    fi 
done



