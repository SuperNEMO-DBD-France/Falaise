#!/bin/bash

RUN_NUMBER=0
MAX_NUMBER_OF_EVENTS=0

# INPUT AND OUTPUT PATH to adapt :
INPUT_DATA_PATH="/home/sheatz/perso/commissioning_data/"
FECOM_SCRIPT_DIR="/home/sheatz/software/Falaise/trunk-git/companions/fecom/scripts"

function usage()
{
    echo "Goal : Process all files contains in the Run_X/ path."
    echo "       Launch raw_data_to_calibrate.sh script for each file of the run"
    echo "Options : -r   [--run-number]   Enter the run number "
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
        -n | --event-number)
            MAX_NUMBER_OF_EVENTS=$VALUE
            ;;
        -r | --run-number)
            RUN_NUMBER=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

INPUT_PATH=${INPUT_DATA_PATH}/raw/Run_${RUN_NUMBER}
OUTPUT_PATH=${INPUT_DATA_PATH}/serialized/Run_${RUN_NUMBER}

mkdir -p  ${OUTPUT_PATH}

echo "INPUT_PATH is $INPUT_PATH";
echo "OUTPUT_PATH is $OUTPUT_PATH";
echo "RUN_NUMBER is $RUN_NUMBER";
echo "MAX_NUMBER_OF_EVENTS is $MAX_NUMBER_OF_EVENTS";

FECOM_SINGLE_LAUNCH=${FECOM_SCRIPT_DIR}/hc_raw_data_to_builded.sh

list_of_input_files=`ls ${INPUT_PATH}/Run_${RUN_NUMBER}.dat*`

LAST_EVENT_NUMBER_FILE=${OUTPUT_PATH}/last_event_number.txt
echo "0" > $LAST_EVENT_NUMBER_FILE

for file in ${list_of_input_files}
do
    file_basename=`basename ${file}`
    file_number=`echo ${file_basename} | cut --d "_" --f 3`
    echo "File basename = ${file_basename} File number = ${file_number}"
    ${FECOM_SINGLE_LAUNCH} -i=${file} -op=${OUTPUT_PATH} -r=${RUN_NUMBER} -n=${MAX_NUMBER_OF_EVENTS}

done
