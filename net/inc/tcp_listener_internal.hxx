#ifndef NGINCC_TCP_LISTENER_HXX
#define NGINCC_TCP_LISTENER_HXX

C_CAPSULE_START

int tcp_listener_init();
int tcp_listener_deinit();

int protostack_get_ports_internal(int*tcp_ports);

C_CAPSULE_END

#endif // NGINCC_TCP_LISTENER_HXX
