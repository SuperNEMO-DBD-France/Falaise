#!/usr/bin/env bash
# hc_launch_serialization_cclyon.sh
APP_NAME=hc_launch_serialization_cclyon.sh
debug=1

SW_PATH="/sps/nemo/scratch/golivier/software/Falaise/build/BuildProducts/bin/"

DECODE_SW=fecom-decode_sort_serialize
BUILDER_SW=fecom-hc_event_builder

function usage(){
echo "--------------"
echo "Goal : Launch serialization for HC raw data at CCLyon"
echo "2 software are launched : fecom::decode_sort_serialized and fecom::hc_event_builder" 
echo "First one convert raw data hits into serialized data (brio format)" 
echo "Second one build the commissioning hits into commissioning events with reasonable timings"
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./${APP_NAME} [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo "-n  [ --number ]   set the number of events"
echo "-r  [--run-number] set the run number to analyze"
echo "-C  [--calo-mapping] set the calo mapping rules for sorting"
echo "-T  [--tracker-mapping] set the tracker mapping rules for sorting"
echo " "
echo "./${APP_NAME} -r 0 -n number_of_events -C CALO_FILE -T TRACKER_FILE"
echo "Default value : number_of_events = 10"
echo " "
echo "--------------"
echo "Example : "
echo "./${APP_NAME} -n 100000 -r 0 -C CALO_FILE -T TRACKER_FILE"
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

INPUT_DIR="/sps/nemo/snemo/half_commissioning/"
INPUT_RAW_DIR="${INPUT_DIR}/raw/Run_${run_number}/"
INPUT_FILES=`ls ${INPUT_RAW_DIR}/*.dat*`
echo "List of input files : ${INPUT_FILES}"
NUMBER_OF_FILE=`ls ${INPUT_RAW_DIR}/*.dat* | wc -l`
TOTAL_NBR_OF_EVENTS=`echo "$((${NUMBER_OF_FILE} * ${nb_event}))"`
echo "TOTAL_NBR_OF_EVENTS = ${TOTAL_NBR_OF_EVENTS}"

CALO_MAPPING_FILE=mapping_calo.csv
TRACKER_MAPPING_FILE=mapping_tracker.csv

OUTPUT_PATH=/sps/nemo/scratch/golivier/data_half_commissioning
SERIALIZED_OUTPUT_PATH=${OUTPUT_PATH}/serialized/
SERIALIZED_RUN_PATH=${SERIALIZED_OUTPUT_PATH}/Run_${run_number}

SERIALIZED_DECODED_PATH=${SERIALIZED_RUN_PATH}/decoded
SERIALIZED_BUILDED_PATH=${SERIALIZED_RUN_PATH}/builded
SERIALIZED_SETTING_PATH=${SERIALIZED_RUN_PATH}/settings
SERIALIZED_LOG_PATH=${SERIALIZED_RUN_PATH}/log_files.d

DECODE_OUTPUT_FILENAME=decode_sort-output.data.bz2
DECODE_OUTPUT_SUFFIX=decode.data.bz2

BUILD_OUTPUT_FILENAME=hc_event_builder_serialized.brio
BUILD_OUTPUT_SUFFIX=builded.brio

# mkdir -p $SERIALIZED_DECODED_PATH $SERIALIZED_BUILDED_PATH $SERIALIZED_SETTING_PATH $SERIALIZED_LOG_PATH

file_counter=0

for file in ${INPUT_FILES}
do
    echo "File counter                : " ${file_counter}
    echo "Input file to convert is    : " ${file}

    cutted_file=`echo ${file} | cut --d="." --f=1`
    short_name=`basename ${cutted_file}`
    OUTPUT_DECODE_FILE=${SERIALIZED_DECODED_PATH}/${short_name}_${file_counter}_${DECODE_OUTPUT_SUFFIX}
    DECODE_LOG_FILE=${short_name}_${file_counter}_decode.log
    echo "DEBUG : TRACE 1"
    
    echo "INFO : Ready to ${SOFT_PATH}/${soft_convertor}"  > ${SERIALIZED_LOG_PATH}/${DECODE_LOG_FILE} 2>&1
    echo "----------------------------------------------"  >> ${SERIALIZED_LOG_PATH}/${DECODE_LOG_FILE} 2>&1

    ${SW_PATH}/${DECODE_SW} -i ${file} -o ${SERIALIZED_DECODED_PATH} -M ${nb_event} >> ${SERIALIZED_LOG_PATH}/${DECODE_LOG_FILE} 2>&1
    if [ $? -ne 0 ];
    then
   	echo "ERROR : ${DECODE_SW} FAILED !"
	exit 1
    else 
	echo "${DECODE_SW} SUCCESS"
    fi

    echo "DEBUG : TRACE 2"
    mv ${SERIALIZED_DECODED_PATH}/${DECODE_OUTPUT_FILENAME} ${OUTPUT_DECODE_FILE}
    if [ $? -eq 1 ];
    then
	echo "ERROR : mv ${SERIALIZED_DATA_PATH}/${DECODE_OUTPUT_FILENAME} ${OUTPUT_DECODE_FILE} FAILED !"
	exit 1
    fi

    echo "DEBUG : TRACE 3"
    stat ${SERIALIZED_BUILDED_PATH}/last_event_number.txt > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
	echo "INFO : ********************** ${SERIALIZED_BUILDED_PATH}/last_event_number.txt does not exist..."
	echo "INFO : ${SERIALIZED_BUILDED_PATH}/last_event_number.txt does not exist..."
	touch ${SERIALIZED_BUILDED_PATH}/last_event_number.txt
	
	echo "0" > ${SERIALIZED_BUILDED_PATH}/last_event_number.txt
	last_event_nb=0
    else
	echo "INFO : ********************** ${SERIALIZED_BUILDED_PATH}/last_event_number.txt exist..."
	last_event_nb=`cat ${SERIALIZED_BUILDED_PATH}/last_event_number.txt`
	echo "ELSE last_event_nb = ${last_event_nb}"
	let last_event_nb=last_event_nb+1
    fi

    echo "DEBUG : TRACE 4"
    echo "LAST EVENT NUMBER = ${last_event_nb}"
    
    BUILD_LOG_FILE=${short_name}_${file_counter}_build.log
    
    echo "INFO : Ready to ${SOFT_PATH}/${event_builder}" > ${SERIALIZED_LOG_PATH}/${BUILD_LOG_FILE} 2>&1
    echo "----------------------------------------------"  >> ${SERIALIZED_LOG_PATH}/${BUILD_LOG_FILE} 2>&1
    echo "INFO : Start event number $last_event_nb" >> ${SERIALIZED_LOG_PATH}/${BUILD_LOG_FILE} 2>&1 
	
    echo "LAST EVENT NUMBER = ${last_event_nb}" >> ${SERIALIZED_LOG_PATH}/${BUILD_LOG_FILE} 2>&1   

    ${SW_PATH}/${BUILDER_SW} -i ${OUTPUT_DECODE_FILE} -o ${SERIALIZED_BUILDED_PATH} -M ${nb_event} -r ${run_number} -c ${HC_CALO_MAPPING_CONFIG_FILE} -t ${HC_TRACKER_MAPPING_CONFIG_FILE} -e ${last_event_nb} >> ${SERIALIZED_LOG_PATH}/${BUILD_LOG_FILE} 2>&1
    if [ $? -ne 0 ];
    then
   	echo "ERROR : ${BUILDER_SW} FAILED !"
	exit 1
    else 
	echo "${BUILDER_SW} SUCCESS"
    fi
    
    OUTPUT_BUILD_FILE=${SERIALIZED_BUILDED_PATH}/${short_name}_${file_counter}_${BUILD_OUTPUT_SUFFIX}

    mv ${SERIALIZED_BUILDED_PATH}/${BUILD_OUTPUT_FILENAME} ${OUTPUT_BUILD_FILE}
    if [ $? -eq 1 ];
    then
	echo "ERROR : mv ${SERIALIZED_BUILDED_PATH}/${BUILD_OUTPUT_FILENAME} ${OUTPUT_BUILD_FILE} FAILED !"
	exit 1
    fi
    

    let file_counter+=1
    
done  
