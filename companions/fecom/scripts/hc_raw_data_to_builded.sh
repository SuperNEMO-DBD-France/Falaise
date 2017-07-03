#!/bin/bash

INPUT_FILE=""
OUTPUT_PATH=""
RUN_NUMBER=0
MAX_NUMBER_OF_EVENTS=0

function usage()
{
    echo "Goal : Chain the programs in fecom library to have the Builded Data."
    echo "       Works on a single file, use multi_raw_data_to_calibrate.sh to process a full run !"
    echo "Options : -i   [--input-file]   Enter the input filename "
    echo "          -op  [--output-path]  Enter the output path for files "
    echo "          -r   [--run-number]   Enter the run number "
    echo "          -n   [--event-number] Enter the maximum event number recorded "
    echo "          -f   [--first-event]  Enter the first event number for this file "
}

while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        -i | --input-file)
            INPUT_FILE=$VALUE
            ;;
        -op | --output-path)
            OUTPUT_PATH=$VALUE
            ;;
        -r | --run-number)
            RUN_NUMBER=$VALUE
            ;;
        -n | --event-number)
            MAX_NUMBER_OF_EVENTS=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

FIRST_EVENT=0

echo "INPUT_FILE is $INPUT_FILE";
echo "OUTPUT_PATH is $OUTPUT_PATH";
echo "RUN_NUMBER is $RUN_NUMBER"
echo "MAX_NUMBER_OF_EVENTS is $MAX_NUMBER_OF_EVENTS";
echo "FIRST_EVENT is $FIRST_EVENT";

SW_PATH=~/software/Falaise/build-git/BuildProducts/bin/

DECODE_SORT_SOFTWARE=${SW_PATH}/fecom-decode_sort_serialize
EVENT_BUILDER_SOFTWARE=${SW_PATH}/fecom-hc_event_builder

SORTED_OUTPUT_FILE=${OUTPUT_PATH}/hc_sorted.data.bz2
BUILDED_OUTPUT_FILE=${OUTPUT_PATH}/hc_builded.brio

do_cadfaelbrew_setup

MAPPING_DIRECTORY=`dirname ${INPUT_FILE}`

cp ${MAPPING_DIRECTORY}/mapping_calo.csv ${OUTPUT_PATH}/
cp ${MAPPING_DIRECTORY}/mapping_tracker.csv ${OUTPUT_PATH}/

CALO_MAPPING=${OUTPUT_PATH}/mapping_calo.csv
TRACKER_MAPPING=${OUTPUT_PATH}/mapping_tracker.csv

LAST_EVENT_NUMBER_FILE=${OUTPUT_PATH}/last_event_number.txt
LAST_EVENT_NUMBER=`cat ${LAST_EVENT_NUMBER_FILE} `

echo "Last event number file = ${LAST_EVENT_NUMBER_FILE}"
echo "Last event number = ${LAST_EVENT_NUMBER}"

LOG_DIR=${OUTPUT_PATH}/log_files.d
mkdir -p ${LOG_DIR}

file_basename=`basename ${INPUT_FILE} .dat_*`
LOG_FILE=${LOG_DIR}/${file_basename}.log

echo "File = ${file_basename}"

# Decode software
echo "Begin decode, sort and serialize process..."
echo ""
${DECODE_SORT_SOFTWARE} -i ${INPUT_FILE} -o ${SORTED_OUTPUT_FILE} -M ${MAX_NUMBER_OF_EVENTS} > ${LOG_FILE} 2>&1
# Minimum number of calo hit in the event :
event_min_calo_hit=0
# Minimum number of tracker hit in the event :
event_min_tracker_hit=0

# Event builder software
echo "Begin event builder process..."
echo ""
${EVENT_BUILDER_SOFTWARE} -i ${SORTED_OUTPUT_FILE} -o ${BUILDED_OUTPUT_FILE} -r ${RUN_NUMBER} -cm ${CALO_MAPPING} -tm ${TRACKER_MAPPING} -e ${LAST_EVENT_NUMBER} -M ${MAX_NUMBER_OF_EVENTS} >> ${LOG_FILE} 2>&1

# -e ${FIRST_EVENT}
