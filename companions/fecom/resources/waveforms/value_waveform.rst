Comparaison valeurs waveform
============================
Toutes les valeurs sont corrigés des piédestaux au 1er ordre car interne au FPGA
CaloPedestal file : résidus des piédestaux, jamais utilisé par jihane et dominique dans leur calcul

raw to volt : * adc_constant = 0.61

Baseline
--------
baseline_raw : Valeur moyenne des 16 premiers samples - 2048


baseline_recalculated : Valeur moyenne des 16 premiers samples et gardé autour de 2048
baseline_raw = (baseline_recalculated-2048)*16 (pour avoir raw value car codé sur 16 bits)

Peak
----
Peak_raw : valeur extrème du peak corrigé de la baseline
peak_adc_fraction_jihane = Peak_raw / 8. (valeur non entière d'ADC)

Pour le recalcul avec la waveform : peak_value_min à peak_cell corrigé de la baseline

peak_recalculated_adc=float("{0:.4f}".format(peak_value-baseline_recalculated))
peak_recalculated_raw=peak_recalculated_adc*8.
Q : si plusieurs valeurs min de peak dans la waveform, on prends la 1ere fois que ca atteinds le minimum ?

Charge
------
Dynamic charge:
Charge length = 992
integration :[Peak_cell - 64;  Peak_cell - 64 + Charge_length]

Force baseline = 0 -> false donc baseline retirée automatiquement pour le calcul de la charge
My calcul : Sum(waveform_data[y_col][j]-baseline_recalculated)

(waveform_data[y_col][j] = 1 sample de la waveform - baseline moyene autour de 2048 (ex: 1825 - 2048.5625)

Timing and offset
-----------------
Edge to cross calculation :
edge_to_cross_adc_jihane(supposition)=peak_raw /8. * CFD_ratio (charge calcul with peak adc_fraction or in volt ?)
Falling cell : last cell before crossing edge
Rising cell : first cell before or after crossing edge ?

Offset calculation : interpolation linéaire entre falling_cell et falling_cell + 1 (valeurs en ADC ou en raw ?)

Falling / rising offset interpolation
ya=0
yb=255
falling_xa=waveform[falling_cell]-baseline_recalculated # in ADC
falling_ya=waveform[falling_cell+1]-baseline_recalculated # in ADC
x1=edge_to_cross
falling_offset_recalculated=falling_ya + (falling_yb - falling_ya) * ((x1 - falling_xa) / (falling_xb - falling_xa))

Literal calcul (works with Jihane values)
rising_time_jihane =float("{0:.6f}".format((rising_cell + rising_offset/256.) * tsampling))
falling_time_jihane =float("{0:.6f}".format((falling_cell + falling_offset/256.) * tsampling))

Waveform examples
-----------------

Hit number : 2

============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   9            9
Peak_cell      ?            581
Peak_raw       -1572        -1572.5
Peak_adc       -196.5       -196.5625
Peak_value_min ?            1852
Charge_raw     -13220       -13142.1875
Falling_cell   567          567
Falling_offset 248          246.035
Falling_time   221.862793   221.859794855
Rising_cell    636          636
Rising_offset  88           90.644
Rising_time    248.571777   248.575812578
============== ============ =============

baseline : [2051, 2049, 2048, 2051, 2050, 2048, 2050, 2047, 2048, 2048, 2048, 2048, 2044, 2049, 2050, 2048]
mean_recalc = 2048.5625
raw = (2048.5625-2048) * 16 = 9


Hit number : 3

============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   -3            -3
Peak_cell      ?            593
Peak_raw       -1670        -1670.5
Peak_adc       -208.75      -208.8125
Peak_value_min ?            1852
Charge_raw     -14531       -14715.1875
Falling_cell   577          577
Falling_offset 165          162.96
Falling_time   225.642395   225.639
Rising_cell    646          646
Rising_offset  200          136.796875
Rising_time    252.648926   252.55
============== ============ =============



Hit number : 4

============== ============ ============
Paramètre      Jihane       Moi
-------------- ------------ ------------
Baseline_raw   -5           -5
Peak_cell      ?            602
Peak_raw       -1829        -1829.5
Peak_adc       -228.625     -228.6875
Peak_value_min ?            1819
Charge_raw     -15585       -15670.125
Falling_cell   580          580
Falling_offset 51           52.26
Falling_time   226.64032    226.6422
Rising_cell    654          654
Rising_offset  316          213.828     ??? why 316 offset Jihane ?
Rising_time    255.950928   255.7950
============== ============ ============



Hit number : 6

Bon hit (le plus proche niveau valeur)

============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   -8           -8
Peak_cell      ?            599
Peak_raw       -2084        -2084
Peak_adc       -260.5       -260.5
Peak_value_min ?            1787
Charge_raw     -19277       -19332.5
Falling_cell   583          583
Falling_offset 74           73.7 (-> 74)
Falling_time   227.84729    227.8468
Rising_cell    662          662
Rising_offset  24           23.90 (-> 24)
Rising_time    258.6304     258.6302
============== ============ =============


Hit number : 8


============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   -6           -6
Peak_cell      ?            598
Peak_raw       -4053        -4053
Peak_adc       -506.625     -506.625
Peak_value_min ?            1541
Charge_raw     -39485       -39699.25
Falling_cell   580          580
Falling_offset 31           29.37
Falling_time   226.609802   226.607
Rising_cell    659          659
Rising_offset  126          167.34
Rising_time    257.614136   257.677
============== ============ =============


Hit number : 12

Rising offset cross plusieurs fois

============== ============ ==========
Paramètre      Jihane       Moi
-------------- ------------ ----------
Baseline_raw   8            8
Peak_cell      ?            578
Peak_raw       -756         -756
Peak_adc       -94.5        -94.5
Peak_value_min ?            1954
Charge_raw     -7612        -7209
Falling_cell   563          563
Falling_offset 132          150.26
Falling_time   220.12329    220.15116
Rising_cell    638          638
Rising_offset  112          223.125
Rising_time    249.389648   249.559
============== ============ ==========


Hit number : 31

Rising cell calcul pas bon

============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   11           11
Peak_cell      ?            590
Peak_raw       -1013        -1013.5
Peak_adc       -126.625     -126.6875
Peak_value_min ?            1922
Charge_raw     -9481        -9522.375
Falling_cell   573          573
Falling_offset 161          158.87
Falling_time   224.0737     224.07055
Rising_cell    648          655
Rising_offset  190          171.328
Rising_time    253.4149     256.1208   (<- diff > 1 sec)
============== ============ =============



Hit number : 71

Falling cell calcul pas bon

============== ============ ===============
Paramètre      Jihane       Moi
-------------- ------------ ---------------
Baseline_raw   -5           -5
Peak_cell      ?            593
Peak_raw       -1525        -1525.5
Peak_adc       -190.625     -190.6875
Peak_value_min ?            1857
Charge_raw     -13166       -13465.3
Falling_cell   579          578        (!!)
Falling_offset 1            254.60     (!!)
Falling_time   226.173      226.1697   (proche au final)
Rising_cell    644          645
Rising_offset  190          1.99
Rising_time    251.8524     251.9561
============== ============ ===============



Hit number : 168

Rising cell calcul pas bon


============== ============ =============
Paramètre      Jihane       Moi
-------------- ------------ -------------
Baseline_raw   2            2
Peak_cell      ?            580
Peak_raw       -1377        -1377
Peak_adc       -172.125     -172.125
Peak_value_min ?            1876
Charge_raw     -11978       -12049.5
Falling_cell   566          566
Falling_offset 84           90.55
Falling_time   221.221924   221.2319244
Rising_cell    633          637
Rising_offset  198          11.95
Rising_time    247.567749   248.8463   (<- diff > 1 sec)
============== ============ =============
