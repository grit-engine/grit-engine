stamp=`date +%Yy%mm%dd-%Hh%M`
output="result-$stamp.dat"
#
COMPILER=icpc
COMPILER_V=10.1.018
COMPILER_DIR=/opt/intel/cc/10.1.018/bin
#
cd ..
source $COMPILER_DIR/iccvars.sh
# NOTE -msse3 --> only for CoreDuo Processors
make clean; make OPTIMIZATION='-O3 -msse3 -ipo -no-prec-div -static' COMPILER=$COMPILER COMPILER_V=$COMPILER_V
cd benchmark
../lexer-lc many-tiny-tokens.c            > $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

cd ..
make clean; make OPTIMIZATION='-O3 -msse3 -ipo -no-prec-div -static' COMPILER=$COMPILER COMPILER_V=$COMPILER_V
cd benchmark
../lexer-lc many-tiny-tokens.c           >> $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

