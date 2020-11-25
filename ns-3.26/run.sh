#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/partitionBitErrorRate --kind=4 --business=1 --partitionBitErrorRate=3 --partitionOpti=false"
