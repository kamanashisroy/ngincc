#ifndef NGINZ_BINARY_CODER_HXX
#define NGINZ_BINARY_CODER_HXX

#include <tuple>
#include <iterator>

#include "ngincc_config.h"

namespace ngincc {
    namespace core {
        // TODO rename buffer_coder to buffer_stream
        class buffer_coder : public std::basic_streambuf<uint8_t> {
        public:
            buffer_coder();
            //! \brief make the buffer empty and position the reader/writer to beginning
            void reset(); 
            void set_rd_length(unsigned int len);
            uint8_t* data();
            unsigned int capacity() const;
            std::streamsize out_avail() const;
            virtual std::char_traits<uint8_t>::pos_type seekpos(
                std::char_traits<uint8_t>::pos_type pos
                , std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
            std::basic_streambuf<char>* rdbuf();
        protected:
            //virtual int_type underflow() override;
            //virtual int_type overflow(int_type ch) override;
        private:
            //uint8_t internal_buffer[NGINZ_MAX_BINARY_MSG_LEN];
            std::array<uint8_t,NGINZ_MAX_BINARY_MSG_LEN> internal_buffer;
        };
        // TODO rename binary_coder to binary_iostream 
        class binary_coder : public std::basic_iostream<uint8_t> {
        public:
            binary_coder(buffer_coder &buffer);
            binary_coder& operator<<=(const uint32_t &intval);
            binary_coder& operator<<=(const std::string &strval);
            binary_coder& operator>>=(uint32_t &intval);
            binary_coder& operator>>=(std::string &strval);
            static int self_test();
            static inline std::string canary_begin {"canary_begin"};
            static inline std::string canary_end {"canary_end"};
            // TODO debug-dump
        };
    }
}

#endif // NGINZ_BINARY_CODER_HXX
