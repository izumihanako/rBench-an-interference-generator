#include "rbench.hpp"
#include "rbench-core.hpp"

namespace mytime{

double OPTIMIZE3 timeval_to_double(const struct timeval *tv)
{
	return (double)tv->tv_sec + ((double)tv->tv_usec * ONE_MILLIONTH);
}

double OPTIMIZE3 time_now(void)
{
	timeval now;

	if (gettimeofday(&now, NULL) < 0)
		return -1.0;

	return timeval_to_double(&now);
}

}
