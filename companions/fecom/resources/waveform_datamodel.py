import sys
import math
import logging
import numpy as np
import collections
import operator
import matplotlib
import matplotlib.pyplot as plt

class waveform_recalc():
    def __init__(self):

        self.adc_count_constant = 0.00061 # to convert into volt
        self.sampling_period = 2.56e9 #GHz
        self.tsampling= 1./self.sampling_period * 1e9 # in ps
        self.resistor_value = 50 # Ohms
        self.CFD_ratio=4./16.
        self.precharge=64
        self.charge_length=992

        logging_priority=logging.INFO
        logging.basicConfig(level=logging_priority, format='%(levelname)s - %(message)s')

        # Waveform data for a signal :
        self.waveform_data=[],[] # x,y (x = sample number, y = sample_data)
        self.x_col, self.y_col = 0, 1

        # All value to recalcul :
        self.metadata = {
            'baseline_raw':0, # baseline // 2 / 8. = baseline adc <- error from Dominique
            'baseline_adc':0, # in adc fraction
            'baseline_adc_real':0, # baseline / 16. = baseline adc 'real' value
            'baseline_sample_centered':0, # around 2048
            'baseline_volt':0,
            'baseline_volt_real':0,
            'peak_position':0,
            'peak_raw':0,
            'peak_raw_real':0,
            'peak_adc':0,
            'peak_adc_real':0,
            'peak_volt':0,
            'peak_volt_real':0,
            'charge_adc':0,
            'charge_pc':0,
            'edge_to_cross':0,
            'falling_cell':0,
            'falling_offset':0,
            'falling_time':0,
            'rising_cell':0,
            'rising_offset':0,
            'rising_time':0
        }

        self.baseline_mod_8_samples=[]
        self.sigma_baseline_mod_8_samples=[]

    def parse_waveform(self, waveform_data_line):
        sample_number=0
        for sample in waveform_data_line.split():
            self.waveform_data[self.x_col].append(sample_number)
            self.waveform_data[self.y_col].append(int(sample))
            sample_number+=1

    def recalcul_from_waveform(self):
        # Baseline modulo 8 samples :
        for i in range (1, 32):
            baseline_X_samples_calc=[]
            for j in range (0, 8*i):
                baseline_X_samples_calc.append((self.waveform_data[self.y_col][j]-2048)*16)
            baseline_X_samples_calc_mean = np.mean(baseline_X_samples_calc)
            sigma_baseline_X_samples_calc_mean=math.sqrt(abs(baseline_X_samples_calc_mean))
            self.baseline_mod_8_samples.append(baseline_X_samples_calc_mean)
            self.sigma_baseline_mod_8_samples.append(sigma_baseline_X_samples_calc_mean)
        logging.debug("Baseline value modulo 8 samples : %s", self.baseline_mod_8_samples)
        logging.debug("Sigma baseline value modulo 8 samples : %s", self.sigma_baseline_mod_8_samples)

        # Rescale baseline modulo 8 samples to value at 16 sample (percentage) :




        # Baseline calculation :
        self.metadata['baseline_raw'] = self.baseline_mod_8_samples[1] # [1] is for 16 samples
        self.metadata['baseline_adc'] = self.metadata['baseline_raw'] // 2 / 8.
        self.metadata['baseline_adc_real'] = self.metadata['baseline_raw'] / 16.
        self.metadata['baseline_volt'] = self.metadata['baseline_adc'] * self.adc_count_constant
        self.metadata['baseline_volt_real'] = self.metadata['baseline_adc_real'] * self.adc_count_constant
        self.metadata['baseline_sample_centered'] = 2048 + self.metadata['baseline_adc']

        # Peak calculation :
        self.metadata['peak_position'], peak_sample_value = min(enumerate(self.waveform_data[self.y_col]), key=operator.itemgetter(1))
        self.metadata['peak_adc'] = float("{0:.4f}".format(peak_sample_value - self.metadata['baseline_sample_centered']))
        self.metadata['peak_raw'] = self.metadata['peak_adc'] * 8.
        self.metadata['peak_volt'] = self.metadata['peak_adc'] * self.adc_count_constant
        self.metadata['peak_adc_real'] = float("{0:.4f}".format(peak_sample_value - (2048 + self.metadata['baseline_adc_real'])))
        self.metadata['peak_raw_real'] = self.metadata['peak_adc_real'] * 8.
        self.metadata['peak_volt_real'] = self.metadata['peak_adc_real'] * self.adc_count_constant

        # Charge cannot be recalculate because in the board it continues after the sample '1024'
        # It must be : [peak_position - 64; (peak_position - 64 + charge_length) | (1024)]
        # Approximation : begin at sample 16 (wo baseline) and stop at 16 + charge length
        charge_lower_bound = 16
        charge_upper_bound = charge_lower_bound + self.charge_length

        # Real charge bounds : (see in RunSettings precharge and charge length)
        charge_lower_bound_real = self.metadata['peak_position'] - 64
        charge_upper_bound_real = charge_lower_bound_real + self.charge_length

        charge_samples = []
        falling_cell_already_crossed = False
        rising_cell_already_crossed = False
        self.metadata['edge_to_cross'] = self.metadata['peak_raw'] / 8. * self.CFD_ratio
        for sample in range (charge_lower_bound, charge_upper_bound):
            sample_value_adc=float("{0:.4f}".format(self.waveform_data[self.y_col][sample]- self.metadata['baseline_sample_centered']))
            charge_samples.append(sample_value_adc)
            if (falling_cell_already_crossed == False and sample_value_adc <= self.metadata['edge_to_cross']):
                self.metadata['falling_cell']= sample - 1
                falling_cell_already_crossed=True
            if (rising_cell_already_crossed == False and falling_cell_already_crossed == True and sample_value_adc >= self.metadata['edge_to_cross']):
                self.metadata['rising_cell']= sample - 1
                rising_cell_already_crossed=True
            # if (falling_cell_already_crossed == False and sample_value_adc <= self.metadata['edge_to_cross']):
            #     self.metadata['falling_cell']= sample - 1
            #     falling_cell_already_crossed=True
            # if (rising_cell_already_crossed == False and falling_cell_already_crossed == True and sample_value_adc >= self.metadata['edge_to_cross']):
            #     self.metadata['rising_cell']= sample - 1
            #     rising_cell_already_crossed=True

        self.metadata['charge_adc'] = np.sum(charge_samples)
        self.metadata['charge_pc'] = float("{0:.6f}".format(self.metadata['charge_adc'] * self.adc_count_constant * self.tsampling / self.resistor_value))

        falling_cell_sample_value = self.waveform_data[self.y_col][self.metadata['falling_cell']] - self.metadata['baseline_sample_centered']
        falling_cell_sample_value_plus_one = self.waveform_data[self.y_col][self.metadata['falling_cell'] + 1] - self.metadata['baseline_sample_centered']
        rising_cell_sample_value = self.waveform_data[self.y_col][self.metadata['rising_cell']] - self.metadata['baseline_sample_centered']
        rising_cell_sample_value_plus_one = self.waveform_data[self.y_col][self.metadata['rising_cell'] + 1] - self.metadata['baseline_sample_centered']

        # Linear interpolation for rising and falling offsets :
        self.metadata['falling_offset'] = self.offset_linear_interpol(falling_cell_sample_value, falling_cell_sample_value_plus_one)
        self.metadata['rising_offset'] = self.offset_linear_interpol(rising_cell_sample_value, rising_cell_sample_value_plus_one)

        self.metadata['falling_time'] = (self.metadata['falling_cell'] + self.metadata['falling_offset'] / 256.) * self.tsampling
        self.metadata['rising_time'] = (self.metadata['rising_cell'] + self.metadata['rising_offset'] / 256.) * self.tsampling





    def offset_linear_interpol(self, xa, xb):
        ya = 0
        yb = 255
        x = self.metadata['edge_to_cross']
        if (xb - xa) != 0:
            return round(ya + (yb - ya) * ((x - xa) / (xb - xa)))
        else:
            return 0

    def tree_dump(self):
        print("")
        logging.debug("Tree dump recalculated metadata from waveform :")
        # logging.debug(collections.OrderedDict(sorted(self.metadata.items(), key=lambda t: t[0])))

        for item in sorted(self.metadata.items()):
            logging.debug(item)

        #logging.debug(self.metadata)

    def print_raw_waveform(self):
        print(self.waveform_data[self.x_col], self.waveform_data[self.y_col])


