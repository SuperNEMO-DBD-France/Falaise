#!/bin/bash

# cclyon_add=nemoprod@ccage.in2p3.fr:/sps/nemo/snemo/half_commissioning/

SOFT_PATH=${FALAISE_BUILD_DIR}/BuildProducts/bin/
soft_convertor=fecom-main_decoder_serializer
histo_builder=fecom-main_histogram_builder

INPUT_PATH=/data/nemo/group/Private/goliviero/data_half_commissioning/
RAW_INPUT_PATH=${INPUT_PATH}/raw
#DATABASE_FILE=/data/nemo/group/Private/goliviero/database_serialized.db

prefix_file_name=Run_
suff_file_name=.dat
output_suff_file_name=serialized.data.bz2

DATE=`date +%F`

echo "*******************************************************"
echo "*   Enter in half-commissioning serializer automaton  *"
echo "*                                                     *"
echo "*******************************************************"

echo "Enter Run filename to process : "

read run_name
OUTPUT_PATH=${INPUT_PATH}/serialized/${run_name}/

echo "INFO  : You will process run called : " ${run_name}
echo "INFO  : Check if the run exist in the input path : ${RAW_INPUT_PATH}"

if [ ! -d "${RAW_INPUT_PATH}/${run_name}" ]; then
    # Control will enter here if $DIRECTORY doesn't exist.
    echo "FATAL : Input directory doesn't exist in the input path : ${RAW_INPUT_PATH}, copy your run in the right place : ${RAW_INPUT_PATH} "
    exit
fi

if [ ! -f "${RAW_INPUT_PATH}/${run_name}/mapping_calo.csv" ]
then
    echo "FATAL : Mapping calo doesn't exist, build it before processing !"
    echo "INFO  : Check example file in /home/lpc/mapping_calo_example.csv and rename it mapping_calo.csv !"
    exit
fi

if [ ! -f "${RAW_INPUT_PATH}/${run_name}/mapping_tracker.csv" ]
then
    echo "FATAL : Mapping tracker doesn't exist, build it before processing !"
    echo "INFO  : Check example file in /home/lpc/mapping_tracker_example.csv and rename it mapping_tracker.csv !"
    exit
fi

run_number=`echo ${run_name} | cut --d "_" --f 2`
list_of_input_file=`ls ${RAW_INPUT_PATH}/${run_name}/${prefix_file_name}${run_number}${suff_file_name}*`

if [ $? -ne 0 ];
then
    echo "WARNING : No data files from FEB (Run_*.dat* type) in your directory ${RAW_INPUT_PATH}/${run_name} !"
else
    ## Prepare directories
    mkdir -p ${OUTPUT_PATH}
    if [ $? -ne 0 ];
    then
        echo "ERROR : mkdir command for ${OUTPUT_PATH} failed !"
    fi
    cp ${RAW_INPUT_PATH}/${run_name}/mapping_calo.csv ${OUTPUT_PATH}/
    if [ $? -ne 0 ];
    then
        echo "ERROR : can not copy mapping_calo.csv...."
    fi
    cp ${RAW_INPUT_PATH}/${run_name}/mapping_tracker.csv ${OUTPUT_PATH}/
    if [ $? -ne 0 ];
    then
        echo "ERROR : can not copy mapping_tracker.csv..."
    fi
    ## End of prepare directories
fi

echo "INFO : List of files " ${list_of_input_file}
file_counter=0

for a_file in ${list_of_input_file}
do
    echo "File counter = " ${file_counter}
    # echo "Input file to convert is : " ${a_file}
    cutted_file=`echo ${a_file} | cut --d="." --f=1`
    output_logfile=${cutted_file}_${file_counter}.log
    output_file=${cutted_file}_${file_counter}_$output_suff_file_name
    #       echo "Output log file : " $output_logfile
    #       echo "Output serialized file : " $output_file
    base_output_file=`basename $output_file`
    echo "OUTPUT PATH : ${OUTPUT_PATH}"
    ${SOFT_PATH}/${soft_convertor} -i ${a_file} -op ${OUTPUT_PATH} > ${output_logfile} 2>&1
    if [ $? -ne 0 ];
    then
        echo "ERROR : ${soft_convertor} failed !"
    fi
    output_serialized_name=${OUTPUT_PATH}/${base_output_file}
    echo "Output serialized name = " ${output_serialized_name}
    mv ${OUTPUT_PATH}/output_serialized.data.bz2 ${output_serialized_name}
    if [ $? -ne 0 ];
    then
        echo "ERROR : move output_serialized.data.bz2 into " ${output_serialized_name} "failed !"
    fi

    mkdir -p ${OUTPUT_PATH}/log
    mv ${output_logfile} ${OUTPUT_PATH}/log/
    if [ $? -ne 0 ];
    then
        echo "ERROR : move output_logfile into " ${output_logfile} "failed !"
    fi

    let file_counter+=1
done
last_run_file_number=`ls ${list_of_input_file} | sort -n | tail -n 1 | cut --d "_" --f 6 | sed 's/^0*//' `
# echo "DEBUG : Last run file number : ${last_run_file_number}"
last_serialized_file_number=`ls -v ${OUTPUT_PATH}/*_serialized.data.bz2 | tail -n 1 | cut --d "_" --f 6 `
# echo "DEBUG : Last serialized file number : ${last_run_file_number}"
serialization_status="bad"
echo "INFO : last run file number = ${last_run_file_number}, last serialized file number = ${last_serialized_file_number}"

if [ ${last_run_file_number} -eq ${last_serialized_file_number} ];
then
    echo "INFO : Serialization seems OK"
    serialization_status="success"
else
    echo "INFO : Serialization seems not OK, flag status is bad"
fi
echo "${DATE} ${run_name} ${serialization_status} ${transfer_status}" # >> ${DATABASE_FILE}
