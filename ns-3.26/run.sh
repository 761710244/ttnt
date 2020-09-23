#!/bin/bash
rm udp*
rm Pid*
rm yuzhi.txt
rm delay*
rm *pcap
rm *xml
./waf --run scratch/NodeNum_reRoute50_1
