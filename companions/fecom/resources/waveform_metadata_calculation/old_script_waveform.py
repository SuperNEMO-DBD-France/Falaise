import numpy as np
import operator
import scipy as sp
import matplotlib.pyplot as plt
from math import *

#np.set_printoptions(threshold=np.nan)
filename_1= 'input_waveforms/waveform.txt'
filename_2= 'input_waveforms/waveform2.txt'
filename_3= 'input_waveforms/waveform3.txt'

calibfile_1="input_waveforms/calib_ch1.txt"
calibfile_3="input_waveforms/calib_ch3.txt"

file_used=filename_3
calibfile_used=calibfile_3

# waveform from Run_40.dat, 2nd hit. Slot 2 Ch 1 HT 1
data_file = np.genfromtxt(file_used, delimiter=' ')

FCR=open(file_used).readline().rstrip().split()[2]
print("First cell read : ", FCR)

x, y = data_file.transpose()
print("Raw values   : ", y)

# Calib slot 2 channel 1 of run 40
calib_file = np.genfromtxt(calibfile_used, delimiter=' ')
y_calib=calib_file.transpose()/16

# Calibration from First Cell Read for this signal (0 <-> FCR (960 here) ), we have to shift the 0
y_calib_signal=[]
for i in range (0,1024):
    if i < 1024-int(FCR):
        y_calib_signal.append(y_calib[i+int(FCR)])
    else:
        y_calib_signal.append(y_calib[i-(1024-int(FCR))])
print(("Calib values shifted :", y_calib_signal))

y_corrected=(y-y_calib_signal)
print("Real values (raw-calib_signal) :", y_corrected)

adc_count_constant = 0.00061 # to convert into volt
sampling_period = 2.56e9 #GHz
tsampling= 1./sampling_period * 1e9 # in ps
resistor_value = 50 # Ohms
print("Tsampling (ns) =", tsampling)

print ("Calcul baseline, 16 first samples without calibration")
y_baseline=[]
for i in range (0,16):
    y_baseline.append((y[i]-2048)*16)

y_raw_baseline=np.mean(y_baseline)
y_volt_baseline = y_raw_baseline / 16 * adc_count_constant

print("Raw baseline      = ", y_raw_baseline)
print("Volt baseline (V) = ", y_volt_baseline)


peak_position, peak_value_min = min(enumerate(y_corrected), key=operator.itemgetter(1))
print("Peak position  = ", peak_position)
print("Peak value min = ", peak_value_min)
print(y[peak_position])

# Raw value corrected by the baseline

# Probleme avec cette valeur :
# A VOIR :
#peak_raw_value=int(round((peak_value_min * 8)-y_raw_baseline))
peak_raw_value=(peak_value_min * 8)-y_raw_baseline
peak_raw_value_wo_baseline=(peak_value_min * 8)
peak_raw_value_w_pedestal=(peak_value_min * 8)-y_raw_baseline #- (y_calib_signal[peak_position])

print("Peak raw value - baseline = ", peak_raw_value)
print("Peak raw value = ", peak_raw_value_wo_baseline)
print("Peak raw value - baseline - pedestal      = ", peak_raw_value_w_pedestal)
# Volt value not corrected by the
peak_volt_value=peak_value_min * adc_count_constant
#peak_volt_value=(y_corrected[peak_position] - y_volt_baseline)* adc_count_constant
print("Peak raw value      = ", peak_raw_value)
print("Peak volt value (V) = ", peak_volt_value)

y_charge=[]
y_charge_test=[]
# Dynamic charge :
charge_length=992 # in samples
charge_lower_bound=peak_position-64 # sample number
charge_upper_bound=1024 # sample number

# Static charge :
# charge_lower_bound=16
# charge_upper_bound=charge_lower_bound + charge_length

print ("Lower bound = ", charge_lower_bound, "Upper bound = ", charge_upper_bound)

for i in range (charge_lower_bound, charge_upper_bound):
    y_charge.append(y[i]-2048)
    y_charge_test.append(y_corrected[i])

integral_charge=sum(y_charge) # Integral charge not OK
integral_charge_test=sum(y_charge_test)

charge_pc=integral_charge * adc_count_constant * tsampling / resistor_value # Calcul OK

print("Raw charge = ", integral_charge)
print("Charge (pc)  = ", charge_pc)
print("Raw charge test = ", integral_charge_test)

# Rising / Falling cell calculation
falling_cell=0 # Last before peak
rising_cell=0 # First after peak
y_corrected_volt=y_corrected
y_corrected_volt *= adc_count_constant
print("Y_corrected_volt = ", y_corrected_volt)

# Fixed threshold :

