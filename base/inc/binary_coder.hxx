#ifndef NGINZ_BINARY_CODER_HXX
#define NGINZ_BINARY_CODER_HXX

#include <tuple>
#include <iterator>

namespace ngincc {
    namespace core {
        class buffer_coder : public std::basic_streambuf<uint8_t> {
        public:
            buffer_coder();
            //! \brief make the buffer empty and position the reader/writer to beginning
            void reset(); 
            void set_rd_length(unsigned int len);
            uint8_t* data();
            unsigned int capacity() const;
            std::streamsize out_avail() const;
        protected:
            //virtual int_type underflow() override;
            //virtual int_type overflow(int_type ch) override;
        private:
            //uint8_t internal_buffer[NGINZ_MAX_BINARY_MSG_LEN];
            std::array<uint8_t,NGINZ_MAX_BINARY_MSG_LEN> internal_buffer;
        };
        class binary_coder : public std::basic_iostream<uint8_t> {
        public:
            binary_coder(buffer_coder &buffer);
            binary_coder& operator<<=(const uint32_t &intval);
            binary_coder& operator<<=(const std::string &strval);
            binary_coder& operator>>=(uint32_t &intval);
            binary_coder& operator>>=(std::string &strval);
            static std::string canary_begin;
            static std::string canary_end;
            // TODO debug-dump and testing
        };
    }
}

#endif // NGINZ_BINARY_CODER_HXX
