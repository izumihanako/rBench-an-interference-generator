//udpsocket.hpp
#include <cstdio>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

int stress_net_interface_exists(const char *interface, const int domain, struct sockaddr *addr) {
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

class UdpSocket
{
	public:
		UdpSocket() :_sockfd(-1) {}
		//创建套接字
		bool Socket() {
			_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (_sockfd < 0) {
				perror("socket error");
				return false;
			}
			return true;
		}
		//为套接字绑定地址信息
		bool Bind(const string &ip, uint16_t port, const char *ifname = NULL ) {
			//定义IPv4地址结构体
			struct sockaddr_in addr;

            if( ifname != NULL && stress_net_interface_exists( ifname , AF_INET , (sockaddr *)&addr ) ){

            } else {
			    addr.sin_addr.s_addr = inet_addr(ip.c_str());//将字符串IP地址转化为网络字节序IP地址
            }
			//地址信息赋值
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);//将主机字节序短整型型数据转化为网络字节序数据
			socklen_t len = sizeof(struct sockaddr_in);
			int ret = bind(_sockfd, (struct sockaddr*)&addr, len);
			if (ret < 0) {
				perror("bind error");
				return false;
			}
			return true;
		}
		//接收数据，获取发送端地址信息
		bool Recv(string *buf, string *ip = NULL, uint16_t *port = NULL ) {
			struct sockaddr_in peer_addr;//用于接收发送端的地址信息
			socklen_t len = sizeof(struct sockaddr_in);
			char tmp[4096] = {0};
			int ret = recvfrom(_sockfd, tmp, 4096, 0, (struct sockaddr*)&peer_addr, &len);
			if (ret < 0) {
				perror("recvfrom error");
				return false;
			}
			buf->assign(tmp, ret);//assign从指定字符串中截取指定长度的数据到buf中
			if (port != NULL) {
				*port = ntohs(peer_addr.sin_port);//网络字节序到主机字节序的转换
			}
			if (ip != NULL) {
				*ip = inet_ntoa(peer_addr.sin_addr);//网络字节序到字符串IP地址的转换
			}
			return true;
		}
		//发送数据
		bool Send(const string &data, string &ip, const uint16_t port)
		{
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = inet_addr(ip.c_str());
			socklen_t len = sizeof(struct sockaddr_in);
			int ret = sendto(_sockfd, data.c_str(), data.size(), 0, (struct sockaddr*)&addr, len);
			if (ret < 0)
			{
				perror("sendto error");
				return false;
			}
			return true;
		}
		//关闭套接字
		bool Close()
		{
			if (_sockfd > 0)
			{
				close(_sockfd);
				_sockfd = -1;
			}
			return true;
		}
	private:
		int _sockfd;
};

