sh auto.sh
./benchmark/benchmark 8000 8192 4
sleep 2
cat *.log > trace
sort -n -k 3 trace > sorted_trace
sleep 2
./benchmark/validate 16000 8192 < sorted_trace
