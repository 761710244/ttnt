#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
rm routingSwitch.txt
./waf --run "scratch/MobilityPredict --kind=3 --business=2 --mobilityOpt=false"
