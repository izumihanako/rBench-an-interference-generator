import subprocess

print( "runiBench.py imported" ) 
print( "    l1i, l1d, l2 on another vcore of the phycore which applications running on" )
print( "    l3, memBw on socket" )

cpus=eval(subprocess.getstatusoutput('cat /proc/cpuinfo |grep "physical id"|sort |uniq|wc -l')[1])
phycores=eval(subprocess.getstatusoutput('cat /proc/cpuinfo |grep "cores"|uniq| tr -cd "[0-9]"')[1]) * cpus
cores=subprocess.getstatusoutput('cat /proc/cpuinfo| grep "processor"| wc -l')[1]
cores=eval(cores)
quotaqunta = 1000
roundtime = 14400
benchname = "iBench"

def l1i( L , R , quotacore ):
    print( "with iBenchl1i interference @ core %d - %d, each quota = %d%%, tot = %d%%"%(L,R,quotacore,quotacore*(R-L+1)) )
    for i in range( L , R + 1 , 1 ):
        subprocess.getstatusoutput("docker run --name ibench_l1i_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotacore*quotaqunta)+" -d anakli/ibench:l1i ./l1i "+str(roundtime)+" 20")
    return

def l1iend( L , R ):
    l1ilist = ""
    for i in range( L , R + 1 , 1  ):
        l1ilist = l1ilist + "ibench_l1i_"+str(i)+" "
    subprocess.getstatusoutput("docker container stop "+l1ilist+" > /dev/null")
    subprocess.getstatusoutput("docker container rm "+l1ilist+" > /dev/null")
    print( "l1i interference end @ core %d - %d"%(L,R) )

def l1d( L , R , quotacore ):
    print( "with iBenchl1d interference @ core %d - %d, each quota = %d%%, tot = %d%%"%(L,R,quotacore,quotacore*(R-L+1)) )
    for i in range( L , R + 1 , 1 ):
        subprocess.getstatusoutput("docker run --name ibench_l1d_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotacore*quotaqunta)+" -d anakli/ibench:l1d ./l1d "+str(roundtime) )
    return

def l1dend( L , R ):
    l1ilist = ""
    for i in range( L , R + 1 , 1  ):
        l1ilist = l1ilist + "ibench_l1d_"+str(i)+" "
    subprocess.getstatusoutput("docker container stop "+l1ilist+" > /dev/null")
    subprocess.getstatusoutput("docker container rm "+l1ilist+" > /dev/null")
    print( "l1d interference end @ core %d - %d"%(L,R) )

def l2( L , R , quotacore ):
    print( "with bubblel2 interference @ core %d - %d, each quota = %d%%, tot = %d%%"%(L,R,quotacore,quotacore*(R-L+1)) )
    for i in range( L , R + 1 , 1 ):
        subprocess.getstatusoutput("docker run --name ibench_l2_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotacore*quotaqunta)+" -d anakli/ibench:l2 ./l2 "+str(roundtime) )
    return

def l2end( L , R ):
    l2list = ""
    for i in range( L , R + 1 , 1 ):
        l2list = l2list + "ibench_l2_"+str(i)+" "
    subprocess.getstatusoutput("docker container stop "+l2list+" > /dev/null")
    subprocess.getstatusoutput("docker container rm "+l2list+" > /dev/null")
    print( "l2 interference end @ core %d - %d"%(L,R) )

def l3( L , R , quotacore ):
    print( "with bubblel3 interference @ core %d - %d, each quota = %d%%, tot = %d%%"%(L,R,quotacore,quotacore*(R-L+1)) )
    for i in range( L , R + 1, 1 ):
        subprocess.getstatusoutput("docker run --name ibench_llc_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotacore*quotaqunta)+" -d anakli/ibench:llc ./l3 "+str(roundtime) )
    return

def l3end( L , R ):
    llclist = ""
    for i in range( L , R + 1 , 1 ):
        llclist = llclist + "ibench_llc_"+str(i)+" "
    subprocess.getstatusoutput("docker container stop "+llclist+" > /dev/null")
    subprocess.getstatusoutput("docker container rm "+llclist+" > /dev/null")
    print( "l3 interference end @ core %d - %d"%(L,R) )

def memBw( L , R , quotacore ):
    print( "with iBenchmemBw interference @ core %d - %d, each quota = %d%%, tot = %d%%"%(L,R,quotacore,quotacore*(R-L+1)) )
    for i in range( L , R + 1 , 1 ):
        subprocess.getstatusoutput("docker run --name ibench_memBw_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotacore*quotaqunta)+" -d anakli/ibench:memBw ./memBw "+str(roundtime) )
    return

def memBwend( L , R ):
    memBwlist = ""
    for i in range( L , R + 1 , 1 ):
        memBwlist = memBwlist + "ibench_memBw_"+str(i)+" "
    subprocess.getstatusoutput("docker container stop "+memBwlist+" > /dev/null")
    subprocess.getstatusoutput("docker container rm "+memBwlist+" > /dev/null")
    print( "memBw interference end @ core %d - %d"%(L,R) )

def cleaner( ):
    l1iend( 0 , 39 )
    l1dend( 0 , 39 )
    l2end( 0 , 39 )
    l3end( 0 , 39 )
    memBwend( 0 , 39 )

# pqos -e "llc:1=0x001;llc:0=0x7ff"
# pqos -a "llc:1=36-39;llc:0=0-35"