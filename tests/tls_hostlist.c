#include "params.h"
#include "hostlist.h"
#include <string.h>

struct params_s params;

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "failed line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	uint8_t hello[sizeof(fake_tls_clienthello_default)];
	char host[256];
	memcpy(hello, fake_tls_clienthello_default, sizeof(hello));
	CHECK(IsTLSClientHello(hello, sizeof(hello), false));
	CHECK(TLSHelloExtractHost(hello, sizeof(hello), host, sizeof(host), false));
	CHECK(!strcmp(host, "www.microsoft.com"));
	// Equal-length replacement preserves all TLS length fields.
	uint8_t *sni = NULL;
	for (size_t i = 0; i + 17 <= sizeof(hello); ++i)
		if (!memcmp(hello + i, host, 17)) { sni = hello + i; break; }
	CHECK(sni);
	memcpy(sni, "x.steamstatic.com", 17);
	CHECK(TLSHelloExtractHost(hello, sizeof(hello), host, sizeof(host), false));
	CHECK(!strcmp(host, "x.steamstatic.com"));
	struct proto_pos split = {.marker = PM_HOST_MIDSLD, .pos = 0};
	size_t pos = ResolvePos(hello, sizeof(hello), TLS, &split);
	CHECK(pos == (size_t)(sni - hello) + 2 + strlen("steamstatic") / 2);
	for (size_t len = 0; len < 6; ++len)
		CHECK(!IsTLSClientHello(hello, len, false));
	hello[5] = 2;
	CHECK(!IsTLSClientHello(hello, sizeof(hello), false));

	struct desync_profile dp;
	dp_init(&dp);
	LIST_INIT(&params.hostlists);
	CHECK(RegisterHostlist(&dp, false, "dist/steam_vn.txt"));
	CHECK(LoadAllHostLists());
	bool excluded = false;
	CHECK(HostlistCheck(&dp, host, false, &excluded, true));
	CHECK(HostlistCheck(&dp, "api.pubg.com", false, &excluded, true));
	CHECK(!HostlistCheck(&dp, "steampowered.com.example.org", false, &excluded, true));
	CHECK(!HostlistCheck(&dp, "example.org", false, &excluded, true));
	dp_clear(&dp);
	hostlist_files_destroy(&params.hostlists);
	return 0;
}
