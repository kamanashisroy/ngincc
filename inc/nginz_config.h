#ifndef NGINZ_CONFIG_H
#define NGINZ_CONFIG_H

C_CAPSULE_START

#define MAX_POLL_FD 10000
#define NGINZ_INLINE inline
#define NUMBER_OF_PROCESSORS 3
//#define NGINZ_POLL_ALL_FLAGS POLLIN | POLLPRI | POLLHUP
#define NGINZ_POLL_ALL_FLAGS POLLIN
#define NGINZ_DEFAULT_PORT 9699
#define NGINZ_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_HASHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_SEARCHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SEARCHABLE | OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})

C_CAPSULE_END

#endif // NGINZ_CONFIG_H
