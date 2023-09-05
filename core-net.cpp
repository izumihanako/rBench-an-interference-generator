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

bool UdpSocket::Socket() {
    _sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (_sockfd < 0) {
        perror("socket error");
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
        perror( "bind error" ) ;
        return false;
    }
    return true;    
}

bool UdpSocket::Recv(char* buf , string* ip , uint16_t* port ) {
    sockaddr_in peer_addr ; //用于接收发送端的地址信息
    socklen_t len = sizeof(struct sockaddr_in);
    char tmp[4096] = {0};
    ssize_t ret = recvfrom( _sockfd, tmp, 4096, 0, (struct sockaddr*)&peer_addr , &len ) ;
    if (ret < 0) {
        // pr_error("recvfrom error") ;
        return false;
    }
    memcpy( buf , tmp , sizeof( char ) * ret ) ;
    if (port != NULL) {
        *port = ntohs( peer_addr.sin_port ) ; // 网络字节序到主机字节序的转换
    }
    if (ip != NULL) {
        *ip = string( inet_ntoa( peer_addr.sin_addr ) ) ; // 网络字节序到字符串IP地址的转换
    }
    return true;
}

bool UdpSocket::Send(const char* data, int data_len , string& ip, const uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    socklen_t len = sizeof(struct sockaddr_in);
    ssize_t ret = sendto(_sockfd, data , data_len , 0, (struct sockaddr*)&addr, len);
    if (ret < 0) {
        // pr_error( "sendto error\n" ) ;
        return false;
    }
    return true;
}

bool UdpSocket::Close() {
    if (_sockfd > 0) {
        close(_sockfd);
        _sockfd = -1;
    }
    return true;
}

#endif