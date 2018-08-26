#ifndef NGINCC_ASYNC_DB_MASTER_HXX
#define NGINCC_ASYNC_DB_MASTER_HXX

#include <vector>
#include <unordered_map>

namespace ngincc {
    namespace core {
        // forward declarations
        class buffer_coder;
        class plugin_manager;
        namespace parallel {
            class pipeline;
        }
    }
    namespace db {
        class async_db;
        //! Master db replies queries of the child processes
        class async_db_master {
            //! TODO implement shared-memory-circular-consumer-producer-queue instead of system-unix-pipe
        public:
            async_db_master(
                ngincc::core::plugin_manager& core_plug
                , ngincc::core::parallel::pipeline& core_pipe
            );
            ~async_db_master();
            int noasync_set(const std::string& key, const std::string& inval);
            int noasync_get(const std::string& key, std::string& outval);
        private:
            static inline const std::string null_value{""};
            static inline const std::string compare_and_swap_app{"asyncdb/cas/request"};
            static inline const std::string set_if_null_app{"asyncdb/sin/request"};
            static inline const std::string unset_app{"asyncdb/unset/request"};
            static inline const std::string get_app{"asyncdb/get/request"};

            bool is_master;
            std::unordered_map<std::string,std::string> data;
            ngincc::core::plugin_manager& core_plug;
            ngincc::core::parallel::pipeline& core_pipe;

            //! \brief get value from local data-store
            int noasync_get(std::string& key, std::string& val);

            //! \brief start the db-master server
            int start(std::vector<std::string>& cmd_args, std::ostringstream& output);
            //! \brief dump db-data
            int db_dump(std::vector<std::string>& cmdargs, std::ostringstream& output);

            //! \addtogroup db-hooks
            //! @{
            int cas_hook(ngincc::core::buffer_coder& recv_buffer);
            int sin_hook(ngincc::core::buffer_coder& recv_buffer);
            int unset_hook(ngincc::core::buffer_coder& recv_buffer);
            int get_hook(ngincc::core::buffer_coder& recv_buffer);
            //! @}

            int reply_helper(uint32_t destpid, uint32_t cb_token, const std::string& cb_hook, uint32_t success, const std::string& key, const std::string& newval);
            bool cas_helper(const std::string& key, const std::string& newval, const std::string& expval);
            bool sin_helper(const std::string& key, const std::string& newval);
            bool unset_helper(const std::string& key);
            bool get_helper(const std::string& key, std::string& outval);

            //! allow others access the static strings
            friend async_db;
        };
    }
}

#endif // NGINCC_ASYNC_DB_MASTER_HXX
