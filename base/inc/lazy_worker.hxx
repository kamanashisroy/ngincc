#ifndef NGINZ_LAZY_CALL_HXX
#define NGINZ_LAZY_CALL_HXX

namespace ngincc {
    namespace core {
        class lazy_worker {
        public:
            lazy_worker(ngincc::core::worker &);
            lazy_worker() = delete;
            ~lazy_worker();

            //! \brief It executes the task after the execution of current fiber.
            //! In fact it is executed in separate fiber.
            int add_future_task(std::function<int()>&) /*throw(std::overflow_error) */;
            //! \sa add_lazy_task 
            int add_future_task(std::function<int()> &task,std::function<int(int)> &when_complete) /* throw(std::overflow_error) */ ;
        private:
            struct future_task {
                std::function<int()> task;
                std::function<int(int )> when_complete;
                future_task();
            };
            int lazy_stack_count;
            struct future_task lazy_stack[NGINZ_LAZY_STACK_SIZE];
        };
    }
}

#endif // NGINZ_LAZY_CALL_HXX
