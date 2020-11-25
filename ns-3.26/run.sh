#!/bin/bash
rm Pid*
rm *pcap
rm *xml
rm delay*
rm throu*
rm pid*
./waf --run "scratch/MutiNodeNum --kind=4 --business=1"
