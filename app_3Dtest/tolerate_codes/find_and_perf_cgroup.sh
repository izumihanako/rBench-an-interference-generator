#!/bin/bash
dockerid=$(docker ps | grep "parsec\|splash" | cut -b 1-10)
echo $dockerid
cgroupname=$(find /sys/fs/cgroup/ -name \*$dockerid\* | head -1 | grep "system.*" -o )
echo $cgroupname
perf stat --time 10000 --no-big-num -e task-clock -e cycles -e instructions -G "$cgroupname"