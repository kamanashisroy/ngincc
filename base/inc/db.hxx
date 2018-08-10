#ifndef NGINZ_DB_HXX
#define NGINZ_DB_HXX

#ifdef HAS_MEMCACHED_MODULE
namespace ngincc {
    namespace core {
        class db : public module {
        public:
            int db_set(const string &key, const string &instr);
            int db_get(const string &key, string &outstr);
            int db_get_int(const string &key, int32_t &outval);
            int db_set_int(const string &key, int32_t inval);
        };
    }
}
#endif

#endif // NGINZ_DB_HXX
