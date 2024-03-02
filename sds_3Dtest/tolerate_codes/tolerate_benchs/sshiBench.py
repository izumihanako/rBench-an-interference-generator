import paramiko

print( "sshiBench.py imported" ) 
print( "l1i, l1d, l2 on vcores which applications running on" )
print( "l3, memBw on socket" )

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect( "10.77.110.141" , 5102 , "root" , "Hpcadmin_141" )

def init( appthr_: int , cores_: int , cpus_: int ): 
    global appthr, cores, cpus, ssh
    appthr = appthr_
    cores = cores_ 
    cpus = cpus_

def l1i():
    global ssh
    roundtime = 14400 #s
    print( "with l1i interference %ds"%(roundtime) )
    quotal1i=100000
    for i in range(0,appthr,1):
        sin , sout , serr = ssh.exec_command("docker run --name ibench_l1i_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotal1i)+" -d anakli/ibench:l1i ./l1i "+str(roundtime)+" 20")
        sout.read() , serr.read()

def l1iend():
    global ssh
    l1ilist = ""
    for i in range(0,appthr,1):
        l1ilist = l1ilist + "ibench_l1i_"+str(i)+" "
    sin , sout , serr = ssh.exec_command("docker container stop "+l1ilist+" > /dev/null")
    sout.read() ,  serr.read()
    sin , sout , serr = ssh.exec_command("docker container rm "+l1ilist+" > /dev/null")
    sout.read() ,  serr.read()
    print( "l1i interference end" )

def l1d():
    global ssh
    roundtime = 7200 #s
    print( "with l1d interference %ds"%(roundtime) )
    quotal1d=100000
    for i in range(0,appthr,1):
        sin , sout , serr = ssh.exec_command("docker run --name ibench_l1d_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotal1d)+" -d anakli/ibench:l1d ./l1d "+str(roundtime) )
        sout.read() , serr.read() 

def l1dend():
    global ssh
    l1ilist = ""
    for i in range(0,appthr,1):
        l1ilist = l1ilist + "ibench_l1d_"+str(i)+" "
    sin , sout , serr = ssh.exec_command("docker container stop "+l1ilist+" > /dev/null")
    sout.read() , serr.read() 
    sin , sout , serr = ssh.exec_command("docker container rm "+l1ilist+" > /dev/null")
    sout.read() , serr.read() 
    print( "l1d interference end" )

def l2():
    global ssh
    roundtime = 7200 #s
    print( "with l2 interference %ds"%(roundtime) )
    quotal2=100000
    for i in range(0,appthr,1):
        sin , sout , serr = ssh.exec_command("docker run --name ibench_l2_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotal2)+" -d anakli/ibench:l2 ./l2 "+str(roundtime) )
        sout.read() , serr.read() 

def l2end():
    global ssh
    l2list = ""
    for i in range(0,appthr,1):
        l2list = l2list + "ibench_l2_"+str(i)+" "
    sin , sout , serr = ssh.exec_command("docker container stop "+l2list+" > /dev/null")
    sout.read() , serr.read() 
    sin , sout , serr = ssh.exec_command("docker container rm "+l2list+" > /dev/null")
    sout.read() , serr.read() 
    print( "l2 interference end" )

def llc():
    global ssh
    roundtime = 7200 #s
    print( "with llc interference %ds"%(roundtime) )
    quotallc=100000
    for i in range(cores-cpus,cores,1):
        sin , sout , serr = ssh.exec_command("docker run --name ibench_llc_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotallc)+" -d anakli/ibench:llc ./l3 "+str(roundtime) )
        sout.read() , serr.read() 

def llcend():
    global ssh
    llclist = ""
    for i in range(cores-cpus,cores,1):
        llclist = llclist + "ibench_llc_"+str(i)+" "
    sin , sout , serr = ssh.exec_command("docker container stop "+llclist+" > /dev/null")
    sout.read() , serr.read() 
    sin , sout , serr = ssh.exec_command("docker container rm "+llclist+" > /dev/null")
    sout.read() , serr.read() 
    print( "llc interference end" )

def memBw():
    global ssh
    roundtime = 7200 #s
    print( "with memBw interference %ds"%(roundtime) )
    quotamemBw=100000
    for i in range(cores-cpus,cores,1):
        ssin , sout , serr = ssh.exec_command("docker run --name ibench_memBw_"+str(i)+" --cpuset-cpus "+str(i)+" --cpu-quota "+str(quotamemBw)+" -d anakli/ibench:memBw ./memBw "+str(roundtime) )
        sout.read() , serr.read() 

def memBwend():
    global ssh
    memBwlist = ""
    for i in range(cores-cpus,cores,1):
        memBwlist = memBwlist + "ibench_memBw_"+str(i)+" "
    sin , sout , serr = ssh.exec_command("docker container stop "+memBwlist+" > /dev/null")
    sout.read() , serr.read() 
    sin , sout , serr = ssh.exec_command("docker container rm "+memBwlist+" > /dev/null")
    sout.read() , serr.read() 
    print( "memBw interference end" )


def cleaner( signum , frame ):
    l1iend()
    l1dend()
    l2end()
    llcend()
    memBwend()
    exit()