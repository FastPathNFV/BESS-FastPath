#include "monitor.h" 

#include <algorithm>
#include <rte_cycles.h>


const Commands Monitor::cmds = {
    {"clear", "EmptyArg", MODULE_CMD_FUNC(&Monitor::CommandClear),
     Command::Command::THREAD_UNSAFE}};

Monitor::Monitor() : Module() {
  sfunc = [&](bess::Packet *pkt[[maybe_unused]], void *arg[[maybe_unused]]) ->bool {
      uint64_t hash = *(uint64_t *)arg;
      ++count[hash];
			return false;
    }; 
}

CommandResponse Monitor::Init(const bess::pb::EmptyArg &){
  return CommandSuccess();
}

CommandResponse Monitor::CommandClear(const bess::pb::EmptyArg &){
  return CommandSuccess();
}

void Monitor::ProcessBatch(bess::PacketBatch *batch) {
  int cnt = batch->cnt();

  for (int i = 0; i < cnt; i++) {
    bess::Packet *pkt = batch->pkts()[i];
    Path *path = batch->path(i);

    uint8_t fid[Path::FIDLEN];
    uint64_t hash = 0;
    MAT::getFID(pkt, hash, fid);
    if (!memcmp(fid, cache[hash], Path::FIDLEN))
      ++count[hash];
    else {
      memcpy(cache[hash], fid, Path::FIDLEN);
      count[hash] = 1;
    }
    
    if (path != nullptr) {
      HeadAction *head = nullptr;
      StateAction *state = nullptr;
      path->appendRule(this, head, state);
      state->action = sfunc;
      uint64_t *arg = (uint64_t *)malloc(sizeof(uint64_t));
      *arg = hash;
      state->arg = (void*)arg;
    }
  }

  RunNextModule(batch);
}

ADD_MODULE(Monitor, "monitor", "count packets in flow")

