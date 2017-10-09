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

def check_value(a, b):
    if (a==b):
        return "SAME"
    else :
        return "DIFFERENT"

def check_all_values(a, b):
    check="SAME"
    for i in range(0, len(a)):
        check=check_value(a[0], b[0])
        if check == "DIFFERENT":
            return "DIFFERENT"
            break
    return "SAME"

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

# print("Data[0] length", len(data[0]))
# print("Data[1] length", len(data[1]))
# print("Data[2] length", len(data[2]))

# print("Data[0] length", data[0][69304])
# print("Data[1] length", data[1][69304])
# print("Data[2] length", data[2][69304])
#print data[0]

logging.debug('Total number of hits : %s', len(data[0]))

if (number_of_hits >= len(data[0])):
    number_of_hits = len(data[0]) - 1

adc_count_constant = 0.00061 # to convert into volt
sampling_period = 2.56e9 #GHz
tsampling= 1./sampling_period * 1e9 # in ps
resistor_value = 50 # Ohms
CFD_ratio=4./16.
precharge=64
charge_length=992
# Data list for histogram ploting :
baseline_mean_raw_jihane = []
# baseline_mean_volt_jihane = []
# baseline_mean_raw_sample = []
# baseline_mean_adc = []
baseline_mean_raw_diff = []
peak_position_hist = []
# peak_raw_waveform = []
peak_raw_jihane = []
peak_raw_diff = []
charge_jihane =[]
charge_diff =[]
falling_cell_jihane=[]
falling_cell_diff =[]
rising_cell_jihane =[]
rising_cell_diff = []
falling_offset_diff=[]
rising_offset_diff=[]

mean_baseline_mod_8_samples=[[],[ [] ]]
mean_sigma_baseline_mod_8_samples=[[],[ [] ]]
effective_hit_number=0
for i in range (1,32):
    mean_baseline_mod_8_samples[0].append(i)
    mean_sigma_baseline_mod_8_samples[0].append(i)

output_waveform_filename="waveforms_diff_values.dat"
output_waveform_filestream=open(output_dir+output_waveform_filename, 'w')

