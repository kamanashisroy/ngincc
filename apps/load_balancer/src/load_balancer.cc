
#include <unistd.h>
#include <sys/socket.h>

#include "log.hxx"
#include "load_balancer.hxx"
#include "parallel/pipeline.hxx"

using ngincc::apps::load_balancer;
using ngincc::apps::round_robin_load_balancer;
using ngincc::core::parallel::pipeline;

/*int round_robin_load_balancer::setup() {
	nid = getpid();
	return 0;
}*/

int round_robin_load_balancer::next() {
	return (nid = pipe.pp_next_worker_nid(nid));
}

round_robin_load_balancer::round_robin_load_balancer(
    pipeline& pipe
    ) : pipe(pipe) {
	nid = getpid();
}

round_robin_load_balancer::~round_robin_load_balancer() {
}

