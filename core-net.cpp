#ifndef RBENCH_CORE_NET_H
#define RBENCH_CORE_NET_H 

#include "rbench.hpp"
#include "core-net.hpp"

int __net_interface_exists(const char* interface , const int domain , sockaddr* addr) {
	struct ifaddrs *ifaddr, *ifa;
	int ret = -1;

	if (!interface)
		return -1;
	if (!addr)
		return -1;

	if (getifaddrs(&ifaddr) < 0)
		return -1;
	for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr)
			continue;
		if (!ifa->ifa_name)
			continue;
		if (ifa->ifa_addr->sa_family != domain)
			continue;
		if (strcmp(ifa->ifa_name, interface) == 0) {
			(void)memcpy(addr, ifa->ifa_addr, sizeof(*addr));
			ret = 0;
			break;
		}
	}
	freeifaddrs(ifaddr);
	return ret;
}

void net_set_headfoot_checksum( char* buf , int32_t buflen ) {
    if( buflen > 2 ) buf[0] = buf[buflen-1] ^ buf[1] ;
    else buf[0] = buf[buflen-1] ;
}

bool net_check_headfoot_checksum( char* buf , int32_t buflen ){
    if( buflen > 2 ) return buf[0] == ( buf[buflen-1] ^ buf[1] ) ;
    else return buf[0] == buf[buflen-1] ;
}

bool UdpSocket::Socket( net_address_type_t _ip_type ) {
    this->ip_type = _ip_type ;
    if( ip_type == NET_ADDRESS_TYPE_IPV4 )      _sockfd = socket(AF_INET , SOCK_DGRAM, IPPROTO_UDP ) ;
    else if( ip_type == NET_ADDRESS_TYPE_IPV6 ) _sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP ) ;
    else {
        pr_error( "Unknown IP type" ) ;
        _sockfd = -1 ;
    }
    if (_sockfd < 0) {
        return false;
    }
    return true;
}

bool UdpSocket::Bind( const string &ip , uint16_t port , const char* ifname ) {
    sockaddr_in addr;
    if( ifname != NULL && __net_interface_exists( ifname , AF_INET , (sockaddr *)&addr ) ){
        // if interface exists, addr is automatically changed in function
    } else {
        addr.sin_addr.s_addr = inet_addr(ip.c_str()); // 将字符串IP地址转化为网络字节序IP地址
    }
    // 地址信息赋值
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port) ; // 将主机字节序短整型型数据转化为网络字节序数据
    socklen_t len = sizeof(struct sockaddr_in);
    int ret = bind( _sockfd , (struct sockaddr*)&addr , len ) ;
    if (ret < 0) {
        return false;
    }
    return true;    
}

bool UdpSocket::Recv(char* buf , int32_t data_len , string* ip , uint16_t* port ) {
    sockaddr_in peer_addr ; //用于接收发送端的地址信息
    socklen_t len = sizeof(struct sockaddr_in);
    ssize_t ret = recvfrom( _sockfd, buf , data_len, 0, (struct sockaddr*)&peer_addr , &len ) ;
    if (ret < 0) {
        return false;
    }
    if (port != NULL) {
        *port = ntohs( peer_addr.sin_port ) ; // 网络字节序到主机字节序的转换
    }
    if (ip != NULL) {
        *ip = string( inet_ntoa( peer_addr.sin_addr ) ) ; // 网络字节序到字符串IP地址的转换
    }
    return true;
}

bool UdpSocket::Send( const char* data , int data_len , const sockaddr &addr ){
    socklen_t len = sizeof( sockaddr ) ;
    ssize_t ret = sendto( _sockfd , data , data_len , 0 , &addr , len ) ;
    if( ret < 0 ){
        return false ;
    }
    return true ;
}

bool UdpSocket::Send(const char* data, int data_len , const string& ip, const uint16_t port ) {
    sockaddr_in addr ;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t len = sizeof(sockaddr_in);
    ssize_t ret = sendto(_sockfd, data , data_len , 0, (sockaddr*)&addr, len);
    if (ret < 0) {
        return false;
    }
    return true;
}

// return true if socket exists
bool UdpSocket::ExistsSocket() const {
    return _sockfd > 0 ;
}

bool UdpSocket::SetRecvTimeout( int32_t sec , int32_t usec ){
    if( sec < 0 || usec < 0 || sec + usec == 0 ) return false ;
    timeval tmptv ;
    tmptv.tv_sec = sec , tmptv.tv_usec = usec ;
    if (setsockopt( _sockfd , SOL_SOCKET, SO_RCVTIMEO, &tmptv, sizeof(tmptv)) < 0) {
        pr_error( "UdpSocket: SetTimeout option SO_RCVTIMEO not support\n" ) ;
        return false ;
    }
    return true ;
}

bool UdpSocket::SetSendTimeout( int32_t sec , int32_t usec ){
    if( sec < 0 || usec < 0 || sec + usec == 0 ) return false;
    timeval tmptv ;
    tmptv.tv_sec = sec , tmptv.tv_usec = usec ;
    if (setsockopt( _sockfd , SOL_SOCKET, SO_SNDTIMEO , &tmptv, sizeof(tmptv)) < 0) {
        pr_error( "UdpSocket: SetTimeout option SO_RCVTIMEO not support\n" ) ;
        return false ;
    }
    return true ;
}

bool UdpSocket::Close() {
    if ( ExistsSocket() ) {
        close(_sockfd);
        _sockfd = -1;
    }
    return true;
}

UdpSocket::~UdpSocket(){
    (void)( this->Close() ) ;
}

#endif