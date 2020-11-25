#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/MobilityPredict --kind=4 --business=1 --mobilityOpt=true"
