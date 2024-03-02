#include <iostream>
#include <string>
#include "udpsocket.hpp"
using namespace std;

#define CHECK_RET(q) if((q)==false){return -1;}
int main( int argc, char *argv[] ) {
        if (argc != 3) {
                cout << "Usage: ./udpcli ip port\n";
                return -1;
        }
        string srv_ip = argv[1];
        uint16_t srv_port = stoi(argv[2]);

        UdpSocket cli_sock;
        //创建套接字
        CHECK_RET( cli_sock.Socket() );
        //绑定数据(不推荐)
    	char *buf = new char[4096] ;
        CHECK_RET( cli_sock.Send(buf , 20 , srv_ip, srv_port) ) ;
        int cnt = 0 ;
        double start = time_now() ;
        while(1) {
                CHECK_RET( cli_sock.Recv( buf ) ) ;
                if( strlen( buf ) != 40 ){
                        printf( "%d\n" , (int)strlen(buf) ) ;
                }
                cnt ++ ;
                if( cnt == 100000 ){
                        printf( "speed = %f/s\n" , 100000.0 / ( ( 1.0 * time_now() - start ) ) ) ;
                        start = time_now() ;
                        cnt = 0 ;
                }
        }
        //关闭套接字
        cli_sock.Close();
        return 0;
}