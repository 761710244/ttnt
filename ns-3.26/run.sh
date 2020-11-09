#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/NodeNumAB --kind=4 --business=2"
