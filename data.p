file_name = ARG1
set title "ADC AVR Mega 2560"
set xlabel "Sample"
set ylabel "ADC Value"
set grid
set datafile separator whitespace
set style data linespoints
clear
plot file_name using 1:2 title "Channel 0" with lines
pause 1
reread