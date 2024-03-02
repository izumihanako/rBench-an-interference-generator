#include <iostream>
#include <string>
#include "udpsocket.hpp"
using namespace std;

#define CHECK_RET(q) if((q)==false){return false;}

int main(int argc, char *argv[]) {
	//运行时有三个参数 udp_srv 192.168.73.29 4096
	if (argc != 3) {
		cout << "Usage: ./udp_srv ip prot\n";
		return -1;
	}
	uint16_t port = stoi(argv[2]);
	string ip = argv[1];
	UdpSocket srv_sock;
	//创建套接字
	CHECK_RET(srv_sock.Socket());
	//绑定地址信息
	CHECK_RET(srv_sock.Bind(ip, port)) ;

    char *buf = new char[4096] ;
	while(1) {
		//接收数据
		string peer_ip;
		uint16_t peer_port;
		CHECK_RET( srv_sock.Recv( buf, &peer_ip, &peer_port ) ) ;
		cout << "client["<<peer_ip<<":"<<peer_port<<"] say: " << buf << endl;
        int cnt = 0 ; 
        double start = time_now() ;
        while( true ){
            // cout << "server say: ";
            cnt ++ ;
            if( cnt % 10000 == 0 )
            for( int i = 0 ; i < 40 ; i ++ ){
                buf[i] = rand() % 26 + 'a' ;
            }
            CHECK_RET( srv_sock.Send( buf , 40 , peer_ip, peer_port ) ) ;
            if( cnt == 100000 ){
                printf( "speed = %f/s\n" , 100000.0 / ( ( 1.0 * time_now() - start ) ) ) ;
                start = time_now() ;
                cnt = 0 ;
            }
        }
	}
	srv_sock.Close();
	return 0;
}

