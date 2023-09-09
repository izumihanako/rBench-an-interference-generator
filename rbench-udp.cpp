// stress-ng method
// https://github.com/ColinIanKing/stress-ng/blob/master/stress-cpu.c#L759
#include "rbench.hpp"
#include "core-net.hpp"
#define TOUCH_INFO_SIZE 15
static const int32_t UDP_SEND_CALC_SPEED_ROUND_CNT = 5000 ;
static const int32_t UDP_RECV_CALC_SPEED_ROUND_CNT = 2000 ;
static const int32_t UDP_SEND_KERNEL_PACK_PER_ROUND = 100 ;
static const int32_t UDP_RECV_KERNEL_PACK_PER_ROUND = 40 ;


static void udp_receive_kernel( UdpSocket &recv_sock , char* recvbuf , int32_t recvlen ){
    for( int i = 0 ; i < UDP_RECV_KERNEL_PACK_PER_ROUND ; i ++ ){
        recv_sock.Recv( recvbuf , recvlen ) ;
        // checksum 
        if( !net_check_headfoot_checksum( recvbuf , recvlen ) ){
            pr_warning( "udp_receive_kernel: check sum error" ) ;
        }
    }
}

static void udp_send_kernel( UdpSocket &send_sock , net_address_t netaddr , char* buf , int32_t size ){
    sockaddr addr ;
    if( netaddr.ip_type == NET_ADDRESS_TYPE_IPV4 ){
        sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>( &addr ) ;
        addr4->sin_addr.s_addr = inet_addr( netaddr.ip.c_str() ) ;
        addr4->sin_port = htons( netaddr.port ) ;
        addr4->sin_family = AF_INET ;
    } else if( netaddr.ip_type == NET_ADDRESS_TYPE_IPV6 ){
        return ;
    } else {
        pr_error( "udp_send_kernel -> Unknown IP type" ) ;
        return ;
    }

    for( int i = 0 ; i < UDP_SEND_KERNEL_PACK_PER_ROUND ; i ++ ){
        send_sock.Send( buf , size , addr ) ;
    }
}


