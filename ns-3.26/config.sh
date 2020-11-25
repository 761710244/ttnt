single hop:
./waf --run "scratch/NodeNum --kind=3 --business=1"


muti hop:
./waf --run "scratch/MutiNodeNum --kind=3 --business=1"


routing reconstruction:
./waf --run "scratch/reRoute_optimize --kind=3 --business=1 --routingOpt=false"


linkError:
./waf --run "scratch/LinkErrorRate --kind=3 --business=1 --linkOpt=false"

partitionBer:
./waf --run "scratch/partitionBitErrorRate --kind=4 --business=1 --partitionBitErrorRate=3 --partitionOpti=false"
