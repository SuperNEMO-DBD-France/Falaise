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
echo " "
echo "./self_trigger_script"
echo " "
echo "--------------"
echo "Example : "
echo "./self_trigger_script -r 0"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
run_number=UNDEFINED

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
    shift 2
done

if [ ${run_number} = "UNDEFINED" ];
then
    echo "ERROR : ${run_number} is not defined : FAILED and EXIT !"
    exit 1
fi

SW_PATH="${SW_WORK_DIR}/Falaise/build-git/BuildProducts/bin/"

##### Self Trigger (ST) part #####
ST_SW_NAME="falaisedigitizationplugin-produce_self_trigger_hits"

ST_OUTPUT_PATH="${SW_WORK_DIR}/analysis/self_trigger/run_${run_number}/"
mkdir -p ${ST_OUTPUT_PATH}
if [ $? -eq 1 ];
then
    echo "ERROR : mkdir ${ST_OUTPUT_PATH} FAILED !"
    exit 0
fi

ST_BASE_CONFIG_FILE="${FALAISE_DIGITIZATION_DIR}/resources/self_trigger.conf"
ST_RUN_CONFIG_FILE="${SW_WORK_DIR}/analysis/self_trigger/run_${run_number}/self_trigger.conf"
cp ${ST_BASE_CONFIG_FILE} ${ST_RUN_CONFIG_FILE}
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${ST_BASE_CONFIG_FILE} into ${ST_RUN_CONFIG_FILE} FAILED !"
    exit 0
fi

# Search and replace ST trigger configuration file for a given run
st_time_interval=0.01 # s
st_calo_freq=1000 # Hz
st_tracker_freq=10000 # Hz

`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"time_interval : real as time = 1 s"/"time_interval : real as time = ${st_time_interval} s"/g ${ST_RUN_CONFIG_FILE}`
`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"calo.self_trigger_frequency : real as frequency = 10 Hz"/"calo.self_trigger_frequency : real as frequency = ${st_calo_freq} Hz"/g ${ST_RUN_CONFIG_FILE}`
`cat ${ST_RUN_CONFIG_FILE} | sed -i s/"geiger.self_trigger_frequency : real as frequency = 5 Hz"/"geiger.self_trigger_frequency : real as frequency = ${st_tracker_freq} Hz"/g ${ST_RUN_CONFIG_FILE}`

${SW_PATH}/${ST_SW_NAME} -c ${ST_RUN_CONFIG_FILE} -o ${ST_OUTPUT_PATH}
ST_OUTPUT_FILE="${ST_OUTPUT_PATH}/self_trigger_hits.data.bz2"


##### Trigger part #####

TRIGGER_SW_NAME="falaisedigitizationplugin-trigger_program_on_signals"
TRIGGER_CONFIG="${FALAISE_DIGITIZATION_DIR}/resources/config/snemo/common/1.0/trigger_parameters.conf"
cp ${TRIGGER_CONFIG} ${ST_OUTPUT_PATH}/
if [ $? -eq 1 ];
then
    echo "ERROR : copy ${TRIGGER_CONFIG} into ${ST_OUTPUT_PATH/} FAILED !"
    exit 0
fi

${SW_PATH}/${TRIGGER_SW_NAME} -i ${ST_OUTPUT_FILE} -c ${TRIGGER_CONFIG} -o ${ST_OUTPUT_PATH}
