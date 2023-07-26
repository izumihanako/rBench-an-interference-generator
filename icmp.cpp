/*
 * Copyright (C) 2013-2021 Canonical, Ltd.
 * Copyright (C) 2022-2023 Colin Ian King.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <sys/time.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

static const char benchname[] = "icmp-flood" ;

/* Memory size constants */
#define KB			(1ULL << 10)
#define	MB			(1ULL << 20)
#define GB			(1ULL << 30)
#define TB			(1ULL << 40)
#define PB			(1ULL << 50)
#define EB			(1ULL << 60)

#define ONE_BILLIONTH		(1.0E-9)
#define ONE_MILLIONTH		(1.0E-6)
#define ONE_THOUSANDTH		(1.0E-3)

/* attributes */
#define ALIGNED(a)	        __attribute__((aligned(a)))
#define ALIGN64		        ALIGNED(64)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0) 
#define LIKELY(x)           __builtin_expect(!!(x), 1) 
#define HOT		            __attribute__ ((hot))
#define OPTIMIZE3 	        __attribute__((optimize("-O3")))

/* for this test */
#define MAX_PAYLOAD_SIZE	(1000)
#define MAX_PKT_LEN		    (sizeof(struct iphdr) + \
				        sizeof(struct icmphdr) + \
				        MAX_PAYLOAD_SIZE + 1)

/* MWC random number initial seed */
#define STRESS_MWC_SEED_W	(521288629UL)
#define STRESS_MWC_SEED_Z	(362436069UL)

/* Fast random number generator state */
struct stress_mwc_t{
	uint32_t w;
	uint32_t z;
	uint32_t n16;
	uint32_t saved16;
	uint32_t n8;
	uint32_t saved8;
	uint32_t n1;
	uint32_t saved1;
} ;

static stress_mwc_t mwc = {
	STRESS_MWC_SEED_W,
	STRESS_MWC_SEED_Z,
	0,
	0,
	0,
	0,
	0,
	0,
};

/*
 *  stress_mwc32()
 *      Multiply-with-carry random numbers
 *      fast pseudo random number generator, see
 *      http://www.cse.yorku.ca/~oz/marsaglia-rng.html
 */
HOT OPTIMIZE3 inline uint32_t stress_mwc32(void)
{
	mwc.z = 36969 * (mwc.z & 65535) + (mwc.z >> 16);
	mwc.w = 18000 * (mwc.w & 65535) + (mwc.w >> 16);
	return (mwc.z << 16) + mwc.w;
}

/*
 *  stress_mwc16()
 *	get a 16 bit pseudo random number
 */
HOT OPTIMIZE3 uint16_t stress_mwc16(void)
{
	if (LIKELY(mwc.n16)) {
		mwc.n16--;
		mwc.saved16 >>= 16;
	} else {
		mwc.n16 = 1;
		mwc.saved16 = stress_mwc32();
	}
	return mwc.saved16 & 0xffff;
}

/*
 *  stress_mwc8()
 *	get an 8 bit pseudo random number
 */
HOT OPTIMIZE3 uint8_t stress_mwc8(void)
{
	if (LIKELY(mwc.n8)) {
		mwc.n8--;
		mwc.saved8 >>= 8;
	} else {
		mwc.n8 = 3;
		mwc.saved8 = stress_mwc32();
	}
	return mwc.saved8 & 0xff;
}

/*
 *  stress_mwc32modn()
 *	return 32 bit non-modulo biased value 1..max (inclusive)
 *	with no non-zero max check
 */
static uint32_t OPTIMIZE3 stress_mwc32modn_nonzero(const uint32_t max)
{
	register uint32_t threshold = max;
	register uint32_t val;

	while (threshold < 0x80000000UL) {
		threshold <<= 1;
	}
	do {
		val = stress_mwc32();
	} while (val >= threshold);

	return val % max;
}

/*
 *  stress_mwc32modn()
 *	return 32 bit non-modulo biased value 1..max (inclusive)
 *	where max is most probably not a power of 2
 */
uint32_t OPTIMIZE3 stress_mwc32modn(const uint32_t max)
{
	return (LIKELY(max > 0)) ? stress_mwc32modn_nonzero(max) : 0;
}

/*
 *  stress_ipv4_checksum()
 *	ipv4 data checksum
 */
uint16_t HOT OPTIMIZE3 stress_ipv4_checksum(uint16_t *ptr, const size_t sz)
{
	register uint32_t sum = 0;
	register size_t n = sz;

	while (n > 1) {
		sum += *ptr++;
		n -= 2;
	}

	if (n)
		sum += *(uint8_t*)ptr;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return (uint16_t)~sum;
}

/*
 *  stress_timeval_to_double()
 *      convert timeval to seconds as a double
 */
double OPTIMIZE3 stress_timeval_to_double(const struct timeval *tv)
{
	return (double)tv->tv_sec + ((double)tv->tv_usec * ONE_MILLIONTH);
}

