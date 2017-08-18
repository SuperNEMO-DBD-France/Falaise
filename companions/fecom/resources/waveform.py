import numpy as np
import operator

data_file = np.genfromtxt('waveform.txt', delimiter=' ')
x, y = data_file.transpose()
# print(x)
print("Raw values   : ", y)

calib_file = np.genfromtxt('calib.txt', delimiter=' ')
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
