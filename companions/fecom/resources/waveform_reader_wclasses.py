import sys
import getopt
import numpy as np
import operator
import scipy as sp
import matplotlib
import matplotlib.pyplot as plt
from math import *
import logging
import random
import math
import waveform_datamodel as wf_dat
import collections

def usage():
    """Command line usage"""
    print("Usage for script: ", sys.argv[0])
    print("Options :")
    print("  -h (--help) : produce this help message")
    print("  -f (--filename) [arg] : input filename (not available yet")
    print("  -n (--number-of-hits) [arg] : number of input hits")
    print("  -d (--debug) : print debug messages")
    print("  -p (--print) : print and draw histograms")
    print("  -s (--show) :  show and plot waveforms")
    print("  -o (--output-dir) : output dir")

def save_waveform(filestream, hit_line, waveform_info_line, waveform_data_line):
    filestream.write(hit_line)
    filestream.write(waveform_info_line)
    filestream.write(waveform_data_line)

# Parse arguments :
filename = ""
number_of_hits=0
debug=False
print_histos=False
show_waveforms=False
output_dir=""
opts, args = getopt.getopt(sys.argv[1:], "hf:n:dpo:s", ["help","filename=","number-of-hits=","debug","print","output-dir=","show"])
for opt, arg in opts:
    if opt in ("-h", "--help"):
        usage()
        sys.exit()
    elif opt in ("-f", "--filename"):
        filename=str(arg)
    elif opt in ("-n", "--number-of-hits"):
        number_of_hits =int(arg)
    elif opt in ("-d", "--debug"):
        debug=True
    elif opt in ("-p", "--print"):
        print_histos=True
    elif opt in ("-s", "--show"):
        show_waveforms=True
    elif opt in ("-o", "--output-dir"):
        output_dir=str(arg)

# Default input file :
if not filename:
    filename = "Run_40.dat"
    input_file = filename
    filestream = open(input_file, 'r')

# Default output dir :
if not output_dir:
    output_dir="/tmp/"

logging_priority=logging.INFO
if (debug):
    logging_priority=logging.DEBUG

logging.basicConfig(level=logging_priority, format='%(levelname)s - %(message)s')

logging.info("Begin python program : %s", sys.argv[0])
logging.info("Filename : %s, Number of hits : %s", filename, number_of_hits)

header_size=9
header = []
# data : 3D tabular
# data[0] Hit / Trig_ID | data[1] waveform information | data[2] waveform data
data = [[],[],[]]

line_number = 0
number_of_tracker_hits=0
for line in filestream:
    if line_number < header_size:
        header.append(line)
    if line_number >= header_size:
        if ('HIT') in line:
            if ('CALO') in line:
                data[0].append(line)
        elif ('Slot') in line:
            if ('LTO') in line:
                data[1].append(line)
        else:
            data[2].append(line)
    line_number+=1

logging.debug('Total number of hits : %s', len(data[0]))

if (number_of_hits >= len(data[0])):
    number_of_hits = len(data[0]) - 1

wf_histos=wf_dat.waveform_histos()

