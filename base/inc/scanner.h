#ifndef NGINZ_SCANNER
#define NGINZ_SCANNER

C_CAPSULE_START

/**
 * NOTE the next needs to be cleaned up by aroop_txt_destroy(next) call
 */
int shotodol_scanner_next_token (aroop_txt_t* src, aroop_txt_t* next);

C_CAPSULE_END

#endif // NGINZ_SCANNER
