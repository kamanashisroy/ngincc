#ifndef NGINZ_DB_H
#define NGINZ_DB_H

C_CAPSULE_START

int db_save(char*key, char*value);
int db_get(char*key,aroop_txt_t*output);


int db_module_init();
int db_module_deinit();

C_CAPSULE_END

#endif // NGINZ_DB_H