void udp_client_bench( int32_t thrid , bench_args_t args , net_address_t srv_thraddr , double* pconn ){
    char infobuf[1024] ;
    
    int sendlen = args.network_psize ;
    char* sendbuf = new char[sendlen] , recvbuf[TOUCH_INFO_SIZE];{ // random sending data
        mwc_t mwc_eng ;
        mwc_eng.reseed() ;
        mwc_eng.fill_array( sendbuf , sendlen ) ;
        net_set_headfoot_checksum( sendbuf , sendlen ) ;
    }
    
    // client's new thread create its socket
    UdpSocket cli_thr_sock ;
    for( int init_socket_try = 0 ; init_socket_try < 10 ; init_socket_try ++ ){
        if( cli_thr_sock.Socket( args.netaddr.ip_type ) ) break ;
    }
    if( !cli_thr_sock.ExistsSocket() ){
        sprintf( infobuf , "%s( thread %d ): Init socket error, IPV%d , IP_PROTOUDP.\n"
                        "10 tries failed. Please check sudo(root) privilege or related settings\n" , 
                        args.bench_name.c_str() , thrid , (int)args.netaddr.ip_type ) ;
        pr_error( infobuf ) ;
        *pconn = NET_THREAD_ERROR ;
        return ;
    }
    for( uint16_t port_delta = 0 ; ; port_delta ++ ){
        uint16_t try_port = (uint16_t) ( args.netaddr.port + port_delta ) ;
        if( cli_thr_sock.Bind( args.netaddr.ip , try_port ) ) break ;
        if( port_delta > (uint16_t)65530 ){
            sprintf( infobuf , "%s( thread %d ): cannot find available port" , args.bench_name.c_str() , thrid ) ;
            pr_error( infobuf ) ;
            *pconn = NET_THREAD_ERROR ;
            return ;      
        }
    }
    cli_thr_sock.SetRecvTimeout( 6 , 0 ) ;

    // client thr inform server thr its ip:address
    if( !cli_thr_sock.Send( sendbuf , TOUCH_INFO_SIZE , srv_thraddr.ip , srv_thraddr.port ) ){
        sprintf( infobuf , "%s( thread %d ): Send pack for informing server_thr error.\n"
                           "From %s:%hu(client thr) to %s:%hu(server thr)", args.bench_name.c_str() , thrid , 
                           args.netaddr.ip.c_str() , args.netaddr.port , srv_thraddr.ip.c_str() , srv_thraddr.port ) ;
        pr_error( infobuf ) ; 
        *pconn = NET_THREAD_ERROR ;
        return ;
    }

    sprintf( infobuf , "%s( thread %d ): from %s:%hu(client thr) to %s:%hu(server thr). "
                       "Now waiting for start command from server...", args.bench_name.c_str() , thrid , 
                        args.netaddr.ip.c_str() , args.netaddr.port , srv_thraddr.ip.c_str() , srv_thraddr.port ) ;
    pr_info( infobuf ) ; 
    
    for( int i = 0 ; i < 1000 ; i ++ ){
        if( cli_thr_sock.Recv( recvbuf , TOUCH_INFO_SIZE , NULL , NULL ) ){
            if( atoi( recvbuf ) == NET_THREAD_READY ){
                *pconn = NET_THREAD_READY ;
                break ;
            }
        }
    }
    if( *pconn != NET_THREAD_READY ){
        sprintf( infobuf , "%s( thread %d ): Cannot receive NET_THREAD_READY from server_thr. "
                           "This thread will exit now.", args.bench_name.c_str() , thrid ) ;
        pr_error( infobuf ) ; 
        *pconn = NET_THREAD_ERROR ;
        return ;
    }
   
    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = UDP_SEND_CALC_SPEED_ROUND_CNT ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        udp_send_kernel( cli_thr_sock , srv_thraddr , sendbuf , sendlen ) ;
        *pconn += UDP_SEND_KERNEL_PACK_PER_ROUND ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    int32_t module_runrounds , module_sleepus ;
    network_pps_to_time( UDP_SEND_KERNEL_PACK_PER_ROUND , args.network_pps , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
    sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, sgl_idle = %.1fus, send_pps=%.1fpps, (rounds=%d , sleeptime=%dus , idletime=%.1fus)" , 
        args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , sgl_idle * ONE_MILLION , UDP_SEND_KERNEL_PACK_PER_ROUND / ( sgl_time + sgl_idle ) , module_runrounds, module_sleepus , sgl_idle*module_runrounds * ONE_MILLION ) ;
    pr_debug( infobuf ) ;

    // Run stressor
    bool in_low_actl_strength_warning = false ;
    int32_t round_cnt = 0 , time_limit = args.time , low_actl_strength_warning = 0 ;
    int64_t knl_round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT64_MAX ;
    int64_t knl_round_sumup = 0 ;
    double t_start = time_now() , sum_krounds = 0 , sum_sleepus = 0 , sum_runtimeus = 0 , sum_runidleus = 0 ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            udp_send_kernel( cli_thr_sock , srv_thraddr , sendbuf , sendlen ) ;
            *pconn += UDP_SEND_KERNEL_PACK_PER_ROUND ;
        }
        md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
        actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , run_idlet = md_t_end - md_t_start - actl_runt ;
        sum_runtimeus += actl_runt * ONE_MILLION , sum_runidleus += run_idlet * ONE_MILLION ;
        sum_krounds += measure_rounds ;

        md_t_start = time_now() ;
        std::this_thread::sleep_for (std::chrono::microseconds( module_sleepus ) );
        md_t_end = time_now() ;
        double actl_sleepus = ( md_t_end - md_t_start ) * ONE_MILLION ;
        sum_sleepus += actl_sleepus ;

        knl_round_sumup += measure_rounds ;
        if( knl_round_sumup >= knl_round_limit ) break ;
        if( time_limit && md_t_end - t_start >= time_limit ) break ;

        // Load strength feedback regulation
        if( !( round_cnt & 0x7 ) ){
            sgl_time = sum_runtimeus * ONE_MILLIONTH / sum_krounds , sgl_idle = sum_runidleus * ONE_MILLIONTH / sum_krounds ;
            double sum_packs = sum_krounds * UDP_SEND_KERNEL_PACK_PER_ROUND ,
                   actual_pps = sum_packs / ( sum_runtimeus + sum_sleepus + sum_runidleus ) * ONE_MILLION ;
            sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, actual_pps=%.1fpps, (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
                args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , actual_pps , sum_runtimeus, sum_sleepus , sum_runidleus ) ;
            pr_debug( infobuf ) ;
            // re-calculate load parameters
            if( args.network_pps - NETWORK_PPS_CONTROL_LBOUND > actual_pps || 
                args.network_pps + NETWORK_PPS_CONTROL_RBOUND < actual_pps ){
                network_pps_to_time( UDP_SEND_KERNEL_PACK_PER_ROUND , args.network_pps , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
            }
            if( args.network_pps - NETWORK_PPS_CONTROL_LBOUND > actual_pps ){
                if( ++low_actl_strength_warning > 8 ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): current %.1f%%pps, target %.1f%%pps, adjusting..." , 
                        args.bench_name.c_str() , thrid , actual_pps , (double)args.network_pps ) ;
                    pr_warning( infobuf ) ;
                    in_low_actl_strength_warning = true ;
                    low_actl_strength_warning = 0 ;
                }
            } else {
                if( in_low_actl_strength_warning ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): adjustment succeed, current %.1f%%pps, target %.1f%%pps" , 
                        args.bench_name.c_str() , thrid , actual_pps , (double)args.network_pps ) ;
                    pr_warning( infobuf ) ;
                }
                in_low_actl_strength_warning = false ;
                low_actl_strength_warning = 0 ;
            }
            sum_runtimeus -= ( sum_runtimeus ) / 5 , sum_krounds -= ( sum_krounds ) / 5 ;
            sum_sleepus -= ( sum_sleepus ) / 5 , sum_runidleus -= ( sum_runidleus ) / 5 ;
        }
    }
    sprintf( infobuf , "%s( thread %d ): stopped after %.3f seconds, %ld rounds" , 
        args.bench_name.c_str() ,  thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
    *pconn *= -1 ;
    delete[] sendbuf ;
}

