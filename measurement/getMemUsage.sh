rm massif.out.*
valgrind --tool=massif --stacks=yes --pages-as-heap=no ./$1
filename=$(ls | grep massif.out)
#ms_print $filename
python getMemUsage.py $filename