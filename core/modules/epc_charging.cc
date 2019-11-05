/* for definitions */
#include "epc_charging.h"
/* for ToIpv4Address() */
#include "utils/ip.h"
/* for rte_zmalloc() */
#include <rte_malloc.h>
/* for be32_t */
#include "utils/endian.h"
/* for rte_hash */
#include <rte_jhash.h>
/* for gtp definitions */
#include "utils/gtp_common.h"
/*----------------------------------------------------------------------------------*/
using bess::utils::Ipv4;
using bess::utils::be16_t;
using bess::utils::be32_t;

enum {DEFAULT_GATE = 0, FORWARD_GATE};
/*----------------------------------------------------------------------------------*/
void
EPCCharging::ProcessBatch(Context *ctx, bess::PacketBatch *batch)
{
	int cnt = batch->cnt();
	int hits = 0;
	uint64_t hit_mask = 0ULL;
	bess::Packet *p = NULL;
	uint64_t key[bess::PacketBatch::kMaxBurst];
	void *key_ptr[bess::PacketBatch::kMaxBurst];
	struct session_info *data[bess::PacketBatch::kMaxBurst];

	for (int i = 0; i < cnt; i++) {
		p = batch->pkts()[i];
		/* assuming that this module is placed in the pipeline */
		/* where the Ethernet header is stripped from each packet */
		Ipv4 *iph = p->head_data<Ipv4 *>();
		be32_t saddr = iph->src;
		key[i] = SESS_ID(saddr.raw_value(), DEFAULT_BEARER);
		key_ptr[i] = &key[i];
	}

	if ((hits = rte_hash_lookup_bulk_data(session_map,
					      (const void **)&key_ptr,
					      cnt,
					      &hit_mask,
					      (void **)data)) <= 0) {
		DLOG(INFO) << "Failed to look-up" << std::endl;
		/* Since default module is sink, the packets go right in the dump */
		/* RunNextModule() sends batch to DEFAULT GATE */
		RunNextModule(ctx, batch);
		return;
	}

	for (int i = 0; i < cnt; i++) {
		bess::Packet *p = batch->pkts()[i];

		if (!ISSET_BIT(hit_mask, i)) {
			EmitPacket(ctx, p, DEFAULT_GATE);
			std::cerr << "Fetch failed for ip->daddr: "
				  << ToIpv4Address(be32_t(UE_ADDR(key[i])))
				  << std::endl;
			continue;
		}
		Ipv4 *iph = p->head_data<Ipv4 *>();
		be16_t len = iph->length;
		/* update dir direction of statistics per session */
		if (dir == UPLINK) data[i]->ipcan_dp_bearer_cdr.data_vol.ul_bytes_delta += len.value();
		else data[i]->ipcan_dp_bearer_cdr.data_vol.dl_bytes_delta += len.value();

		/* forward the packet */
		EmitPacket(ctx, p, FORWARD_GATE);
	}
}
/*----------------------------------------------------------------------------------*/
CommandResponse
EPCCharging::Init(const bess::pb::EPCChargingArg &arg) {

	std::string ename = arg.ename();

	if (ename.c_str()[0] == '\0')
		return CommandFailure(EINVAL,
				      "Invalid input name");

	dir = arg.dir();

	std::string hashtable_name = "session_map" + ename;
	std::cerr << "Fetching rte_hash: " << hashtable_name << std::endl;

	session_map = rte_hash_find_existing(hashtable_name.c_str());
	if (session_map == NULL)
		return CommandFailure(EINVAL,
				      "Unable to find rte_hash table: %s\n",
				      "session_map");
	return CommandSuccess();
}
/*----------------------------------------------------------------------------------*/
ADD_MODULE(EPCCharging, "epc_charging", "first version of EPC Charging module")
