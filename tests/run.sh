#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d)
trap 'rm -r "$test_dir"' EXIT HUP INT TERM
cc=${CC:-gcc}
"$cc" -std=gnu99 -DNDEBUG -O3 -Wall -Werror -ffunction-sections -fdata-sections -Infq tests/logging_release.c nfq/params.c -Wl,--gc-sections -o "$test_dir/logging"
"$test_dir/logging"
"$cc" -std=gnu99 -DNDEBUG -O3 -ffunction-sections -fdata-sections -Infq tests/tls_hostlist.c nfq/desync.c nfq/protocol.c nfq/params.c nfq/helpers.c nfq/pools.c nfq/hostlist.c nfq/gzip.c -Wl,--gc-sections -lz -o "$test_dir/tls"
"$test_dir/tls"
# The portable BSD branch avoids host libmnl/Netfilter dependencies. Only the
# raw-send boundary is replaced; real packet construction/dissection is linked.
"$cc" -std=gnu99 -DNDEBUG -O3 -U__linux__ -DBSD -ffunction-sections -fdata-sections -Drawsend=unused_platform_rawsend -c nfq/darkmagic.c -o "$test_dir/darkmagic.o"
"$cc" -std=gnu99 -DNDEBUG -O3 -U__linux__ -DBSD -ffunction-sections -fdata-sections -Infq tests/packet_multisplit.c "$test_dir/darkmagic.o" nfq/desync.c nfq/protocol.c nfq/params.c nfq/helpers.c nfq/pools.c nfq/hostlist.c nfq/ipset.c nfq/conntrack.c nfq/packet_queue.c nfq/checksum.c nfq/gzip.c nfq/crypto/*.c -Wl,--gc-sections -lz -o "$test_dir/packet"
"$test_dir/packet"
echo "Release logging, TLS/SNI/midsld/hostlist and stateless packet segmentation tests passed."
