import re 
import os
import sys 
import subprocess
import numpy as np
import time
import psutil
import signal

print( "parsec3.py imported" ) 

def init( pack_ , app_ , thr_ ) :
    global pack, app, thr, period, quota, cpuset, toolpath, dockername 
    toolpath = "/root/SIG/sds_3Dtest/tolerate_codes/"
    dockername = "illusprog"
    pack = pack_ 
    app = app_ 
    thr = thr_
    period = 100000 
    quota = thr * period
    cpuset = "0-" + str(thr-1)

def run( rounds = 5 , cutprefix = 1 , cutsuffix = 1 ):
    print( "running %s/%s (%d threads) @ period %d quota %d%% cpuset=%s"%( pack , app , thr , period , quota/period*100 , cpuset ) , flush = True )
    IPCs = []
    for i in range( rounds ) :
        print( pack , app , "round %d of %d: "%(i+1,rounds) , flush = True , end = "" )
        subprog = subprocess.Popen( shell = False , args = ["docker","run","--name=" + dockername, "--cpuset-cpus="+cpuset,"--cpu-period="+str(period),"--cpu-quota="+str(quota),"spirals/parsec-3.0","-S",pack,"-a","run","-p",app,"-i","native","-n",str(thr)] , stdout = subprocess.PIPE , stderr = subprocess.DEVNULL , encoding="utf-8" , universal_newlines="" ) 
        runningFlag = False 
        while runningFlag == False:
            time.sleep( 1 )
            cpususe = psutil.cpu_percent( interval = 0.5 , percpu = True ) 
            sumappcpu = sum( cpususe[0:thr] )
            if sumappcpu > 1.0 * thr / 2 * 100 :
                runningFlag = True 
        temp = subprocess.getstatusoutput( toolpath + "find_and_perf_cgroup.sh" )[1]
        perfoutput = ( ( temp ) ).split( "\n" )[2:]
        cycles =  float( re.search( r"\d+" , perfoutput[4] ).group( 0 ) )
        insts = float( re.search( r"\d+" , perfoutput[5] ).group( 0 ) )
        IPC = round( float( 1.0 * insts / cycles ) , 4 )
        print( "IPC =" , IPC , flush = True )
        subprog.kill()
        subprocess.getstatusoutput("docker container stop " + dockername + " > /dev/null" )
        subprocess.getstatusoutput("docker container rm " + dockername + " > /dev/null" )
        IPCs.append( IPC )
        time.sleep( 1 )
    IPCs = sorted( IPCs )
    print( " - except value: ", end = "" )
    for i in range( 0 , cutprefix ):
        print( IPCs[i] , end = " " )
    for i in range( rounds - cutsuffix , rounds ):
        print( IPCs[i] , end = " " )
    print( "" )
    res = round( np.mean( IPCs[0+cutprefix:rounds-cutsuffix] ) , 4 )
    print( "IPC =" , res , flush = True )
    return res 

def cleaner(  ):
    print( "docker stop $(docker ps -aq)" )
    subprocess.getstatusoutput("docker stop $(docker ps -aq)")
    print( "docker rm $(docker ps -aq)" )
    subprocess.getstatusoutput("docker rm $(docker ps -aq)")