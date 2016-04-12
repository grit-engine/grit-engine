stamp=`date +%Yy%mm%dd-%Hh%M`
output="result-$stamp.dat"
#
export TOOLPATH=/opt/sunstudio
export PATH=/opt/sunstudio/sunstudioceres/bin/:$PATH
COMPILER=sunCC
COMPILER_V=5.10
COMPILER_DIR=/opt/sunstudio/sunstudioceres/bin/
#
cd ..
# NOTE -msse3 --> only for CoreDuo Processors
make clean; make OPTIMIZATION='-O5 ' COMPILER=$COMPILER COMPILER_V=$COMPILER_V
cd benchmark
../lexer-lc many-tiny-tokens.c            > $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

cd ..
make clean; make OPTIMIZATION='-O5 -xspace' COMPILER=$COMPILER COMPILER_V=$COMPILER_V
cd benchmark
../lexer-lc many-tiny-tokens.c           >> $output
../lexer-lc single-large-token.c         >> $output
../lexer-lc linux-2.6.22.17-kernel-dir.c >> $output

../lexer many-tiny-tokens.c           >> $output
../lexer single-large-token.c         >> $output
../lexer linux-2.6.22.17-kernel-dir.c >> $output

