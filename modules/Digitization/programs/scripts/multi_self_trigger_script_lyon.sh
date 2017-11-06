#!/usr/bin/env bash
# multi_self_trigger_script_lyon.sh

qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 1 -s 1 -c 1 -t 0.1
#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 2 -s 1 -c 1 -t 1
#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 3 -s 1 -c 1 -t 10
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 4 -s 1 -c 1 -t 100

#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 5 -s 1 -c 5 -t 0.1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 6 -s 1 -c 5 -t 1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 7 -s 1 -c 5 -t 10
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 8 -s 1 -c 5 -t 100

#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 9 -s 1 -c 10 -t 0.1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 10 -s 1 -c 10-t 1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 11 -s 1 -c 10-t 10
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 12 -s 1 -c 10 -t 100

#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 13 -s 1 -c 50 -t 0.1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 14 -s 1 -c 50 -t 1
# qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 15 -s 1 -c 50 -t 10
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 16 -s 1 -c 50 -t 100


qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 17 -s 1 -c 100 -t 0.1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 18 -s 1 -c 100-t 1
#qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 19 -s 1 -c 100 -t 10
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 20 -s 1 -c 100 -t 100

qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 21 -s 10 -c 5 -t 0.1
qsub -V -P P_nemo -l h_cpu=12:00:00,s_cpu=11:30:00,fsize=8000M,sps=1 self_trigger_script.sh -r 22 -s 10 -c 50 -t 100


# debug=1

# echo "Starting..." >&2


# function usage(){
# echo "--------------"
# echo "Goal : Produce self trigger hits and pack them into events @ 1600 ns."
# echo "--------------"
# echo "How to use it"
# echo " "
# echo "$ ./self_trigger_script_lyon [OPTIONS] [ARGUMENTS]"
# echo ""
# echo "Allowed options: "
# echo "-h  [ --help ]     produce help message"
# echo " "
# echo "./multi_self_trigger_script_lyon"
# echo " "
# echo "--------------"
# echo "Example : "
# echo "./multi_self_trigger_script"
# echo " "
# }

# #### ->MAIN<- #####

# START_DATE=`date "+%Y-%m-%d"`

# SCRIPT_DIR=/sps/nemo/scratch/golivier/software/Falaise/module/Digitization/programs/scripts/
# ST_SCRIPT_NAME=self_trigger_script_lyon.sh

# calo_freq=1
# tracker_req=1
# time_interval=1
# run_number=0

# # Chain qsubs to submit at cclyon