void udp_server_bench( int32_t thrid , bench_args_t args , net_address_t cli_addr , int32_t recvlen , double* pconn ){
    char infobuf[1024] , recvbuf[NETWORK_MAX_PACK_SIZE] , sendbuf[TOUCH_INFO_SIZE] ;
    sprintf( sendbuf , "%d" , thrid ) ; // reply info includes the thread's id

    // server's new thread create its socket 
    UdpSocket srv_thr_sock ;
    for( int init_socket_try = 0 ; init_socket_try < 10 ; init_socket_try ++ ){
        if( srv_thr_sock.Socket( args.netaddr.ip_type ) ) break ;
    }
    if( !srv_thr_sock.ExistsSocket() ){
        sprintf( infobuf , "%s( thread %d ): init socket error, IPV%d , IP_PROTOUDP.\n"
                        "10 tries failed. Please check sudo(root) privilege or related settings\n" , 
                        args.bench_name.c_str() , thrid , (int)args.netaddr.ip_type ) ;
        pr_error( infobuf ) ;
        *pconn = NET_THREAD_ERROR ;
        return ;
    }
    for( uint16_t port_delta = 0 ; ; port_delta ++ ){
        uint16_t try_port = (uint16_t) ( args.netaddr.port + port_delta ) ;
        if( srv_thr_sock.Bind( args.netaddr.ip , try_port ) ) break ;
        if( port_delta > (uint16_t)65530 ){
            sprintf( infobuf , "%s( thread %d ): Cannot find available port" , args.bench_name.c_str() , thrid ) ;
            pr_error( infobuf ) ;
            *pconn = NET_THREAD_ERROR ;
            return ;      
        }
    }
    
    // server's new thread touches client main socket
    if( !srv_thr_sock.Send( sendbuf , TOUCH_INFO_SIZE , cli_addr.ip , cli_addr.port ) ){
        sprintf( infobuf , "%s( thread %d ): Send pack for touching client error.\n"
                           "From %s:%hu(server thr) to %s:%hu(client main)", args.bench_name.c_str() , thrid , 
                           args.netaddr.ip.c_str() , args.netaddr.port , cli_addr.ip.c_str() , cli_addr.port ) ;
        pr_error( infobuf ) ; 
        *pconn = NET_THREAD_ERROR ;
        return ;
    }

    // server's new thread get client thread's ip:port
    net_address_t cli_thr_addr ;
    if( !srv_thr_sock.Recv( recvbuf , TOUCH_INFO_SIZE , &cli_thr_addr.ip , &cli_thr_addr.port ) ){
        sprintf( infobuf , "%s( thread %d ): Receive pack from client_thr error.\n"
                           "From Unknown(client thr) to %s:%hu(server thr) ", args.bench_name.c_str() , thrid , 
                           args.netaddr.ip.c_str() , args.netaddr.port ) ;
        pr_error( infobuf ) ; 
        *pconn = NET_THREAD_ERROR ;
        return ;
    }

    while( round( *pconn ) != NET_THREAD_READY ){
        usleep( ONE_THOUSAND * 10 ) ;
    }

    for( int i = 0 ; i < 10 ; i ++ ){
        sprintf( sendbuf , "%d" , NET_THREAD_READY ) ;
        srv_thr_sock.Send( sendbuf , TOUCH_INFO_SIZE , cli_thr_addr.ip , cli_thr_addr.port ) ;
    }

    // Calculate load parameters 
    double md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
    int measure_rounds = UDP_RECV_CALC_SPEED_ROUND_CNT ;
    for( int i = 1 ; i <= measure_rounds ; i ++ ){
        udp_receive_kernel( srv_thr_sock , recvbuf , recvlen ) ;
        *pconn += UDP_RECV_KERNEL_PACK_PER_ROUND ;
    }
    double md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
    double actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , sgl_time = actl_runt / measure_rounds , 
           run_idlet = md_t_end - md_t_start - actl_runt , sgl_idle = run_idlet / measure_rounds ;
    int32_t module_runrounds , module_sleepus ;
    network_pps_to_time( UDP_RECV_KERNEL_PACK_PER_ROUND , args.network_pps , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
    sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, sgl_idle = %.1fus, recv_pps=%.1fpps, (rounds=%d , sleeptime=%dus , idletime=%.1fus)" , 
        args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , sgl_idle * ONE_MILLION , UDP_RECV_KERNEL_PACK_PER_ROUND / ( sgl_time + sgl_idle ) , module_runrounds, module_sleepus , sgl_idle*module_runrounds * ONE_MILLION ) ;
    pr_debug( infobuf ) ;

    // Run stressor
    bool in_low_actl_strength_warning = false ;
    int32_t round_cnt = 0 , time_limit = args.time , low_actl_strength_warning = 0 ;
    int64_t knl_round_limit = get_arg_flag( args.flags , FLAG_IS_LIMITED ) ? args.limit_round : INT64_MAX ;
    int64_t knl_round_sumup = 0 ;
    double t_start = time_now() , sum_krounds = 0 , sum_sleepus = 0 , sum_runtimeus = 0 , sum_runidleus = 0 ;
    while( true ){
        round_cnt ++ ;

        measure_rounds = module_runrounds ;
        md_thr_cpu_t_start = thread_time_now() , md_t_start = time_now() ;
        for( int i = 0 ; i < measure_rounds ; i ++ ){
            udp_receive_kernel( srv_thr_sock , recvbuf , recvlen ) ;
            *pconn += UDP_RECV_KERNEL_PACK_PER_ROUND ;
        }
        md_thr_cpu_t_end = thread_time_now() , md_t_end = time_now() ;
        actl_runt = md_thr_cpu_t_end - md_thr_cpu_t_start , run_idlet = md_t_end - md_t_start - actl_runt ;
        sum_runtimeus += actl_runt * ONE_MILLION , sum_runidleus += run_idlet * ONE_MILLION ;
        sum_krounds += measure_rounds ;

        md_t_start = time_now() ;
        std::this_thread::sleep_for (std::chrono::microseconds( module_sleepus ) );
        md_t_end = time_now() ;
        double actl_sleepus = ( md_t_end - md_t_start ) * ONE_MILLION ;
        sum_sleepus += actl_sleepus ;

        knl_round_sumup += measure_rounds ;
        if( knl_round_sumup >= knl_round_limit ) break ;
        if( time_limit && md_t_end - t_start >= time_limit ) break ;

        // Load strength feedback regulation
        if( !( round_cnt & 0x7 ) ){
            sgl_time = sum_runtimeus * ONE_MILLIONTH / sum_krounds , sgl_idle = sum_runidleus * ONE_MILLIONTH / sum_krounds ;
            double sum_packs = sum_krounds * UDP_RECV_KERNEL_PACK_PER_ROUND ,
                   actual_pps = sum_packs / ( sum_runtimeus + sum_sleepus + sum_runidleus ) * ONE_MILLION;
            sprintf( infobuf , "%s( thread %d ): sgl_time = %.1fus, recv_pps=%.1fpps, (runtime=%.1fus , sleeptime=%.1fus , idletime=%.1fus)" , 
                args.bench_name.c_str() , thrid , sgl_time * ONE_MILLION , actual_pps , sum_runtimeus, sum_sleepus , sum_runidleus ) ;
            pr_debug( infobuf ) ;
            // re-calculate load parameters
            if( args.network_pps - NETWORK_PPS_CONTROL_LBOUND > actual_pps || 
                args.network_pps + NETWORK_PPS_CONTROL_RBOUND < actual_pps ){
                network_pps_to_time( UDP_RECV_KERNEL_PACK_PER_ROUND , args.network_pps , sgl_time , sgl_idle , args.period , module_runrounds , module_sleepus ) ;
            }
            if( args.network_pps - NETWORK_PPS_CONTROL_LBOUND > actual_pps ){
                if( ++low_actl_strength_warning > 8 ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): current %.1f%%pps, target %.1f%%pps, adjusting..." , 
                        args.bench_name.c_str() , thrid , actual_pps , (double)args.network_pps ) ;
                    pr_warning( infobuf ) ;
                    in_low_actl_strength_warning = true ;
                    low_actl_strength_warning = 0 ;
                }
            } else {
                if( in_low_actl_strength_warning ){
                    sprintf( infobuf , "LOW STRENGTH - %s( thread %d ): adjustment succeed, current %.1f%%pps, target %.1f%%pps" , 
                        args.bench_name.c_str() , thrid , actual_pps , (double)args.network_pps ) ;
                    pr_warning( infobuf ) ;
                }
                in_low_actl_strength_warning = false ;
                low_actl_strength_warning = 0 ;
            }
            sum_runtimeus -= ( sum_runtimeus ) / 5 , sum_krounds -= ( sum_krounds ) / 5 ;
            sum_sleepus -= ( sum_sleepus ) / 5 , sum_runidleus -= ( sum_runidleus ) / 5 ;
        }
    }
    sprintf( infobuf , "%s( thread %d ): stopped after %.3f seconds, %ld rounds" , 
        args.bench_name.c_str() ,  thrid , time_now() - t_start , knl_round_sumup ) ;
    pr_info( infobuf ) ;
    *pconn *= -1 ;
}

