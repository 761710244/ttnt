#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
rm routingSwitch.txt
./waf --run "scratch/NodeNum --kind=3 --business=1"
