file_name = ARG1
# DEBUG print
#print "File name: ", file_name
#print "arg0 :", ARG0
#print "arg1 :", ARG1
#print "arg2 :", ARG2
#print "arg3 :", ARG3
#print "arg4 :", ARG4
#print "arg5 :", ARG5

set title "ADC AVR Mega 2560"
set xlabel "Sample"
set ylabel "ADC Value"
set grid
set autoscale
set datafile separator whitespace
set style data linespoints
# estraggo numero di colonne dagli argomenti
num_columns = ARG2
if (num_columns > 1) {
    set multiplot layout num_columns,1 title "Plots for Each Column"
}
clear

samples_to_display = 50
total_samples = system(sprintf("wc -l < %s", file_name)) + 0  
min_sample = (total_samples > samples_to_display) ? total_samples - samples_to_display : 0
max_sample = total_samples
set xrange [min_sample:max_sample]
set yrange [40:80]

# plotto i canali
do for [i=3:2+num_columns] {
    col_num = int(word(ARGV[i], 1))
    plot file_name using 1:col_num+2 title sprintf('Column %d', col_num) with lines
}
pause 0.5
unset multiplot
reread