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
            int set_if_null(uint32_t cb_token, const std::string& cb_hook, const std::string& key, uint32_t intval);
            int get(uint32_t cb_token, const std::string& cb_hook, const std::string& key);
            int increment(uint32_t cb_token, const std::string& cb_hook, const std::string& key, uint32_t intval, uint32_t increment);
            int unset(uint32_t cb_token, const std::string& cb_hook, const std::string& key);
            int compare_and_swap(uint32_t cb_token, const std::string& cb_hook, const std::string& key, const std::string& newval, const std::string& oldval);
            static inline const std::string empty_hook{""};
        private:
            ngincc::core::parallel::pipeline& core_pipe;
        };
    }
}

#endif // NGINCC_ASYNC_DB_HXX
