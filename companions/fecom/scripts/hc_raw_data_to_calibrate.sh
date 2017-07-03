#!/bin/bash

INPUT_FILE=""
OUTPUT_PATH=""
RUN_NUMBER=0
MAX_NUMBER_OF_EVENTS=0
FIRST_EVENT=0

function usage()
{
    echo "Goal : Chain the programs in fecom library to have the Calibrated Data."
    echo "       Works on a single file, use multi_raw_data_to_calibrate.sh to process a full run !"
    echo "Options : -i   [--input-file]   Enter the input filename "
    echo "          -op  [--output-path]  Enter the output path for files "
    echo "          -r   [--run-number]   Enter the run number "
    echo "          -n   [--event-number] Enter the maximum event number recorded "
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
	-f | --first-event)
            FIRST_EVENT=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

echo "INPUT_FILE is $INPUT_FILE";
echo "OUTPUT_PATH is $OUTPUT_PATH";
echo "RUN_NUMBER is $RUN_NUMBER"
echo "MAX_NUMBER_OF_EVENTS is $MAX_NUMBER_OF_EVENTS";
echo "FIRST_EVENT is $FIRST_EVENT";

SOFTWARES_PATH=~/software/Falaise/Falaise-build/BuildProducts/bin/

DECODE_SORT_SOFTWARE=${SOFTWARES_PATH}/fecom-decode_sort_serialize
EVENT_BUILDER_SOFTWARE=${SOFTWARES_PATH}/fecom-hc_event_builder
CALIBRATE_SOFTWARE=${SOFTWARES_PATH}/fecom-hc_raw_data_to_calibrated_data

SORTED_OUTPUT_FILE=${OUTPUT_PATH}/hc_sorted.data.bz2
BUILDED_OUTPUT_FILE=${OUTPUT_PATH}/hc_builded.brio
CALIBRATE_OUTPUT_FILE=${OUTPUT_PATH}/hc_calibrated_CD.brio

do_cadfaelbrew_setup

MAPPING_DIRECTORY=`dirname ${INPUT_FILE}`

# cp ${MAPPING_DIRECTORY}/mapping_calo.csv ${OUTPUT_PATH}/
# cp ${MAPPING_DIRECTORY}/mapping_tracker.csv ${OUTPUT_PATH}/

calo_mapping=${OUTPUT_PATH}/mapping_calo.csv
tracker_mapping=${OUTPUT_PATH}/mapping_tracker.csv

# Decode software
echo "Begin decode, sort and serialize process..."
echo ""
echo ""
# ${DECODE_SORT_SOFTWARE} -i ${INPUT_FILE} -o ${SORTED_OUTPUT_FILE}

# Minimum number of calo hit in the event :
event_min_calo_hit=0
# Minimum number of tracker hit in the event :
event_min_tracker_hit=0

# # Event builder software
# echo "Begin event builder process..."
# echo ""
# echo ""
# last_event_number=`cat last_event_number.txt`
# let ${last_event_number}+1
# ${EVENT_BUILDER_SOFTWARE} -i ${SORTED_OUTPUT_FILE} -o ${BUILDED_OUTPUT_FILE} -r ${RUN_NUMBER} -cm ${calo_mapping} -tm ${tracker_mapping} -e ${last_event_number}

# Calibrate software
echo "Begin calibration process..."
echo ""
echo ""
${CALIBRATE_SOFTWARE} -i ${BUILDED_OUTPUT_FILE} -o ${CALIBRATE_OUTPUT_FILE} -cm ${calo_mapping} -tm ${tracker_mapping} -C ${event_min_calo_hit} -T ${event_min_tracker_hit} -M ${MAX_NUMBER_OF_EVENTS}

# -e ${FIRST_EVENT}
