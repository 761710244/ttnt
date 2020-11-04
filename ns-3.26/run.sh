#!/bin/bash
rm udp*
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/NodeNumAB --kind=1 --business=3"
