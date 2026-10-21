#include "params.h"
#include "hostlist.h"
#include <arpa/inet.h>
#include <string.h>

struct params_s params;
static uint8_t sent[4][2048];
static size_t sent_len[4];
static unsigned int sent_count;

// Capture the OS send boundary; parsing, matching and segmentation stay real.
bool rawsend(const struct sockaddr *dst __attribute__((unused)),
	uint32_t fwmark __attribute__((unused)), const char *ifout __attribute__((unused)),
	const void *data, size_t len)
{
	if (sent_count >= 4 || len > sizeof(sent[0])) return false;
	memcpy(sent[sent_count], data, len);
	sent_len[sent_count++] = len;
	return true;
}

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "failed line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

static int run_packet(const uint8_t *hello, size_t hello_len, uint16_t port, unsigned int expected_count)
{
	uint8_t packet[2048];
	size_t len = sizeof(packet);
	struct sockaddr_in src = {.sin_family = AF_INET, .sin_port = htons(50000)};
	struct sockaddr_in dst = {.sin_family = AF_INET, .sin_port = htons(port)};
	inet_pton(AF_INET, "192.0.2.1", &src.sin_addr);
	inet_pton(AF_INET, "198.51.100.1", &dst.sin_addr);
	CHECK(prepare_tcp_segment4(&src, &dst, TH_ACK | TH_PUSH, false, 0,
		htonl(1000), htonl(2000), htons(65535), SCALE_NONE, NULL, true,
		64, 0, 0, FOOL_NONE, 0, 0, 0, hello, hello_len, packet, &len));
	sent_count = 0;
	uint8_t verdict = dpi_desync_packet(0, "test", "test", packet, &len);
	CHECK(sent_count == expected_count);
	CHECK((verdict & VERDICT_MASK) == (expected_count ? VERDICT_DROP : VERDICT_PASS));
	if (expected_count)
	{
		uint8_t restored[1024];
		size_t total = 0;
		for (unsigned int i = 0; i < sent_count; ++i)
		{
			struct dissect dis;
			proto_dissect_l3l4(sent[i], sent_len[i], &dis);
			CHECK(dis.tcp && dis.len_payload);
			size_t offset = ntohl(dis.tcp->th_seq) - 1000;
			CHECK(offset + dis.len_payload <= sizeof(restored));
			memcpy(restored + offset, dis.data_payload, dis.len_payload);
			total += dis.len_payload;
		}
		CHECK(total == hello_len);
		CHECK(!memcmp(restored, hello, hello_len));
	}
	return 0;
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	LIST_INIT(&params.desync_profiles);
	LIST_INIT(&params.hostlists);
	LIST_INIT(&params.ipsets);
	params.ctrack_disable = true;
	params.tls_clienthello_only = true;
	struct desync_profile_list *entry = dp_list_add(&params.desync_profiles);
	CHECK(entry);
	entry->dp.n = 1;
	entry->dp.desync_mode = DESYNC_MULTISPLIT;
	entry->dp.split_count = 1;
	entry->dp.splits[0].marker = PM_HOST_MIDSLD;
	CHECK(RegisterHostlist(&entry->dp, false, "dist/steam_vn.txt"));
	CHECK(dp_list_add(&params.desync_profiles)); // default fallback while SNI is unknown
	CHECK(LoadAllHostLists());
	uint8_t hello[sizeof(fake_tls_clienthello_default)];
	memcpy(hello, fake_tls_clienthello_default, sizeof(hello));
	CHECK(!run_packet(hello, sizeof(hello), 443, 0)); // unrelated SNI
	for (size_t i = 0; i + 17 <= sizeof(hello); ++i)
		if (!memcmp(hello + i, "www.microsoft.com", 17)) { memcpy(hello + i, "x.steamstatic.com", 17); break; }
	CHECK(!run_packet(hello, sizeof(hello), 443, 2));
	entry->dp.desync_mode = DESYNC_MULTIDISORDER;
	CHECK(!run_packet(hello, sizeof(hello), 443, 2));
	CHECK(!run_packet(hello, sizeof(hello), 80, 0));
	hello[5] = 2;
	CHECK(!run_packet(hello, sizeof(hello), 443, 0));
	dp_list_destroy(&params.desync_profiles);
	hostlist_files_destroy(&params.hostlists);
	return 0;
}