// 0. both client and server create its own main socket
// 1. client touch server's listening address:port 
// 2. server create a new thread. This new thread touches the client's socket
// 3. client create a new thread. This new thread sends packs to server.
// client(send) -->>(pack)>>--  server(receive)

int32_t udp_client_bench_entry( bench_args_t args ){
    char infobuf[1024] ;
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }

    // create client's touching socket
    UdpSocket cli_touch_sock ;
    for( int init_socket_try = 0 ; init_socket_try < 10 ; init_socket_try ++ ){
        if( cli_touch_sock.Socket( args.netaddr.ip_type ) ) break ;
    } 
    if( !cli_touch_sock.ExistsSocket() ){
        sprintf( infobuf , "%s, creating touching socket: init socket error, IPV%d , IP_PROTOUDP.\n"
                        "10 tries failed. Please check sudo(root) privilege or related settings\n" , 
                        args.bench_name.c_str() , (int)args.netaddr.ip_type ) ;
        pr_error( infobuf ) ;
        return -1 ; 
    }
    if( !cli_touch_sock.Bind( args.netaddr.ip , args.netaddr.port ) ){
        sprintf( infobuf , "%s, creating touching socket: bind to %s:%hu error.\n"
                        "Please make sure that the giving ip:port is available\n" , 
                        args.bench_name.c_str() , args.netaddr.ip.c_str() , args.netaddr.port ) ;
        pr_error( infobuf ) ;
        return -1 ;
    }

    // set socket timeout
    cli_touch_sock.SetRecvTimeout( 6 , 0 ) ;
    cli_touch_sock.SetSendTimeout( 6 , 0 ) ;

    // touch connections and run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    double *conns = new double[count_thr] ;
    for( int i = 0 ; i < count_thr ; i ++ ) conns[i] = NET_THREAD_WAIT ;
    for( int thrid = 1 ; thrid <= count_thr ; thrid ++ ){
        // touch connections
        char *sendbuf = new char[TOUCH_INFO_SIZE] ; // the information is the pack size 
        char *recvbuf = new char[TOUCH_INFO_SIZE] ; // receive reply from server threads 
        net_address_t srv_addr( args.to_addr.ip_type ) ;
        int32_t touch_fail_cnt = 0 , srv_thrid = -1 ;
        sprintf( sendbuf , "%d" , args.network_psize ) ;
        
        while( touch_fail_cnt < 10 ){
            cli_touch_sock.Send( sendbuf , TOUCH_INFO_SIZE , args.to_addr.ip , args.to_addr.port ) ;
            if( cli_touch_sock.Recv( recvbuf , TOUCH_INFO_SIZE , &srv_addr.ip , &srv_addr.port ) ){
                srv_thrid = atoi( recvbuf ) ;
                break ;
            }
            touch_fail_cnt ++ ;
            sprintf( infobuf , "%s, creating thread %d: Receive timeout " , args.bench_name.c_str() , thrid ) ;
            pr_debug( infobuf ) ;
            if( touch_fail_cnt >= 10 ){
                sprintf( infobuf , "%s, creating thread %d: Cannot receive server's reply. Retrying... " , args.bench_name.c_str() , thrid ) ;
                pr_error( infobuf ) ;
                touch_fail_cnt = 0 ;
            }
            usleep( ONE_THOUSAND * 10 ) ; // sleep 0.1s 
        }
        // server create a new thread
        sprintf( infobuf , "%s, received reply from server %s:%hu, thread %d, pack size=%d bytes" , 
                        args.bench_name.c_str() , srv_addr.ip.c_str() , srv_addr.port , srv_thrid , args.network_psize ) ;
        pr_info( infobuf ) ;
        delete[] sendbuf ;
        delete[] recvbuf ;
        thrs[thrid-1] = thread( udp_client_bench , thrid , args , srv_addr , &conns[thrid-1] ) ;
    }
    while( true ){
        int WAIT_cnt = 0 ;
        for( int i = 0 ; i < count_thr ; i ++ ) WAIT_cnt += conns[i] == NET_THREAD_WAIT ;
        if( !WAIT_cnt ) break ;
        usleep( ONE_THOUSAND * 10 ) ;
    }

    if( get_arg_flag( args.flags , FLAG_IS_RUN_PARALLEL ) ){
        pr_warning( "udp test do not support argument:\"parallel\", ignored" ) ;
    }

    // output real time pps
    double prepsum = 0 , nowpsum = 0 , lasttime = time_now() , thistime ;
    while( true ){
        prepsum = nowpsum , nowpsum = 0 ;
        int cnt_norun = 0 ;
        char tmp[100] ;
        auto curr_tm = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ) ;
        std::strftime( tmp , sizeof( tmp ) , "%F %T" , std::localtime( &curr_tm ) ) ;
        for( int i = 0 ; i < count_thr ; i ++ ) {
            if( conns[i] >= 0 ) nowpsum += conns[i] ;
            else if( conns[i] < NET_THREAD_STATE_BEGIN ) cnt_norun ++ , nowpsum += -conns[i] ;
        }
        thistime = time_now() ;
        sprintf( infobuf , "%s, %s, real time speed: %.1fpps" , 
                        args.bench_name.c_str() , tmp , ( nowpsum - prepsum ) / (thistime - lasttime) ) ;
        pr_info( infobuf ) ;
        lasttime = thistime ;
        if( cnt_norun == count_thr ) break ;
        sleep( 1 ) ;
    }

    for( auto &thr : thrs ){
        thr.join() ;
    }        
    return 0 ;
}