class waveform_histos():
    def __init__(self):
        # General histos :
        # Signal distrib :
        self.baseline_distrib = []
        self.peak_raw_distrib = []
        self.peak_adc_distrib = []
        self.charge_adc_distrib = []
        self.charge_pc_distrib = []
        self.falling_cell_distrib = []
        self.falling_offset_distrib = []
        self.falling_time_distrib = []
        self.rising_cell_distrib = []
        self.rising_offset_distrib = []
        self.rising_time_distrib = []

        # Differences distrib :
        self.diff_baseline = []
        self.diff_peak_adc = []
        self.diff_charge_adc = []
        self.diff_falling_cell = []
        self.diff_falling_offset = []
        self.diff_falling_time = []
        self.diff_rising_cell = []
        self.diff_rising_offset = []
        self.diff_rising_time = []

        # Analysis and optimization :
        self.mean_baseline_mod_8_samples=[[],[ [] ]]
        self.mean_sigma_baseline_mod_8_samples=[[],[ [] ]]
        for i in range (1,32):
            self.mean_baseline_mod_8_samples[0].append(i)
            self.mean_sigma_baseline_mod_8_samples[0].append(i)


    def add_signal_to_histos(self, metadata_jihane, metadata_recalc):
        logging.debug("Tree dump Jihane's metadata")
        for item in sorted(metadata_jihane.items()):
            logging.debug('%s', item)
        logging.debug("")
        logging.debug("Tree dump recalculated metadata")
        for item in sorted(metadata_recalc.items()):
            logging.debug('%s', item)
        logging.debug("")

        # Append signal distribution (from jihane values) :
        self.baseline_distrib.append(metadata_jihane['baseline_raw'])
        self.peak_raw_distrib.append(metadata_jihane['peak_raw'])
        self.peak_adc_distrib.append(metadata_jihane['peak_adc'])
        self.charge_adc_distrib.append(metadata_jihane['charge_adc'])
        self.charge_pc_distrib.append(metadata_jihane['charge_pc'])
        self.falling_cell_distrib.append(metadata_jihane['falling_cell'])
        self.falling_offset_distrib.append(metadata_jihane['falling_offset'])
        self.falling_time_distrib.append(metadata_jihane['falling_time'])
        self.rising_cell_distrib.append(metadata_jihane['rising_cell'])
        self.rising_offset_distrib.append(metadata_jihane['rising_offset'])
        self.rising_time_distrib.append(metadata_jihane['rising_time'])

        # Append differences to histos :
        self.diff_baseline.append(metadata_jihane['baseline_raw'] - metadata_recalc['baseline_raw'])
        self.diff_peak_adc.append(metadata_jihane['peak_adc'] - metadata_recalc['peak_adc'])
        self.diff_charge_adc.append(metadata_jihane['charge_adc'] - metadata_recalc['charge_adc'])
        self.diff_falling_cell.append(metadata_jihane['falling_cell'] - metadata_recalc['falling_cell'])
        self.diff_falling_offset.append(metadata_jihane['falling_offset'] - metadata_recalc['falling_offset'])
        self.diff_falling_time.append(metadata_jihane['falling_time'] - metadata_recalc['falling_time'])
        self.diff_rising_cell.append(metadata_jihane['rising_cell'] - metadata_recalc['rising_cell'])
        self.diff_rising_offset.append(metadata_jihane['rising_offset'] - metadata_recalc['rising_offset'])
        self.diff_rising_time.append(metadata_jihane['rising_time'] - metadata_recalc['rising_time'])

        # Don't forget to convert offset (1/256 ns into ns or ps)


    def print_histos(self):
        # First window : raw signal distribution (baseline, peak, charge ...)
        f, ax = plt.subplots(2, 5, figsize=(18,18))
        n, bins, patches = ax[0,0].hist(self.baseline_distrib, 50) #, normed=1)facecolor='green', alpha=0.75)
        ax[0, 0].set_title('Baseline distribution')

        n, bins, patches = ax[1,0].hist(self.charge_adc_distrib, 100)
        ax[1, 0].set_title('Charge adc distribution')

        n, bins, patches = ax[0,1].hist(self.peak_adc_distrib, 50)
        ax[0, 1].set_title('Peak adc distribution')

        n, bins, patches = ax[0,2].hist(self.falling_cell_distrib, 50)
        ax[0, 2].set_title('Falling cell distribution')

        n, bins, patches = ax[0,3].hist(self.falling_offset_distrib, 50)
        ax[0, 3].set_title('Falling offset distribution')

        n, bins, patches = ax[0,4].hist(self.falling_time_distrib, 50)
        ax[0, 4].set_title('Falling time distribution')

        n, bins, patches = ax[1,2].hist(self.rising_cell_distrib, 50)
        ax[1, 2].set_title('Rising cell distribution')

        n, bins, patches = ax[1,3].hist(self.rising_offset_distrib, 50)
        ax[1, 3].set_title('Rising offset distribution')

        n, bins, patches = ax[1,4].hist(self.rising_time_distrib, 50)
        ax[1, 4].set_title('Rising time distribution')


        f1, ax1 = plt.subplots(2, 5, figsize=(18,18))
        n, bins, patches = ax1[0,0].hist(self.diff_baseline, 50) #, normed=1)facecolor='green', alpha=0.75)
        ax1[0, 0].set_title('Baseline differences')

        n, bins, patches = ax1[1,0].hist(self.diff_charge_adc, 50)
        ax1[1, 0].set_title('Charge adc differences')

        n, bins, patches = ax1[0,1].hist(self.diff_peak_adc, 50)
        ax1[0, 1].set_title('Peak adc differences')

        n, bins, patches = ax1[0,2].hist(self.diff_falling_cell, 50)
        ax1[0, 2].set_title('Falling cell differences')

        n, bins, patches = ax1[0,3].hist(self.diff_falling_offset, 50)
        ax1[0, 3].set_title('Falling offset differences')

        n, bins, patches = ax1[0,4].hist(self.diff_falling_time, 100)
        ax1[0, 4].set_title('Falling time differences')

        n, bins, patches = ax1[1,2].hist(self.diff_rising_cell, 50)
        ax1[1, 2].set_title('Rising cell differences')

        n, bins, patches = ax1[1,3].hist(self.diff_rising_offset, 50)
        ax1[1, 3].set_title('Rising offset differences')

        n, bins, patches = ax1[1,4].hist(self.diff_rising_time, 100)
        ax1[1, 4].set_title('Rising time differences')


        plt.show()
