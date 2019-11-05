#ifndef BESS_MODULES_CHARGING_H_
#define BESS_MODULES_CHARGING_H_
/*----------------------------------------------------------------------------------*/
#include <rte_hash.h>
#include "../module.h"
/*----------------------------------------------------------------------------------*/
enum {UPLINK = 0, DOWNLINK};
/*----------------------------------------------------------------------------------*/
class EPCCharging final : public Module {
 public:
	EPCCharging() {
		max_allowed_workers_ = Worker::kMaxWorkers;
	}

	static const gate_idx_t kNumOGates = 2;

	CommandResponse Init(const bess::pb::EPCChargingArg &arg);
	void ProcessBatch(Context *ctx, bess::PacketBatch *batch) override;

 private:
	struct rte_hash *session_map = NULL;
	uint8_t dir = UPLINK;
};
/*----------------------------------------------------------------------------------*/
#endif // BESS_MODULES_CHARGING_H_
