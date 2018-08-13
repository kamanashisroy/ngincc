#ifndef NGINZ_LOAD_BALANCER_HXX
#define NGINZ_LOAD_BALANCER_HXX

#include <string>
#include "module.hxx"

namespace ngincc {
    namespace core {
        namespace parallel {
            // forward declaration
            class pipeline;
        }
    }
    namespace apps {
        //! \brief load balancer selects the next process in pipeline
        //!        to distribute the task probably evenly
        class load_balancer {
        public:
            virtual ~load_balancer() = 0;
            //! \brief get next node
            virtual int next() = 0;
        };
        class round_robin_load_balancer : public load_balancer {
        public:
            round_robin_load_balancer(ngincc::core::parallel::pipeline&);
            round_robin_load_balancer() = delete;
            virtual ~round_robin_load_balancer() override;
            //! \brief get next node
            virtual int next() override;
        private:
            ngincc::core::parallel::pipeline& pipe;
            int nid; //! worker/node id
            // int setup();
        };
    }
}

#endif // NGINZ_LOAD_BALANCER_HXX
