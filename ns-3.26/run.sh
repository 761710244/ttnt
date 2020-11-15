#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/reRoute_optimize --kind=4 --business=1 --routingOpt=true"