# threshold=-0.05 # volt
# for i in range(0, len(y_corrected_volt)):
#     if (y_corrected_volt[i] <= threshold):
#         if (falling_cell==0):
#             falling_cell=i
#             print(y_corrected_volt[i-2])
#             print(y_corrected_volt[i-1])
#             print(y_corrected_volt[i])
#             print(y_corrected_volt[i+1])

#         rising_cell=i


# Constant fraction of the peak amplitude (CFD) :
CFD_ratio=4./16.
edge_value=peak_volt_value*CFD_ratio
print("Edge value = ", edge_value)

for i in range(0, len(y_corrected_volt)):
    if (y_corrected_volt[i] <= edge_value):
        if (falling_cell==0):
            falling_cell=i-1 # -1 because falling is last cell before peak

        rising_cell=i # first cell after peak


print("Falling cell = ", falling_cell, "Rising cell = ", rising_cell)

# Falling and rising offset calculation :
falling_offset=0
falling_ya=0
falling_yb=255
x=edge_value
falling_xa=y_corrected_volt[falling_cell]
falling_xb=y_corrected_volt[falling_cell+1]

print ("Thresh_value = ", x, "falling_ya = ", falling_xa, "falling_yb = ", falling_xb)

falling_offset = falling_ya + (falling_yb - falling_ya) * ((x - falling_xa) / (falling_xb - falling_xa))
print("Falling offset = ", round(falling_offset))

rising_offset=0
rising_ya=0
rising_yb=255
x=edge_value
rising_xa=y_corrected_volt[rising_cell]
rising_xb=y_corrected_volt[rising_cell+1]

print ("Thresh_value = ", x, "rising_ya = ", rising_xa, "rising_yb = ", rising_xb)

rising_offset = rising_ya + (rising_yb - rising_ya) * ((x - rising_xa) / (rising_xb - rising_xa))
print("Rising offset = ", round(rising_offset))


print("Falling offset = ", round(falling_offset), "Rising offset = ", round(rising_offset))

falling_time=(falling_cell + float(falling_offset)/256.) * tsampling
rising_time=(rising_cell + float(rising_offset)/256.) * tsampling

print("Falling time (ns) = ", falling_time, "Rising time (ns) = ", rising_time)



#sigma sur amplitude du peak en fonction du nombre de samples du calcul de la baseline
# meme chose pour la charge, soustrait N fois baseline moyenne donc somme quad

# QDC = S Ai
# Sigm

# Evolution QDC en fonction de sigma b calcule et de charge length
edge_line = []
for i in range(0, len(y_corrected_volt)):
    edge_line.append(edge_value)

# plt.plot(y_corrected_volt, 'r')
# plt.plot(falling_cell, y_corrected_volt[falling_cell], 'bo')
# plt.plot(falling_cell+1, y_corrected_volt[falling_cell+1], 'bo')
# plt.plot(rising_cell+1, y_corrected_volt[rising_cell+1], 'yo')
# plt.plot(rising_cell, y_corrected_volt[rising_cell], 'yo')
# plt.plot(edge_line, 'g')
# plt.plot(peak_position, peak_volt_value, 'go')
# plt.show()

#plt.plot(y-2048, 'r+')
# plt.plot(y_corrected, 'g+')
# plt.show()

# plt.plot(y2-2048, 'b+')
# plt.show()

# plt.plot(y3-2048, 'y')
# plt.show()

import numpy as np
import operator

data_file = np.genfromtxt(filename_1, delimiter=' ')
x, y = data_file.transpose()
# print(x)
print("Raw values   : ", y)

calib_file = np.genfromtxt(calibfile_1, delimiter=' ')
y_calib=calib_file.transpose()/16
print("Calib values : ", y_calib)

y_real_value=(y-y_calib)
print("Real values (diff) :", y_real_value)

y_test_baseline=[]
y_true_baseline=[]
for i in range (0,16):
    y_test_baseline.append(y_real_value[i]*16)
    y_true_baseline.append((y[i]-2048)*16)

print("Test baseline (corrected with y_calib) : ", y_test_baseline)
print("True baseline (based on true values) : ", y_true_baseline)

test_baseline=np.mean(y_test_baseline)
print("Test baseline = ", test_baseline)

raw_true_baseline=np.mean(y_true_baseline)
print("Raw true baseline = ", raw_true_baseline)

raw_test_charge=np.sum(y_real_value)
print("Raw test charge = ", raw_test_charge)

raw_true_charge=np.sum(y-2048)
print("Raw true charge = ", raw_true_charge)

peak_position, y_min = min(enumerate(y), key=operator.itemgetter(1))
print("Y Min         = ", y_min)
print("Peak position = ", peak_position)
