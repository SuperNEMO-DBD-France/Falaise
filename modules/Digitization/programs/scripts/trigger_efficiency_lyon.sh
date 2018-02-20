#!/usr/bin/env bash
# trigger_efficiency_lyon.sh

debug=1

echo "Starting..." >&2


function usage(){
echo "--------------"
echo "Goal : Produce self trigger hits and pack them into events @ 1600 ns."
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./trigger_efficiency_lyon [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-r  [--run-number] set the run number"
echo " "
echo "./trigger_efficiency_lyon"
echo " "
echo "--------------"
echo "Example : "
echo "./trigger_efficiency -r 0"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
run_number=UNDEFINED
INPUT_FILE=UNDEFINED
NUMBER_OF_EVENTS=UNDEFINED
while [ -n "$1" ];
do
    arg="$1"
    arg_value="$2"
    if [ "x$arg" = "x-h" -o "x$arg" = "x--help" ]; then
        usage
        exit 0
    fi
    if [ "x$arg" = "x-r" ]; then
	run_number=$arg_value
    fi    
    if [ "x$arg" = "x-n" ]; then
	NUMBER_OF_EVENTS=$arg_value
    fi    
    if [ "x$arg" = "x-i" ]; then
	INPUT_FILE=$arg_value
    fi
    shift 2
done

if [ ${run_number} = "UNDEFINED" ];
then
    echo "ERROR : ${run_number} is not defined : FAILED and EXIT !"
    exit 1
fi
echo "RUN_NUMBER=" $run_number
echo "INPUT_FILE=" ${INPUT_FILE}
echo "NUMBER_OF_EVENTS=" ${NUMBER_OF_EVENTS}

SW_PATH="${SW_WORK_DIR}/Falaise/build/BuildProducts/bin/"
SW_NAME="falaisedigitizationplugin-trigger_program"

OUTPUT_PATH="${SW_WORK_DIR}/Analysis/trigger_efficiency/run_${run_number}/"
mkdir -p ${OUTPUT_PATH}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${ST_OUTPUT_PATH} FAILED !"
    exit 0
fi

TRIGGER_CONFIG_FILE="${FALAISE_DIGITIZATION_DIR}/resources/config/snemo/common/1.0/trigger_parameters.conf"
cp ${TRIGGER_CONFIG_FILE} ${OUTPUT_PATH}/
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${TRIGGER_CONFIG_FILE} into ${OUTPUT_PATH} FAILED !"
    exit 0
fi
echo "SW name : ${SW_PATH}/${SW_NAME}"
echo "File name : ${INPUT_FILE} "
echo "Trigger config file name ${TRIGGER_CONFIG_FILE}"
echo "Output path : ${OUTPUT_PATH} "

${SW_PATH}/${SW_NAME} -i ${INPUT_FILE} -c ${TRIGGER_CONFIG_FILE} -o ${OUTPUT_PATH} -n ${NUMBER_OF_EVENTS}
