#!/usr/bin/env bash
# self_trigger_script.sh

debug=1

echo "Starting..." >&2


function usage(){
echo "--------------"
echo "Goal : Produce self trigger hits and pack them into events @ 1600 ns."
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./self_trigger_script [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-r  [--run-number] set the run number"
echo "-s  [--st_time_interval] set the self trigger time interval [seconds]"
echo "-c  [--st_calo_freq] set the self trigger calo frequency [Hertz]"
echo "-t  [--st_tracker_freq] set the self trigger tracker frequency [Hertz]"
echo " "
echo "./self_trigger_script"
echo " "
echo "--------------"
echo "Example : "
echo "./self_trigger_script -r 0 -s 1 -c 10 -t 0.1"
echo "In this example, time interval is 1 second, calo frequency = 10Hz per OM and tracker frequency = 0.1Hz per cell"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
run_number=UNDEFINED
st_time_interval=UNDEFINED # s
st_calo_freq=UNDEFINED # Hz
st_tracker_freq=UNDEFINED # Hz

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
    if [ "x$arg" = "x-s" ]; then
	st_time_interval=$arg_value
    fi
    if [ "x$arg" = "x-c" ]; then
	st_calo_freq=$arg_value
    fi
    if [ "x$arg" = "x-t" ]; then
	st_tracker_freq=$arg_value
    fi
    shift 2
done

if [ ${run_number} = "UNDEFINED" ];
then
    echo "ERROR : ${run_number} is not defined : FAILED and EXIT !"
    exit 1
fi
echo "RUN_NUMBER=" $run_number
echo "TIME INTERVAL=" ${st_time_interval}
echo "CALO FREQ=" ${st_calo_freq}
echo "TRACKER FREQ=" ${st_tracker_freq}


SW_PATH="${SW_WORK_DIR}/Falaise/build/BuildProducts/bin/"
##### Self Trigger (ST) part #####
ST_SW_NAME="falaisedigitizationplugin-produce_self_trigger_hits"

WORKER_OUTPUT_DIR=${TMPDIR}/

ST_BASE_CONFIG_FILE="${FALAISE_DIGITIZATION_DIR}/resources/self_trigger.conf"
ST_RUN_CONFIG_FILE="${WORKER_OUTPUT_DIR}/self_trigger.conf"
cp ${ST_BASE_CONFIG_FILE} ${ST_RUN_CONFIG_FILE}
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${ST_BASE_CONFIG_FILE} into ${ST_RUN_CONFIG_FILE} FAILED !"
    exit 0
fi

`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"time_interval : real as time = 1 s"/"time_interval : real as time = ${st_time_interval} s"/g ${ST_RUN_CONFIG_FILE}`
`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"calo.self_trigger_frequency : real as frequency = 10 Hz"/"calo.self_trigger_frequency : real as frequency = ${st_calo_freq} Hz"/g ${ST_RUN_CONFIG_FILE}`
`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"geiger.self_trigger_frequency : real as frequency = 2 Hz"/"geiger.self_trigger_frequency : real as frequency = ${st_tracker_freq} Hz"/g ${ST_RUN_CONFIG_FILE}`

echo "SW name : ${ST_SW_PATH}/${ST_SW_NAME}"
echo "ST Config name : ${ST_RUN_CONFIG_FILE} "


${SW_PATH}/${ST_SW_NAME} -c ${ST_RUN_CONFIG_FILE} -o ${WORKER_OUTPUT_DIR}
ST_OUTPUT_FILE="${WORKER_OUTPUT_DIR}/self_trigger_hits.data.bz2"

##### Trigger part #####

TRIGGER_SW_NAME="falaisedigitizationplugin-trigger_program_on_signals"
TRIGGER_CONFIG="${FALAISE_DIGITIZATION_DIR}/resources/config/snemo/common/1.0/trigger_parameters.conf"
cp ${TRIGGER_CONFIG} ${WORKER_OUTPUT_DIR}/
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${TRIGGER_CONFIG} into ${WORKER_OUTPUT_DIR/} FAILED !"
    exit 0
fi

${SW_PATH}/${TRIGGER_SW_NAME} -i ${ST_OUTPUT_FILE} -c ${TRIGGER_CONFIG} -o ${WORKER_OUTPUT_DIR}

OUTPUT_PATH="${SW_WORK_DIR}/Analysis/self_trigger/run_${run_number}/"
OUTPUT_TRIGGER_DIR=${OUTPUT_PATH}/trigger_output
OUTPUT_DATA_DIR=${OUTPUT_PATH}/signals_brio
mkdir -p ${OUTPUT_PATH} ${OUTPUT_TRIGGER_DIR} ${OUTPUT_DATA_DIR}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${ST_OUTPUT_PATH} or ${OUTPUT_TRIGGER_DIR} or ${OUTPUT_DATA_DIR} FAILED !"
    exit 0
fi

echo "Begin copy from worker to /sps/"
echo "OUTPUT_PATH=        " ${OUTPUT_PATH}
echo "OUTPUT_TRIGGER_DIR= " ${OUTPUT_TRIGGER_DIR}
echo "OUTPUT_DATA_DIR=    " ${OUTPUT_DATA_DIR}
LIST_OF_OUTPUT_FILES=`ls ${WORKER_OUTPUT_DIR}/*`
echo "List of output files on worker :"
echo "${LIST_OF_OUTPUT_FILES}"

scp -r ${WORKER_OUTPUT_DIR}/output_trigger.* ${OUTPUT_TRIGGER_DIR}
if  [ $? -eq 1 ];
then
    echo "ERROR : scp ${WORKER_OUTPUT_DIR}/output_trigger* ${OUTPUT_TRIGGER_DIR} failed !"
fi 
scp -r ${WORKER_OUTPUT_DIR}/trigger_parameters.conf ${OUTPUT_TRIGGER_DIR}
if  [ $? -eq 1 ];
then
    echo "ERROR : scp ${WORKER_OUTPUT_DIR}/trigger_parameters.conf ${OUTPUT_TRIGGER_DIR} failed !"
fi 

scp -r ${WORKER_OUTPUT_DIR}/*self* ${OUTPUT_DATA_DIR}
if  [ $? -eq 1 ];
then
    echo "ERROR : scp ${WORKER_OUTPUT_DIR}/*self* ${OUTPUT_DATA_DIR} failed !"
fi 
# scp -r ${WORKER_OUTPUT_DIR}/anode_timestamp.dat ${OUTPUT_DATA_DIR}
# if  [ $? -eq 1 ];
# then
#     echo "ERROR : scp ${WORKER_OUTPUT_DIR}/anode_timestamp.dat ${OUTPUT_DATA_DIR} failed !"
# fi 