static OPTIMIZE3 double stress_time_now(void)
{
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0)
		return -1.0;

	return stress_timeval_to_double(&now);
}


void pr_fail(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void)vprintf(fmt, ap);
	va_end(ap);
}

void pr_dbg(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void)vfprintf(stdout,fmt, ap);
	va_end(ap);
}

/*
 *  stress_rndbuf()
 *	fill buffer with pseudorandom bytes
 */
void stress_rndbuf(void *buf, size_t len)
{
	register char *ptr = (char *)buf;
	register const char *end = ptr + len;

	while (ptr < end)
		*ptr++ = stress_mwc8();
}

/*
 *  stress_icmp_flood
 *	stress local host with ICMP flood
 *  must run with root or CAP_NET_RAW rights
 */
static int stress_icmp_flood( uint64_t num_of_packs )
{
	int fd , rc = -1 ;
	const int set_on = 1;
	const unsigned long addr = inet_addr("10.77.110.141");
	struct sockaddr_in servaddr;
	uint64_t counter, sendto_fails = 0, sendto_ok, rest_pack = num_of_packs;
	double bytes = 0.0, t_start, duration, rate;

	char ALIGN64 pkt[MAX_PKT_LEN];
	struct iphdr *const ip_hdr = (struct iphdr *)pkt;
	struct icmphdr *const icmp_hdr = (struct icmphdr *)(pkt + sizeof(struct iphdr));
	char *const payload = pkt + sizeof(struct iphdr) + sizeof(struct icmphdr);

	(void)memset(pkt, 0, sizeof(pkt));
	stress_rndbuf(payload, MAX_PAYLOAD_SIZE);

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (fd < 0) {
		pr_fail("%s: socket failed, errno=%d (%s)\n",
			benchname, errno, strerror(errno));
		goto err;
	}
	if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL,
		(const char *)&set_on, sizeof(set_on)) < 0) {
		pr_fail("%s: setsockopt IP_HDRINCL  failed, errno=%d (%s)\n",
			benchname, errno, strerror(errno));
		goto err_socket;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
		(const char *)&set_on, sizeof(set_on)) < 0) {
		pr_fail("%s: setsockopt SO_BROADCAST failed, errno=%d (%s)\n",
			benchname, errno, strerror(errno));
		goto err_socket;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = (in_addr_t)addr;
	(void)memset(&servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));

	printf( "now running\n" ) ;

	t_start = stress_time_now();
	do {
		const size_t payload_len = stress_mwc32modn(MAX_PAYLOAD_SIZE) + 1;
		const size_t pkt_len =
			sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len;
		ssize_t ret;

		(void)memset(pkt, 0, sizeof(pkt));

		ip_hdr->version = 4;
		ip_hdr->ihl = 5;
		ip_hdr->tos = 0;
		ip_hdr->tot_len = htons(pkt_len);
		ip_hdr->id = stress_mwc16();
		ip_hdr->frag_off = 0;
		ip_hdr->ttl = 64;
		ip_hdr->protocol = IPPROTO_ICMP;
		ip_hdr->saddr = (in_addr_t)addr;
		ip_hdr->daddr = (in_addr_t)addr;

		icmp_hdr->type = ICMP_ECHO;
		icmp_hdr->code = 0;
		icmp_hdr->un.echo.sequence = stress_mwc16();
		icmp_hdr->un.echo.id = stress_mwc16();

		/*
		 * Generating random data is expensive so do it every 64 packets
		 */
		if ( ( rest_pack & 0x3f ) == 0)
			stress_rndbuf(payload, payload_len);
		icmp_hdr->checksum = stress_ipv4_checksum((uint16_t *)icmp_hdr,
			sizeof(struct icmphdr) + payload_len);

		ret = sendto(fd, pkt, pkt_len, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if (UNLIKELY(ret < 0)) {
			sendto_fails++;
		} else {
			bytes += (double)ret;
		}
		rest_pack -- ;
	} while ( rest_pack > 0 );
	duration = stress_time_now() - t_start;

	counter = num_of_packs ;
	sendto_ok = counter - sendto_fails;

	rate = (duration > 0.0) ? sendto_ok / duration : 0.0;
	pr_dbg( "sendto calls per sec : %.3f\n" , rate ) ;
	rate = (duration > 0.0) ? bytes / duration : 0.0;
	pr_dbg( "MB written per sec   : %.3f\n" , rate / (double)MB);

	pr_dbg("%s: %.2f%% of %llu sendto messages succeeded.\n",
		benchname,
		100.0 * (double)sendto_ok / (double)counter, counter);

	rc = 0;

err_socket:
	(void)close(fd);
err:
	return rc;
}

int main(){
	uint64_t num_of_packs ;
	scanf( "%llu" , &num_of_packs ) ;
	printf( "%llu\n" , num_of_packs ) ;
	stress_icmp_flood( num_of_packs ) ;
}