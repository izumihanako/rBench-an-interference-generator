print( "---------------- program header ----------------\n" ) 

import os
import signal
import sys
import time 
import subprocess 
sys.path.append( ".." ) 
from parsec3 import init as appinit
from parsec3 import run as apprun 
from parsec3 import cleaner as appcleaner
pack = "parsec" ; app = "ferret" ; appthr = 8 ; appthrstr = "0-" + str(appthr-1)
appinit( pack_ = pack , app_ = app , thr_ = appthr )

def kill_with_name( name ):
    subprocess.getstatusoutput("ps -ef | grep " + name + " | awk '{print $2}' | xargs kill -9 > /dev/null")
    print( "%s killed"%(name) )

def cleaner(  signum , frame  ):
    kill_with_name( "rbench" )
    appcleaner()
    exit() 

ways    = [ "0x001","0x003","0x007","0x00f","0x01f","0x03f","0x07f","0x0ff","0x1ff","0x3ff","0x7ff" ]
mbgears = [ "10","20","30","40","50","60","90","100" ]
def set_pqos_to_default():
    subprocess.getstatusoutput( "pqos -R" )
    return

def pqos_set_to( res , setid , gear ):
    subprocess.getstatusoutput( "pqos -e " + "\"" + res + ":" + str(setid) + "=" + gear + "\"")
    return

def pqos_allocate( res , setid , cores  ):
    subprocess.getstatusoutput( "pqos -a " + "\"" + res + ":" + str(setid) + "=" + cores + "\"")
    return


signal.signal( signal.SIGINT , cleaner )
os.system("date")


rbenchPath   = "/root/SIG/sds_3Dtest/rbench.exe"
llcstrengths = [ "10","20","30","40","50","60","70","80","90","100" ]  # percent per instance
mem_bw_mbs   = [ "1000" , "2000" , "3000" , "4000" , "5000" , "6000" , "7000" , "8000" , "9000" ]  # mem-bandwidth per instance
def app_3D_by_intf() :
    print( "\n---------------- app 3D by intf ----------------\n")
    print( "default setting, baseline start:" )
    set_pqos_to_default()
    baseIPC = apprun()
    print( "the base IPC is" , baseIPC , "\n" , flush = True )

    intf_llc_thr = 4 ; intf_mbw_thr = 8
    intf_llc_thrstr = "28-31" ; intf_mbw_thrstr = "32-39"
    # limit the mbw stressor's llc usage
    intf_lmem_pqos_id = 2 ; intf_lllc_pqos_id = 3
    pqos_set_to( "llc" , intf_lmem_pqos_id , ways[0] ) ; pqos_set_to( "mba" , intf_lmem_pqos_id , mbgears[7] ) 
    pqos_allocate( "core" , intf_lmem_pqos_id , intf_mbw_thrstr )
    print( "pqos COS%d: llc=%s , mba = %s -> core %s"%( intf_lmem_pqos_id , ways[0] , mbgears[7] , intf_mbw_thrstr ) , flush = True )

    pqos_set_to( "llc" , intf_lllc_pqos_id , ways[10] ) ; pqos_set_to( "mba" , intf_lllc_pqos_id , mbgears[0] ) 
    pqos_allocate( "core" , intf_lllc_pqos_id , intf_llc_thrstr )
    print( "pqos COS%d: llc=%s , mba = %s -> core %s"%( intf_lllc_pqos_id , ways[10] , mbgears[0] , intf_mbw_thrstr ) , flush = True )

    ress = []
    for llcstrength in llcstrengths :
        llcres = []
        for mem_bw_mb in mem_bw_mbs :
            llcprog = subprocess.Popen( args = [ "taskset" , "-c" , intf_llc_thrstr ,
                                                  rbenchPath , "-r" , "cacheL3" , "-s" , llcstrength , "-n" , str(intf_llc_thr) ] , 
                      shell = False , stdout = None , stderr = sys.stderr , encoding="utf-8" ) 
            mbwprog = subprocess.Popen( args = [ "taskset" , "-c" , intf_mbw_thrstr ,
                                                  rbenchPath , "-r" , "mem-bw" , "--mb" , mem_bw_mb   , "-n" , str(intf_mbw_thr) ] , 
                      shell = False , stdout = None , stderr = sys.stderr , encoding="utf-8" ) 
            print( "\nrbench: llc %dx%s%% @(%s) + mbw %dx%sMB/s @(%s), test start: "%(
                    intf_llc_thr , llcstrength , intf_llc_thrstr , intf_mbw_thr , mem_bw_mb , intf_mbw_thrstr) , flush = True )
            caseIPC = apprun()
            print( "rbench: llc %dx%s%% @(%s) + mbw %dx%sMB/s @(%s), app IPC = %.4f"%( 
                    intf_llc_thr , llcstrength , intf_llc_thrstr , intf_mbw_thr , mem_bw_mb , intf_mbw_thrstr , caseIPC ) , flush = True )
            llcprog.kill()
            mbwprog.kill()
            llcres.append( caseIPC )
            kill_with_name( "rbench" )
        ress.append( llcres )
    for res in ress :
        print( res )

