#!/bin/bash
rm udp*
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/reRoute_optimize --kind=3 --business=1 --routingOpt=true"