for i in range(0,number_of_hits):#range(len(data[0])):
    logging.debug("")
    logging.debug("*************************************************************")
    logging.debug("*********************NEW HIT*********************************")
    logging.debug("*************************************************************")
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
        slot_id=int(waveform_info_line.split("Slot", 2)[1].split()[0])
        channel_id=int(waveform_info_line.split(" Ch", 2)[1].split()[0])
        low_threshold=int(waveform_info_line.split(" LTO", 2)[1].split()[0])
        high_threshold=int(waveform_info_line.split(" HT", 2)[1].split()[0])
        event_id=int(waveform_info_line.split("EvtID", 2)[1].split()[0])
        timestamp_raw=int(waveform_info_line.split("RawTDC", 2)[1].split()[0])
        timestamp_ns=float(waveform_info_line.split(" TDC", 2)[1].split()[0])
        trig_count=int(waveform_info_line.split("TrigCount", 2)[1].split()[0])
        time_count=int(waveform_info_line.split("Timecount", 2)[1].split()[0])
        baseline_raw=int(waveform_info_line.split(" RawBaseline", 2)[1].split()[0])
        baseline_volt=float(waveform_info_line.split(" Baseline", 2)[1].split()[0])
        peak_raw=int(waveform_info_line.split(" RawPeak", 2)[1].split()[0])
        peak_volt=float(waveform_info_line.split(" Peak", 2)[1].split()[0])
        charge_raw=int(waveform_info_line.split(" RawCharge", 2)[1].split()[0])
        charge_pc=float(waveform_info_line.split(" Charge", 2)[1].split()[0])
        overflow=int(waveform_info_line.split(" Overflow", 2)[1].split()[0])
        rising_cell=int(waveform_info_line.split(" RisingCell", 2)[1].split()[0])
        rising_offset=int(waveform_info_line.split(" RisingOffset", 2)[1].split()[0])
        rising_time=float(waveform_info_line.split(" RisingTime", 2)[1].split()[0])
        falling_cell=int(waveform_info_line.split(" FallingCell", 2)[1].split()[0])
        falling_offset=int(waveform_info_line.split(" FallingOffset", 2)[1].split()[0])
        falling_time=float(waveform_info_line.split(" FallingTime", 2)[1].split()[0])
        FCR=int(waveform_info_line.split(" FCR", 2)[1].split()[0])

        if (high_threshold == 0):
            logging.debug("NO HIGH THRESHOLD")
        else:
            logging.debug("slot_id : %s, channel_id : %s, low_threshold : %s, high_threshold : %s, event_id : %s, timestamp_raw : %s, timestamp_ns : %s, trig_count : %s, time_count : %s, baseline_raw : %s, baseline_volt : %s, peak_raw : %s, peak_volt : %s, charge_raw : %s, charge_pc : %s, overflow : %s, rising_cell : %s, rising_offset : %s, rising_time : %s, falling_cell : %s, falling_offset : %s, falling_time : %s, FCR : %s", slot_id, channel_id, low_threshold, high_threshold, event_id, timestamp_raw, timestamp_ns, trig_count, time_count, baseline_raw, baseline_volt, peak_raw, peak_volt, charge_raw, charge_pc, overflow, rising_cell, rising_offset, rising_time, falling_cell, falling_offset, falling_time, FCR)
            # Parse waveform data :
            waveform_data=[],[] # x,y (x = sample number, y = sample_data)
            x_col, y_col = 0, 1
            sample_number=0
            for sample in waveform_data_line.split():
                waveform_data[0].append(sample_number)
                waveform_data[1].append(int(sample))
                sample_number+=1
            # logging.debug(waveform_data)
            waveform_data_wo_baseline=[]
            for j in range (0, len(waveform_data[y_col])):
                waveform_data_wo_baseline.append(waveform_data[y_col][j] - baseline_raw)

            # logging.debug waveform_data[y_col]
            # logging.debug waveform_data_wo_baseline
            # Redo all calcul from Jihane values :
            logging.debug("")
            logging.debug("Calcul from Jihane's values :")
            # Baseline :
            baseline_adc_fraction_jihane=baseline_raw/16.
            baseline_volt_jihane = float("{0:.6f}".format((baseline_raw / 16.) * adc_count_constant)) # OK

            baseline_16_first_samples=[]
            for j in range(0, 16):
                baseline_16_first_samples.append((waveform_data[y_col][j]))
            #print("Baseline 16 first samples", baseline_16_first_samples)
            baseline_recalculated=np.mean(baseline_16_first_samples)
            baseline_recalculated_adc_to_zero=np.mean(baseline_16_first_samples)-2048
            baseline_recalculated_raw_to_zero=baseline_recalculated_adc_to_zero*16.
            baseline_recalculated_volt_to_zero=baseline_recalculated_adc_to_zero*adc_count_constant
            logging.debug("Baseline Raw Jihane : %s", baseline_raw)
            logging.debug("Baseline Volt Jihane : %s", baseline_volt)
            logging.debug("Baseline moi mean recalculated RAW to zero : %s", baseline_recalculated_raw_to_zero)
            logging.debug("Baseline moi mean recalculated volt to zero : %s", baseline_recalculated_volt_to_zero)
            logging.debug("Baseline moi mean recalculated : %s", baseline_recalculated)
            logging.debug("Baseline moi mean recalculated ADC to zero : %s", baseline_recalculated_adc_to_zero)
            baseline_mean_raw_jihane.append(baseline_raw)
            baseline_mean_raw_diff.append(baseline_raw - baseline_recalculated_raw_to_zero)

            # Peak :
            peak_adc_fraction_jihane=peak_raw/8.
            peak_volt_jihane = float("{0:.6f}".format(peak_adc_fraction_jihane * adc_count_constant)) # OK
            peak_cell, peak_value = min(enumerate(waveform_data[y_col]), key=operator.itemgetter(1))
            peak_recalculated_adc=float("{0:.4f}".format(peak_value-baseline_recalculated))
            peak_recalculated_raw=peak_recalculated_adc*8.
            logging.debug("")
            logging.debug("Peak raw Jihane (extremum) : %s", peak_raw)
            #logging.debug("Peak volt Jihane : %s", peak_volt)
            logging.debug("Peak moi recalculated raw : %s", peak_recalculated_raw)
            logging.debug("Peak recalculated ADC : %s", peak_recalculated_adc)
            logging.debug("Peak position (sample number) : %s", peak_cell)
            logging.debug("Peak min value (from waveform) : %s (min at peak_position)", peak_value)
            # Question : comment c'est arrondi ?
            peak_position_hist.append(peak_cell)
            peak_raw_jihane.append(peak_raw)
            peak_raw_diff.append(peak_raw-peak_recalculated_raw)

            # Dynamic charge :
            charge_length=992 # in samples
            charge_lower_bound=peak_cell-64 # sample number
            charge_upper_bound=1024 # sample number
            charge_adc_samples=[]
            for j in range(charge_lower_bound, charge_upper_bound):
                charge_adc_samples.append(float("{0:.4f}".format(waveform_data[y_col][j]-baseline_recalculated)))
            #logging.debug(charge_adc_samples)
            charge_recalculated=np.sum(charge_adc_samples)
            logging.debug("")
            logging.debug("Charge ADC Jihane : %s", charge_raw)
            logging.debug("Charge moi recalculated : %s", charge_recalculated)
            logging.debug("")
            charge_jihane.append(charge_raw)
            charge_diff.append(int(charge_raw-charge_recalculated))

            # Try to calculate the charge and rising / falling time with raw or adC (but it's equivalent)
            edge_to_cross_adc_jihane=peak_recalculated_raw / 8. * CFD_ratio # peak_raw / 8. * CFD_ratio#
            falling_cell_adc=0
            rising_cell_adc=0
            waveform_polarity=0
            falling_already_crossed_adc=False
            rising_already_crossed_adc=False
            for j in range(charge_lower_bound, len(waveform_data[y_col])):
                sample_value_adc=waveform_data[y_col][j]
                sample_value_adc=waveform_data[y_col][j]-baseline_recalculated
                # logging.debug("Sample value adc", sample_value_adc)
                # logging.debug("Sample value ADC", sample_value_adc)
                if (falling_already_crossed_adc == False and sample_value_adc <= edge_to_cross_adc_jihane):
                    falling_cell_adc=j-1
                    falling_already_crossed_adc=True
                if (rising_already_crossed_adc == False and falling_already_crossed_adc == True and sample_value_adc >= edge_to_cross_adc_jihane):
                    rising_cell_adc=j-1
                    rising_already_crossed_adc=True
                if (falling_already_crossed_adc == False and sample_value_adc <= edge_to_cross_adc_jihane):
                    falling_cell_adc=j-1
                    falling_already_crossed_adc=True
                if (rising_already_crossed_adc == False and falling_already_crossed_adc == True and sample_value_adc >= edge_to_cross_adc_jihane):
                    rising_cell_adc=j-1
                    rising_already_crossed_adc=True
            logging.debug("Edge to cross ADC Jihane : %s", edge_to_cross_adc_jihane)
            logging.debug("Falling cell Jihane : %s", falling_cell)
            logging.debug("Falling cell moi : %s", falling_cell_adc)
            logging.debug("Rising cell Jihane : %s", rising_cell)
            logging.debug("Rising cell moi : %s", rising_cell_adc)
            falling_cell_sample_value=waveform_data[y_col][falling_cell_adc]#  + np.mean(baseline_recalculated_adc_to_zero)#-baseline_rawrecalculated
            falling_cell_plus_one_sample_value=waveform_data[y_col][falling_cell_adc+1] #+ np.mean(baseline_recalculated_adc_to_zero)#-baseline_recalculated
            # logging.debug("Falling cell sample value : %s", falling_cell_sample_value)
            # logging.debug("Falling cell+1 sample value : %s", falling_cell_plus_one_sample_value)
            rising_cell_sample_value=waveform_data[y_col][rising_cell_adc]#  + np.mean(baseline_recalculated_adc_to_zero)#-baseline_rawrecalculated
            rising_cell_plus_one_sample_value=waveform_data[y_col][rising_cell_adc+1] #+ np.mean(baseline_recalculated_adc_to_zero)#-baseline_recalculated
            # logging.debug("Rising cell sample value : %s", rising_cell_sample_value)
            # logging.debug("Rising cell+1 sample value : %s", rising_cell_plus_one_sample_value)

            # Falling offset calculation (in adc):
            falling_ya=0
            falling_yb=255
            x1=edge_to_cross_adc_jihane
            falling_xa=falling_cell_sample_value-baseline_recalculated # ADC value
            falling_xb=falling_cell_plus_one_sample_value-baseline_recalculated # ADC value
            falling_offset_recalculated = 0
            if (falling_xb - falling_xa) != 0:
                falling_offset_recalculated=falling_ya + (falling_yb - falling_ya) * ((x1 - falling_xa) / (falling_xb - falling_xa))
            falling_time_recalculated=(falling_cell_adc+falling_offset_recalculated/256.)* tsampling
            logging.debug("")
            logging.debug("Falling offset Jihane : %s", falling_offset)
            logging.debug("Falling offset moi recalculated : %s", falling_offset_recalculated)

            # Rising offset calculation (in adc):
            rising_ya=0
            rising_yb=255
            x2=edge_to_cross_adc_jihane
            rising_xa=rising_cell_sample_value-baseline_recalculated # ADC value
            rising_xb=rising_cell_plus_one_sample_value-baseline_recalculated # ADC value
            rising_offset_recalculated = 0
            if (rising_xb - rising_xa) != 0:
                rising_offset_recalculated=rising_ya + (rising_yb - rising_ya) * ((x2 - rising_xa) / (rising_xb - rising_xa))
            logging.debug("Rising offset Jihane : %s", rising_offset)
            logging.debug("Rising offset moi recalculated : %s", rising_offset_recalculated)
            falling_cell_jihane.append(falling_cell)
            falling_cell_diff.append(falling_cell - falling_cell_adc)
            rising_cell_jihane.append(rising_cell)
            rising_cell_diff.append(rising_cell - rising_cell_adc)
            falling_offset_diff.append(falling_offset - falling_offset_recalculated)
            rising_offset_diff.append(rising_offset - rising_offset_recalculated)

            rising_time_recalculated=(rising_cell_adc+rising_offset_recalculated/256.)* tsampling
            logging.debug("")
            logging.debug("Falling time Jihane : %s", falling_time)
            logging.debug("Falling time moi : %s", falling_time_recalculated)
            logging.debug("Rising time Jihane : %s", rising_time)
            logging.debug("Rising time moi : %s", rising_time_recalculated)

            logging.debug("")
            edge_line = []
            for i in range(0, len(waveform_data[y_col])):
                edge_line.append(baseline_recalculated+edge_to_cross_adc_jihane)

            if show_waveforms:
                plt.plot(waveform_data[x_col], waveform_data[y_col], 'r')
                plt.plot(falling_cell,     waveform_data[y_col][falling_cell], 'bo')
                plt.plot(falling_cell+1,   waveform_data[y_col][falling_cell+1], 'bo')
                plt.plot(rising_cell+1,    waveform_data[y_col][rising_cell+1], 'yo')
                plt.plot(rising_cell,      waveform_data[y_col][rising_cell], 'yo')
                plt.plot(edge_line, 'g')
                plt.show()


            charge_pc_jihane = float("{0:.6f}".format(charge_raw * adc_count_constant * tsampling / resistor_value))
            # Times :
            # QUESTION :
            # Rising time calculation : why cell + offset, it should be cell - offset because cell is the last cell after peak
            # Offset 0 a 255 dans le sens montant ? de rising_cell - 1 a rising_cell ?
            #   |    rising cell
            #--+--   - edge   + offset
            # |      rising cell -1

            # Solution probably : first cell after peak, is the first time the edge is crossed
            #   |    rising cell + 1
            #--+--   - edge   + offset
            # |      rising cell
            rising_time_jihane =float("{0:.6f}".format((rising_cell + rising_offset/256.) * tsampling))
            falling_time_jihane =float("{0:.6f}".format((falling_cell + falling_offset/256.) * tsampling))

            # logging.debug("baseline_raw", baseline_raw,
            #       "baseline_volt_jihane", baseline_volt_jihane,
            #       "peak_raw_jihane", peak_raw,
            #       "peak_volt_jihane", peak_volt_jihane,
            #       "charge_raw_jihane", charge_raw,
            #       "charge_pc_jihane", charge_pc_jihane,
            #       "rising_time_jihane", rising_time_jihane,
            #       "falling_time_jihane", falling_time_jihane)


            # My own calcul and analysis :
            # Baseline : mean value on X samples
            logging.debug("")
            logging.debug("Begin own analysis and redo all calculs :")

            # Different baseline calcul modulo 8 : [0-7], [0-15], [0-23], [0-31]...
            baseline_raw_mod_8 = []
            sigma_baseline_raw_mod_8 = []
            mean_baseline_mod_8_samples[1].append([])
            mean_sigma_baseline_mod_8_samples[1].append([])
            for j in range(1, 32):
                baseline_raw_calc=[]
                for j in range(0, 8*j):
                    baseline_raw_calc.append((waveform_data[y_col][j]-2048)*16)
                baseline_raw_calc_mean=np.mean(baseline_raw_calc)
                sigma_baseline_raw_calc_mean=math.sqrt(abs(baseline_raw_calc_mean))
                baseline_raw_mod_8.append(baseline_raw_calc_mean)
                sigma_baseline_raw_mod_8.append(sigma_baseline_raw_calc_mean)

                mean_baseline_mod_8_samples[1][effective_hit_number].append(abs(baseline_raw_calc_mean))
                mean_sigma_baseline_mod_8_samples[1][effective_hit_number].append(sigma_baseline_raw_calc_mean)

            # logging.debug(baseline_raw_mod_8)
            # logging.debug(sigma_baseline_raw_mod_8)

            # logging.debug(mean_baseline_mod_8_samples[1][effective_hit_number])
            # logging.debug(mean_sigma_baseline_mod_8_samples[1][effective_hit_number])

            peak_raw_mod_8 = []
            # Calcul RawPeak and test for charge calculation
            peak_position, peak_sample_value_min = min(enumerate(waveform_data[y_col]), key=operator.itemgetter(1))
            # logging.debug("Peak position  = ", peak_position)
            # logging.debug("Peak sample value min = ", peak_sample_value_min)
            # logging.debug("Peak raw (given by Jihane)", peak_raw)
            # logging.debug("Test peak", round(2048 + ((peak_raw) / 8.)))
            # for i in range(0, len(baseline_raw_mod_8)):
            #     peak_raw_mod_8.append(peak_sample_value_min - baseline_raw_mod_8[i])
            # logging.debug(peak_raw_mod_8)
            effective_hit_number+=1

            # Check if waveform values are different from Jihane and save this waveform in an other file for further analysis :

            waveform_different=False
            if (baseline_mean_raw_diff[-1] != 0 or peak_raw_diff[-1] != 0 or falling_cell_diff[-1] != 0 or rising_cell_diff[-1] != 0 or falling_offset_diff[-1] != 0 or rising_offset_diff[-1] != 0):
                waveform_different = True

            if (waveform_different):
                save_waveform(output_waveform_filestream, hit_line, waveform_info_line, waveform_data_line)


