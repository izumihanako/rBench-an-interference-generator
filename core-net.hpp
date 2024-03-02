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

#ifndef NET_ADDRESS_TYPE_DEF
#define NET_ADDRESS_TYPE_DEF
enum net_address_type_t {
    NET_ADDRESS_TYPE_IPV4 = 4 ,
    NET_ADDRESS_TYPE_IPV6 = 6 ,
    NET_ADDRESS_TYPE_UNKNOWN = 255 ,
} ;
#endif

enum net_thread_state_t {
	NET_THREAD_STATE_BEGIN = -100 ,
	NET_THREAD_WAIT      = -10 ,
	NET_THREAD_READY     = - 9 ,
	NET_THREAD_ERROR     = - 8 ,
	NET_THREAD_FINISH    = - 1 ,
	NET_THREAD_STATE_END =   0 ,
} ;

int __net_interface_exists( const char* , const int , sockaddr* ) ;
void net_set_headfoot_checksum( char* , int32_t ) ;
bool net_check_headfoot_checksum( char* , int32_t ) ;

class UdpSocket {
	public:
		UdpSocket() :_sockfd(-1) {}
		~UdpSocket() ;
		//创建套接字
		bool Socket( net_address_type_t ) ;
		bool Bind(const std::string& , uint16_t , const char* = NULL ) ;
		bool Recv(char* , int , std::string* = NULL , uint16_t* = NULL ) ;
		bool Send(const char* , int , const sockaddr& ) ;
		bool Send(const char* , int , const std::string& , const uint16_t ) ;
		bool SetRecvTimeout( int32_t sec = 5 , int32_t usec = 0 ) ;
		bool SetSendTimeout( int32_t sec = 5 , int32_t usec = 0 ) ;
		bool Close() ; 
		bool ExistsSocket() const ;
	private:
		int _sockfd ;
		net_address_type_t ip_type ;
};
