#!/usr/bin/env bash
# multi_self_trigger_script_lyon.sh

debug=1

echo "Starting..." >&2


function usage(){
echo "--------------"
echo "Goal : Produce self trigger hits and pack them into events @ 1600 ns."
echo "--------------"
echo "How to use it"
echo " "
echo "$ ./self_trigger_script_lyon [OPTIONS] [ARGUMENTS]"
echo ""
echo "Allowed options: "
echo "-h  [ --help ]     produce help message"
echo " "
echo "./multi_self_trigger_script_lyon"
echo " "
echo "--------------"
echo "Example : "
echo "./multi_self_trigger_script"
echo " "
}

#### ->MAIN<- #####

START_DATE=`date "+%Y-%m-%d"`

SCRIPT_DIR=/sps/nemo/scratch/golivier/software/Falaise/module/Digitization/programs/scripts/
ST_SCRIPT_NAME=self_trigger_script_lyon.sh

calo_freq=1
tracker_req=1
time_interval=1
run_number=0

# Chain qsubs to submit at cclyon