sh auto.sh
./benchmark/benchmark 1024 8192 4
cat *.log > trace
sort -n -k 3 trace > sorted_trace
./benchmark/validate 2048 8192 < sorted_trace

