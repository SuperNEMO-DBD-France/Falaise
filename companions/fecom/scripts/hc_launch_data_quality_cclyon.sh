#!/usr/bin/env bash
# hc_launch_data_quality_cclyon.sh

APP_NAME=hc_launch_data_quality_cclyon.sh
debug=1

echo "Starting..." >&2

SW_PATH="/sps/nemo/scratch/golivier/software/Falaise/build/BuildProducts/bin/"
SW_NAME="fecom-hc_data_quality"

function usage(){
echo "--------------"
echo "Goal : Analyze half commissioning raw data at cclyon"
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./hc_launch_data_quality_cclyon.sh [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-n  [ --number ]   set the number of events"
echo "-r  [--run-number] set the run number to analyze"
echo "-C  [--calo-mapping] set the calo mapping rules for sorting"
echo "-T  [--tracker-mapping] set the tracker mapping rules for sorting"
echo " "
echo "./hc_analyze_data_cclyon.sh -r 0 -n number_of_events -C CALO_FILE -T TRACKER_FILE"
echo "Default value : number_of_events = 10"
echo " "
echo "--------------"
echo "Example : "
echo "./hc_analyze_data_cclyon.sh -n 100000 -r 0 -C CALO_FILE -T TRACKER_FILE"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`
nb_event=10
run_number=UNDEFINED
HC_CALO_MAPPING_CONFIG_FILE=UNDEFINED
HC_TRACKER_MAPPING_CONFIG_FILE=UNDEFINED

while [ -n "$1" ];
do
    arg="$1"
    arg_value="$2"
    if [ "x$arg" = "x-h" -o "x$arg" = "x--help" ]; then
        usage
        exit 1
    fi
    if [ "x$arg" = "x-n" ]; then
        nb_event=$arg_value
    fi
    if [ "x$arg" = "x-r" ]; then
	run_number=$arg_value
    fi
    if [ "x$arg" = "x-C" ]; then
	HC_CALO_MAPPING_CONFIG_FILE=$arg_value
    fi
    if [ "x$arg" = "x-T" ]; then
	HC_TRACKER_MAPPING_CONFIG_FILE=$arg_value
    fi
    shift 2
done

echo "RUN_NUMBER=" ${run_number}
echo "HC_CALO_MAPPING_CONFIG_FILE=" ${HC_CALO_MAPPING_CONFIG_FILE}
echo "HC_TRACKER_MAPPING_CONFIG_FILE=" ${HC_TRACKER_MAPPING_CONFIG_FILE}

echo "Starting process..."
echo "Processing..."

INPUT_DIR="/sps/nemo/scratch/golivier/data_half_commissioning/"
INPUT_SERIALIZED_DIR="${INPUT_DIR}/serialized/Run_${run_number}/builded/"
INPUT_FILES=`ls ${INPUT_SERIALIZED_DIR}/*.brio`

ANALYZED_OUTPUT_PATH="/sps/nemo/scratch/golivier/data_half_commissioning/analyzed/Run_${run_number}/"
ANALYZED_ROOT_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/root_files
ANALYZED_BRIO_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/brio_files
ANALYZED_STAT_OUTPUT_PATH=${ANALYZED_OUTPUT_PATH}/stat_files
LOG_DIR=${ANALYZED_OUTPUT_PATH}/log_files.d
SERIALIZED_ROOT_DATA_FORMAT_PATH="/sps/nemo/scratch/golivier/data_half_commissioning/root/Run_${run_number}/"

echo "ANALYZED_OUTPUT_PATH ${ANALYZED_OUTPUT_PATH}"
echo "ANALYZED_ROOT_OUTPUT_PATH ${ANALYZED_ROOT_OUTPUT_PATH}"
echo "ANALYZED_BRIO_OUTPUT_PATH ${ANALYZED_BRIO_OUTPUT_PATH}"
echo "ANALYZED_STAT_OUTPUT_PATH ${ANALYZED_STAT_OUTPUT_PATH}"
echo "LOG_DIR ${LOG_DIR}"
echo "SERIALIZED_ROOT_DATA_FORMAT_PATH ${SERIALIZED_ROOT_DATA_FORMAT_PATH}"

LOG_FILE=${LOG_DIR}/hc_data_quality.log
echo "LOG_FILE ${LOG_FILE}"

${SW_PATH}/${SW_NAME} -i ${INPUT_FILES} -o ${ANALYZED_OUTPUT_PATH} -M ${nb_event} -c ${HC_CALO_MAPPING_CONFIG_FILE} -t ${HC_TRACKER_MAPPING_CONFIG_FILE} > ${LOG_FILE} 2>&1
if [ $? -eq 1 ];
then
    echo "ERROR : command ${SW_PATH}/${SW_NAME} -i ${INPUT_FILES} -o ${ANALYZED_OUTPUT_PATH} -M ${nb_event} -c ${HC_CALO_MAPPING_CONFIG_FILE} -t ${HC_TRACKER_MAPPING_CONFIG_FILE} > ${LOG_FILE} 2>&1 FAILED !" >> ${LOG_FILE}
    echo "FILE_ANALYZING:FAILED" >> ${LOG_FILE}
    exit 1
fi
OUTPUT_CALO_TRACKER_FILENAME=output_calo_tracker_events.brio
OUTPUT_STAT_FILENAME=output_data.stat
OUTPUT_HISTO_FILENAME=hc_quality_histograms.root
OUTPUT_DATA_ROOT_FILENAME=output_rootfile.root

mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_CALO_TRACKER_FILENAME} ${ANALYZED_BRIO_OUTPUT_PATH}/
if [ $? -eq 1 ];
then
    echo "ERROR : mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_CALO_TRACKER_FILENAME} ${ANALYZED_BRIO_OUTPUT_PATH}/ FAILED !"
    exit 1
fi
mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_STAT_FILENAME} ${ANALYZED_STAT_OUTPUT_PATH}/
if [ $? -eq 1 ];
then
    echo "ERROR : mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_STAT_FILENAME} ${ANALYZED_STAT_OUTPUT_PATH}/ FAILED !"
    exit 1
fi
mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_HISTO_FILENAME} ${ANALYZED_ROOT_OUTPUT_PATH}/
if [ $? -eq 1 ];
then
    echo "ERROR : mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_HISTO_FILENAME} ${ANALYZED_ROOT_OUTPUT_PATH} FAILED !"
    exit 1
fi
mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_DATA_ROOT_FILENAME} ${SERIALIZED_ROOT_DATA_FORMAT_PATH}
if [ $? -eq 1 ];
then
    echo "ERROR : mv ${ANALYZED_OUTPUT_PATH}/${OUTPUT_DATA_ROOT_FILENAME} ${SERIALIZED_ROOT_DATA_FORMAT_PATH} FAILED !"
    exit 1
fi

echo "FILE_ANALYZING:SUCCESS" >> ${LOG_FILE}

cp ${LOG_FILE} ${SERIALIZED_ROOT_DATA_FORMAT_PATH}/

echo "Ending process..."