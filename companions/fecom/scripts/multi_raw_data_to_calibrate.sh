INPUT_PATH=""
OUTPUT_PATH=""
RUN_NUMBER=0
MAX_NUMBER_OF_EVENTS=0

function usage()
{
    echo "Goal : Process all files contains in the Run_X/ path."
    echo "       Launch raw_data_to_calibrate.sh script for each file of the run"
    echo "Options : -i   [--input-path]   Enter the input path "
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
        -i | --input-path)
            INPUT_PATH=$VALUE
            ;;
        -op | --output-path)
            OUTPUT_PATH=$VALUE
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

mkdir -p  ${OUTPUT_PATH}/Run_${RUN_NUMBER}

echo "INPUT_PATH is $INPUT_PATH";
echo "OUTPUT_PATH is $OUTPUT_PATH";
echo "RUN_NUMBER is $RUN_NUMBER";
echo "MAX_NUMBER_OF_EVENTS is $MAX_NUMBER_OF_EVENTS";

SINGLE_LAUNCH=raw_data_to_calibrate.sh

list_of_input_files=`ls ${INPUT_PATH}/Run_*.dat*`

for file in ${list_of_input_files}
do
    file_basename=`basename ${file}`
    file_number=`echo ${file_basename} | cut --d "_" --f 3`
    echo "File basename = ${file_basename} File number = ${file_number}"
    echo "./${SINGLE_LAUNCH} -i=${file} -op=${OUTPUT_PATH} -r=${RUN_NUMBER} -n=${MAX_NUMBER_OF_EVENTS}"

done
