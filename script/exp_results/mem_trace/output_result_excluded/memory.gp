myfont = "Arial Bold, 17"

set xlabel "Selectivity (%)" font myfont offset -1, -0.5 
set ylabel "Peak memory consumption (MB)"  font myfont offset -1
set terminal pdf dashed
set title ""
set pointsize 1.5

my_purple = "#8064A2"
my_green = "#9BBB59"
my_red = "#C0504D"
my_blue = "#4E81B3"
my_aqua = "#4BACB6"

set xrange [0.0003:50]
set logscale x 10

set xtics (0.001, 0.01, 0.1, 1, 10) font myfont
set ytics font myfont

set style data linespoints
set datafile separator ","

set key vertical Left center left reverse font myfont spacing 3 samplen 2 width 2.4 opaque
show key

show mytics 
set output "memory_consumption.pdf";

plot "4var.csv" using 1:($2/1024/1024) title col lt 5 linecolor rgbcolor my_red      pt 9 lw 6, \
     "4var.csv" using 1:($3/1024/1024) title col lt 2 linecolor rgbcolor my_green    pt 6 lw 6, \
     "4var.csv" using 1:($4/1024/1024) title col lt 6 linecolor rgbcolor my_aqua     pt 10 lw 6, \
     "4var.csv" using 1:($5/1024/1024) title col lt 2 linecolor rgbcolor my_purple   pt 4 lw 6, \
     "4var.csv" using 1:($6/1024/1024) title col lt 3 linecolor rgbcolor my_blue     pt 2 lw 6;

plot "3var.csv" using 1:($2/1024/1024) title col lt 5 linecolor rgbcolor my_red      pt 9 lw 6, \
     "3var.csv" using 1:($3/1024/1024) title col lt 2 linecolor rgbcolor my_green    pt 6 lw 6, \
     "3var.csv" using 1:($4/1024/1024) title col lt 6 linecolor rgbcolor my_aqua     pt 10 lw 6, \
     "3var.csv" using 1:($5/1024/1024) title col lt 2 linecolor rgbcolor my_purple   pt 4 lw 6, \
     "3var.csv" using 1:($6/1024/1024) title col lt 3 linecolor rgbcolor my_blue     pt 2 lw 6;

set key vertical Left top left reverse font myfont spacing 3 samplen 2 width 2.4 opaque
plot "2var.csv" using 1:($2/1024/1024) title col lt 5 linecolor rgbcolor my_red      pt 9 lw 6, \
     "2var.csv" using 1:($3/1024/1024) title col lt 2 linecolor rgbcolor my_green    pt 6 lw 6, \
     "2var.csv" using 1:($4/1024/1024) title col lt 6 linecolor rgbcolor my_aqua     pt 10 lw 6, \
     "2var.csv" using 1:($5/1024/1024) title col lt 2 linecolor rgbcolor my_purple   pt 4 lw 6, \
     "2var.csv" using 1:($6/1024/1024) title col lt 3 linecolor rgbcolor my_blue     pt 2 lw 6;

