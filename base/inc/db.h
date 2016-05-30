#ifndef NGINZ_DB_H
#define NGINZ_DB_H

C_CAPSULE_START

#ifdef HAS_MEMCACHED_MODULE
int db_set(const char*key, const char*value);
/**
 * Note that the implementation allocates memroy. To release the memory aroop_txt_destroy(output) call is needed.
 */
int db_get(const char*key,aroop_txt_t*output);
int db_get_int(const char*key);
int db_set_int(const char*key, int value);


int db_module_init();
int db_module_deinit();
#endif

C_CAPSULE_END

#endif // NGINZ_DB_H
