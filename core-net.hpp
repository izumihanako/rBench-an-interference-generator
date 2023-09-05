#include <cstdio>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

int __net_interface_exists( const char* , const int , sockaddr* ) ;

class UdpSocket {
	public:
		UdpSocket() :_sockfd(-1) {}
		//创建套接字
		bool Socket() ;
		bool Bind(const std::string& , uint16_t , const char* = NULL ) ;
		bool Recv(char* , std::string* = NULL , uint16_t* = NULL ) ;
		bool Send(const char* , int , std::string& , const uint16_t ) ;
		bool Close() ; 
	private:
		int _sockfd;
};