# Close the output filestream
output_waveform_filestream.close()

logging.info("Effective hit number (HT) : %s ", effective_hit_number)

# Plot histograms :

if (print_histos):
    f, ax = plt.subplots(2, 6, figsize=(18,18))
    n, bins, patches = ax[0,0].hist(baseline_mean_raw_jihane, 50) #, normed=1)facecolor='green', alpha=0.75)
    ax[0, 0].set_title('Baseline raw Jihane')
    n, bins, patches = ax[1,0].hist(baseline_mean_raw_diff, 50)
    ax[1, 0].set_title('Baseline diff')
    n, bins, patches = ax[0,1].hist(peak_raw_jihane, 50)
    ax[0, 1].set_title('Peak raw Jihane')
    n, bins, patches = ax[1,1].hist(peak_raw_diff, 50)
    ax[1, 1].set_title('Peak diff')
    n, bins, patches = ax[0,2].hist(charge_jihane, 50)
    ax[0, 2].set_title('Charge raw Jihane')
    n, bins, patches = ax[1,2].hist(charge_diff, 50)
    ax[1, 2].set_title('Charge diff')
    n, bins, patches = ax[0,3].hist(falling_cell_diff, 30)
    ax[0, 3].set_title('Falling cell diff')
    n, bins, patches = ax[1,3].hist(rising_cell_diff, 30)
    ax[1, 3].set_title('Rising cell diff')
    n, bins, patches = ax[0,4].hist(falling_offset_diff, 25)
    ax[0, 4].set_title('Falling offset diff')
    n, bins, patches = ax[1,4].hist(rising_offset_diff, 25)
    ax[1, 4].set_title('Rising offset diff')

    output_histos_png="output_histos.png"
    output_histos_pdf="output_histos.pdf"
    f.savefig(output_dir+output_histos_png)
    f.savefig(output_dir+output_histos_pdf)


    # for i in range (0, effective_hit_number):
    #     RGB=(random.uniform(0, 1), random.uniform(0, 1), random.uniform(0, 1))
    #     ax[0, 5].plot(mean_baseline_mod_8_samples[0], mean_baseline_mod_8_samples[1][i], color=RGB, marker='o', markersize=4, linestyle="None")
    # ax[0, 5].set_title('Baseline evolution last hit')

    plt.show()

# end of program
