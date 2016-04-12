cp data.txt data-1-kB.txt
# from 1 to 64KB create files for each KB
j=1; for i in `seq 2 64`; do cat data.txt data-${j}-kB.txt > data-${i}-kB.txt; j=$i; done

# from 64KB to 1MB create files of 64KB increment
j=1; for i in `seq 2 16`; do cat data-64-kB.txt data-$((j * 64))-kB.txt > data-$((i * 64))-kB.txt; j=$i; done

# from 1MB to 32MB create files of 1MB increment
j=1; for i in `seq 2 32`; do cat data-1024-kB.txt data-$((j * 1024))-kB.txt > data-$((i * 1024))-kB.txt; j=$i; done

touch EXAMPLE-DB-CREATED