def app_3D_res_set_intf_membw():
    intf_llc_thr = 0 ; intf_mbw_thr = 8
    intf_llc_thrstr = "None" ; intf_mbw_thrstr = "32-39"
    # limit the mbw stressor's llc usage
    intf_lmem_pqos_id = 2 ; 
    pqos_set_to( "llc" , intf_lmem_pqos_id , ways[0] ) ; pqos_set_to( "mba" , intf_lmem_pqos_id , mbgears[7] ) 
    pqos_allocate( "core" , intf_lmem_pqos_id , intf_mbw_thrstr )
    print( "pqos COS%d: llc=%s , mba = %s -> core %s"%( intf_lmem_pqos_id , ways[0] , mbgears[7] , intf_mbw_thrstr ) , flush = True )
    mem_bw_mb = "8000" ; llcstrength = 0 
    mbwprog = subprocess.Popen( args = [ "taskset" , "-c" , intf_mbw_thrstr ,
                                            rbenchPath , "-r" , "mem-bw" , "--mb" , mem_bw_mb , "-n" , str(intf_mbw_thr) ] , 
                shell = False , stdout = None , stderr = sys.stderr , encoding="utf-8" ) 
    print( "\nwith rbench: llc %dx%s%% @(%s) + mbw %dx%sMB/s @(%s)\n"%(
            intf_llc_thr , llcstrength , intf_llc_thrstr , intf_mbw_thr , mem_bw_mb , intf_mbw_thrstr) , flush = True )

def app_3D_res_set_intf_llc():
    intf_llc_thr = 4 ; intf_mbw_thr = 0
    intf_llc_thrstr = "28-31" ; intf_mbw_thrstr = "None"
    # limit the mbw stressor's llc usage
    intf_lllc_pqos_id = 3 ; 
    pqos_set_to( "llc" , intf_lllc_pqos_id , ways[10] ) ; pqos_set_to( "mba" , intf_lllc_pqos_id , mbgears[0] ) 
    pqos_allocate( "core" , intf_lllc_pqos_id , intf_llc_thrstr )
    print( "pqos COS%d: llc=%s , mba = %s -> core %s"%( intf_lllc_pqos_id , ways[10] , mbgears[0] , intf_mbw_thrstr ) , flush = True )
    mem_bw_mb = 0 ; llcstrength = "100"
    llcprog = subprocess.Popen( args = [ "taskset" , "-c" , intf_llc_thrstr ,
                                            rbenchPath , "-r" , "cacheL3" , "-s" , llcstrength , "-n" , str(intf_llc_thr) ] , 
                shell = False , stdout = None , stderr = sys.stderr , encoding="utf-8" ) 
    print( "\nwith rbench: llc %dx%s%% @(%s) + mbw %dx%sMB/s @(%s)\n"%(
            intf_llc_thr , llcstrength , intf_llc_thrstr , intf_mbw_thr , mem_bw_mb , intf_mbw_thrstr) , flush = True )

def app_3D_by_res( set_intf = False ) :
    print( "\n---------------- app 3D by res -----------------\n")
    print( "default setting, baseline start:" )
    set_pqos_to_default()
    baseIPC = apprun()
    print( "the base IPC is" , baseIPC , "\n" , flush = True )

    if set_intf == True :
        app_3D_res_set_intf_membw() 

    ress = []
    for way in ways :
        restmp = []
        for mbgear in mbgears :
            # set application resource
            app_pqos_id = 1 ; 
            pqos_set_to( "llc" , app_pqos_id , way ) ; pqos_set_to( "mba" , app_pqos_id , mbgear ) 
            pqos_allocate( "core" , app_pqos_id , appthrstr )
            print( "pqos COS%d: llc=%s , mba = %s -> core %s, test start: "%( app_pqos_id , way , mbgear , appthrstr ) , flush = True )
            caseIPC = apprun( 3 , 0 , 0 )
            print( "pqos COS%d: llc=%s , mba = %s -> core %s, app IPC = %.4f\n"%( app_pqos_id , way , mbgear , appthrstr , caseIPC ) , flush = True )
            restmp.append( caseIPC )
        ress.append( restmp )
    
    for restmp in ress :
        print( restmp )
    
    return 

app_3D_by_intf() 
# app_3D_by_res()
# app_3D_by_res( True )

cleaner( None , None )