for i in range(0, number_of_hits):
    logging.debug("")
    logging.debug("*************************************************")
    logging.debug("*********************NEW HIT*********************")
    logging.debug("*************************************************")
    logging.debug("i : %s", i)
    hit_line = data[0][i]
    # Parse hit line :
    logging.debug ("Hit line : %s", hit_line)
    hit_number = hit_line.split("HIT", 2)[1].split()[0]
    hit_type = hit_line.split(" = ", 2)[1]
    trigger_id = hit_line.split("TRIG_ID", 2)[1].split()[0]
    logging.debug ("Hit number : %s, TRIGGER_ID : %s, Hit type : %s", hit_number, trigger_id, hit_type)
    if (hit_type == "CALO"):
        waveform_info_line = data[1][i]
        waveform_data_line = data[2][i]
        # logging.debug ('hit_line, waveform_info_line, waveform_data_line')
        # Parse waveform info :
        # Parse that way because it is variable position independant.
        # Simplification idea : use a regex ?
        metadata = {'slot_id':int(waveform_info_line.split("Slot", 2)[1].split()[0]),
                    'channel_id':int(waveform_info_line.split(" Ch", 2)[1].split()[0]),
                    'low_threshold':int(waveform_info_line.split(" LTO", 2)[1].split()[0]),
                    'high_threshold':int(waveform_info_line.split(" HT", 2)[1].split()[0]),
                    'event_id':int(waveform_info_line.split("EvtID", 2)[1].split()[0]),
                    'timestamp_raw':int(waveform_info_line.split("RawTDC", 2)[1].split()[0]),
                    'timestamp_ns':float(waveform_info_line.split(" TDC", 2)[1].split()[0]),
                    'trig_count':int(waveform_info_line.split("TrigCount", 2)[1].split()[0]),
                    'time_count':int(waveform_info_line.split("Timecount", 2)[1].split()[0]),
                    'baseline_raw':int(waveform_info_line.split(" RawBaseline", 2)[1].split()[0]),
                    'baseline_volt':float(waveform_info_line.split(" Baseline", 2)[1].split()[0]),
                    'peak_raw':int(waveform_info_line.split(" RawPeak", 2)[1].split()[0]),
                    'peak_volt':float(waveform_info_line.split(" Peak", 2)[1].split()[0]),
                    'charge_adc':int(waveform_info_line.split(" RawCharge", 2)[1].split()[0]),
                    'charge_pc':float(waveform_info_line.split(" Charge", 2)[1].split()[0]),
                    'overflow':int(waveform_info_line.split(" Overflow", 2)[1].split()[0]),
                    'rising_cell':int(waveform_info_line.split(" RisingCell", 2)[1].split()[0]),
                    'rising_offset':int(waveform_info_line.split(" RisingOffset", 2)[1].split()[0]),
                    'rising_time':float(waveform_info_line.split(" RisingTime", 2)[1].split()[0]),
                    'falling_cell':int(waveform_info_line.split(" FallingCell", 2)[1].split()[0]),
                    'falling_offset':int(waveform_info_line.split(" FallingOffset", 2)[1].split()[0]),
                    'falling_time':float(waveform_info_line.split(" FallingTime", 2)[1].split()[0]),
                    'FCR':int(waveform_info_line.split(" FCR", 2)[1].split()[0]) }
        metadata['baseline_adc'] = metadata['baseline_raw'] // 2 / 8.
        metadata['peak_adc'] = metadata['peak_raw'] / 8.
        logging.debug("Board metadata : %s", collections.OrderedDict(sorted(metadata.items(), key=lambda t: t[0])))

        if (metadata['high_threshold'] == 0):
            logging.debug("NO HIGH THRESHOLD")
        else:
            #logging.debug("slot_id : %s, channel_id : %s, low_threshold : %s, high_threshold : %s, event_id : %s, timestamp_raw : %s, timestamp_ns : %s, trig_count : %s, time_count : %s, baseline_raw : %s, baseline_volt : %s, peak_raw : %s, peak_volt : %s, charge_raw : %s, charge_pc : %s, overflow : %s, rising_cell : %s, rising_offset : %s, rising_time : %s, falling_cell : %s, falling_offset : %s, falling_time : %s, FCR : %s", slot_id, channel_id, low_threshold, high_threshold, event_id, timestamp_raw, timestamp_ns, trig_count, time_count, baseline_raw, baseline_volt, peak_raw, peak_volt, charge_raw, charge_pc, overflow, rising_cell, rising_offset, rising_time, falling_cell, falling_offset, falling_time, FCR)

            wf_recalc=wf_dat.waveform_recalc()
            wf_recalc.parse_waveform(waveform_data_line)
            # wf_recalc.print_raw_waveform()
            wf_recalc.recalcul_from_waveform()
            #wf_recalc.tree_dump()

            wf_histos.add_signal_to_histos(metadata, wf_recalc.metadata)

            # wf_histos.mean_baseline_mod_8_samples.append(...



# Outside signal loop

#wf_histos.print_histos()
