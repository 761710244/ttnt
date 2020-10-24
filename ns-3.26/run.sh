#!/bin/bash
rm udp*
rm Pid*
rm yuzhi.txt
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run scratch/NodeNumAB
