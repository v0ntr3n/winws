#include "params.h"

struct params_s params;

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	int evaluated = 0;
	params.debug = true;
	DLOG("%d", ++evaluated);
	hexdump_limited_dlog((const uint8_t *)"x", ++evaluated, 1);
	return evaluated != 0;
}
