RawTDC : timestamp final, FPGA donne timestamp fin d'enregistrement
par contre
TDC en ns : début de la waveform (revenu au début du signal)
160Mhz clock

Timestamp aboslu TDC(ns) + autre

Raw baseline : moyenne des 16 premier samples * 16 (calculés par le firmware (pas le software), arrivé sur 2 octet
Retrouvé en ADC count : /16

Cst adc count -> Volt : 0.00061

Baseline = Raw Baseline / 16 * cst
(sign short, mot signé 16 bit)

RawPeak : 2 octets
(sign short, mot signé 16 bit)
Valeur du peak : /8
Peak : Rawpeak * cst adc->volt

RawCharge : 24 bits
bit 23 sign l'overflow donc que 23 bits significatif

RawCharge : Fabrique entier signé sur 32 bits
Q=Sum * adc->Volt * temps / R(ohm)

2 modes pour charge
calcul de la charge en soustraant(ou pas) la baseline
apr défaut: suppress baseline

Donne RIsing cell et l'offset entre Risingcell et risingcell+1
Offset 8 bits,  1/256 offset précision
 on doit *SamplingPeriod pour avoir en ns


Donne temps du signal "réel". -> CHose importante a recalculer
