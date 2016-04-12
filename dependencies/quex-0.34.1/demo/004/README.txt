
This program can be called in three modes:

(1) BENCHMARK: 

    > lexer  file-of-size-1024kB.txt  1024

    Specify file name and size on the command line. The benchmark
    may repeat the analysis of this file multiple times in order to
    converge to a stable result. At the end, the output reports
    the number of repetitions. 

    NOTE: The benchmark measures also a 'measuring overhead' which
    has to be subtracted. The overhead can be determined by
    using the lexer in the following modes.

(2) COUNT TOKENS: 

    > lexer  filename.txt

    Counts the number of tokens found in the file 'filename.txt'. 
    The number of tokens is important to determine the measurement
    overhead.

(3) REFERENCE: 

    > lexer  file-of-size-1024kB.txt  1024  NumberOfTokens RepetitionN

    Computes the same timings as the BENCHMARK, but **does not** do
    any real analysis. Use the data to determine the 'real' values
    computed by the benchmark. 


