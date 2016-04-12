stamp=`date +%Yy%mm%dd-%Hh%M`
if [[ $1 != "HWUT-TEST" ]]; then
    output="result-$stamp.dat"
else
    output="tmp.dat"
    rm -f tmp.dat
    cd ..
    make clean >& /dev/null; 
    make lexer OPTIMIZATION=' ' >& /dev/null
    cd benchmark
    ../lexer linux-2.6.22.17-kernel-dir.c > $output
    make clean >& /dev/null; 
    exit
fi
cd ..
make clean; make OPTIMIZATION='-O3' >& /dev/null
cd benchmark
../lexer-lc many-tiny-tokens.c            > $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

cd ..
make clean; make OPTIMIZATION='-Os' >& /dev/null
cd benchmark
../lexer-lc many-tiny-tokens.c           >> $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

