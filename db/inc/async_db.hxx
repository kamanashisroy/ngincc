#ifndef NGINCC_ASYNC_DB_HXX
#define NGINCC_ASYNC_DB_HXX

#include <string>

namespace ngincc {
    namespace core {
        namespace parallel {
            // forward declaration
            class pipeline;
        }
    }
    namespace db {
        //! client-side db-api
        class async_db {
        public:
            async_db() = delete;
            async_db(ngincc::core::parallel::pipeline& core_pipe);
            ~async_db() = default;
            int set_if_null(int cb_token, const std::string& cb_hook, const std::string& key, int intval);
            int get(int cb_token, const std::string& cb_hook, const std::string& key);
            int increment(int cb_token, const std::string& cb_hook, const std::string& key, int intval, int increment);
            int unset(int cb_token, const std::string& cb_hook, const std::string& key);
            int compare_and_swap(int cb_token, const std::string& cb_hook, const std::string& key, const std::string& newval, const std::string& oldval);
            static inline const std::string empty_hook{""};
        private:
            ngincc::core::parallel::pipeline& core_pipe;
        };
    }
}

#endif // NGINCC_ASYNC_DB_HXX