int32_t udp_server_bench_entry( bench_args_t args ){
    char infobuf[1024] ;
    int count_thr = args.threads ;
    if( get_arg_flag( args.flags , FLAG_IS_CHECK ) || get_arg_flag( args.flags , FLAG_PRINT_DEBUG_INFO ) ){
        args.print_argsinfo() ;
    }

    // create server's listening socket 
    UdpSocket srv_listen_sock ;
    for( int init_socket_try = 0 ; init_socket_try < 10 ; init_socket_try ++ ){
        if( srv_listen_sock.Socket( args.netaddr.ip_type ) ) break ;
    }
    if( !srv_listen_sock.ExistsSocket() ){
        sprintf( infobuf , "%s, creating listening socket: init socket error, IPV%d , IP_PROTOUDP.\n"
                        "10 tries failed. Please check sudo(root) privilege or related settings\n" , 
                        args.bench_name.c_str() , (int)args.netaddr.ip_type ) ;
        pr_error( infobuf ) ;
        return -1 ;
    }
    if( !srv_listen_sock.Bind( args.netaddr.ip , args.netaddr.port ) ){
        sprintf( infobuf , "%s, creating listening socket: bind to %s:%hu error.\n"
                        "Please make sure that the giving ip:port is available\n" , 
                        args.bench_name.c_str() , args.netaddr.ip.c_str() , args.netaddr.port ) ;
        pr_error( infobuf ) ;
        return -1 ;
    }

    // set socket timeout
    srv_listen_sock.SetRecvTimeout( 6 , 0 ) ;
    srv_listen_sock.SetSendTimeout( 6 , 0 ) ;

    // listen connections and run stressors 
    vector<thread> thrs ;
    thrs.resize( count_thr ) ;
    double *conns = new double[count_thr] ;
    for( int i = 0 ; i < count_thr ; i ++ ) conns[i] = NET_THREAD_WAIT ;

    for( int thrid = 1 ; thrid <= count_thr ; thrid ++ ){
        // listen connections
        char *listenbuf = new char[TOUCH_INFO_SIZE] ; // the information is the pack size 
        net_address_t cli_addr( args.netaddr.ip_type ) ;
        int32_t listen_fail_cnt = 0 , cli_packsize = -1 ;
        while( listen_fail_cnt < 10 && !srv_listen_sock.Recv( listenbuf , TOUCH_INFO_SIZE , &cli_addr.ip , &cli_addr.port ) ){
            listen_fail_cnt ++ ;
            sprintf( infobuf , "%s, creating thread %d: Receive timeout " , args.bench_name.c_str() , thrid ) ;
            pr_debug( infobuf ) ;
            if( listen_fail_cnt >= 10 ){
                sprintf( infobuf , "%s, creating thread %d: Cannot receive touch info from client. Listening... " , args.bench_name.c_str() , thrid ) ;
                pr_error( infobuf ) ;
                listen_fail_cnt = 0 ;
            }
        }
        // server create a new thread
        cli_packsize = atoi( listenbuf ) ;
        sprintf( infobuf , "%s, received connection from client %s:%hu, pack size=%d bytes" , 
                        args.bench_name.c_str() , cli_addr.ip.c_str() , cli_addr.port , cli_packsize ) ;
        pr_info( infobuf ) ;
        delete []listenbuf ;
        thrs[thrid-1] = thread( udp_server_bench , thrid , args , cli_addr , cli_packsize , &conns[thrid-1] ) ;
    }
    for( int i = 0 ; i < count_thr ; i ++ ) conns[i] = NET_THREAD_READY ;

    if( get_arg_flag( args.flags , FLAG_IS_RUN_PARALLEL ) ){
        pr_warning( "udp test do not support argument:\"parallel\", ignored" ) ;
    }

    // output real time pps
    double prepsum = 0 , nowpsum = 0 , lasttime = time_now() , thistime ;
    while( true ){
        prepsum = nowpsum , nowpsum = 0 ;
        int cnt_norun = 0 ;
        char tmp[100] ;
        auto curr_tm = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ) ;
        std::strftime( tmp , sizeof( tmp ) , "%F %T" , std::localtime( &curr_tm ) ) ;
        for( int i = 0 ; i < count_thr ; i ++ ) {
            if( conns[i] >= 0 ) nowpsum += conns[i] ;
            else if( conns[i] < NET_THREAD_STATE_BEGIN ) cnt_norun ++ , nowpsum += -conns[i] ;
        }
        thistime = time_now() ;
        sprintf( infobuf , "%s, %s, real time speed: %.1fpps" , 
                        args.bench_name.c_str() , tmp , ( nowpsum - prepsum ) / (thistime - lasttime) ) ;
        pr_info( infobuf ) ;
        lasttime = thistime ;
        if( cnt_norun == count_thr ) break ;
        sleep( 1 ) ;
    }

    for( auto &thr : thrs ){
        thr.join() ;
    }        
    return 0 ;
}

#undef TOUCH_INFO_SIZE