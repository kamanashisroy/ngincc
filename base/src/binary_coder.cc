
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "log.hxx"
#include "ngincc_config.h"
//#include "module.hxx"
//#include "plugin_manager.hxx"
#include "binary_coder.hxx"

using std::string;
using std::overflow_error;
using ngincc::core::binary_coder;
using ngincc::core::buffer_coder;

buffer_coder::buffer_coder() {
    reset();
}

void buffer_coder::reset() {
    setp(internal_buffer.begin(),internal_buffer.end());
    //setg(&internal_buffer[0],&internal_buffer[0]);
    setg(data(),data(),data());
}

void buffer_coder::set_rd_length(unsigned int len) {
    if(len > internal_buffer.size()) {
        throw overflow_error("set_length buffer overflow");
    }
    setg(data(),data(),data()+len);
    //setg(&internal_buffer[0],&internal_buffer[len]);
}

unsigned int buffer_coder::capacity() const {
    return NGINZ_MAX_BINARY_MSG_LEN;
}

uint8_t* buffer_coder::data() {
    return internal_buffer.data();
}

std::streamsize buffer_coder::out_avail() const {
    return std::distance(pptr(),epptr());
}

std::char_traits<uint8_t>::pos_type buffer_coder::seekpos(
    std::char_traits<uint8_t>::pos_type pos
    , std::ios_base::openmode which) {
    if(std::ios_base::in & which) {
        if(pos > std::distance(data(),egptr())) {
            throw overflow_error("cannot set the read position");
        }
        setg(data()+pos,data(),egptr());
    }
    if(std::ios_base::out & which) {
        if(pos > std::distance(data(),epptr())) {
            throw overflow_error("cannot set the write position");
        }
        setp(data()+pos,epptr());
    }
    return pos;
}

binary_coder::binary_coder(buffer_coder &buffer) : std::basic_iostream<uint8_t>(&buffer) {
}

binary_coder& binary_coder::operator<<=(const uint32_t &intval) {
    uint8_t buf[5] = {0,0,0,0,0};
    int len = 1;
    // TODO host-to-network order
    if(intval >= 0xFFFF) {
        //buf[intbuf.len++] = /*(0<<6) |*/ 4; // 0 means numeral , 4 is the numeral size
        buf[len++] = (unsigned char)((intval & 0xFF000000)>>24);
        buf[len++] = (unsigned char)((intval & 0x00FF0000)>>16);
    //} else {
        //buf[intbuf.len++] = /*(0<<6) |*/ 2; // 0 means numeral , 2 is the numeral size
    }
    buf[len++] = (unsigned char)((intval & 0xFF00)>>8);
    buf[len++] = (unsigned char)(intval & 0x00FF);
    buf[0] = len-1;
    write(buf,len);
    return *this;
}

binary_coder& binary_coder::operator<<=(const string &strval) {
    uint8_t blen = strval.size();
    write(&blen,1);
    write((const unsigned char*)strval.c_str(),strval.size());
    return *this;
}

binary_coder& binary_coder::operator>>=(uint32_t &intval) {
    uint8_t blen = 0;
    read(&blen,1);
    if(blen > 4) {
        throw std::length_error("integer size is bigger than 32 bit");
    }
    //*intval = aroop_txt_to_int(&sandbox);
    intval = 0;
    uint8_t intbyte = 0;
    if(blen >= 1) {
        read(&intbyte,1);
        intval = intbyte;
    }
    if(blen >= 2) {
        read(&intbyte,1);
        intval = intval << 8;
        intval |= intbyte & 0xFF;
    }
    if(blen >= 3) {
        read(&intbyte,1);
        intval = intval << 8;
        intval |= intbyte & 0xFF;
    }
    if(blen == 4) {
        read(&intbyte,1);
        intval = intval << 8;
        intval |= intbyte & 0xFF;
    }
    return *this;
}

binary_coder& binary_coder::operator>>=(string &strval) {
    uint8_t blen = 0;
    read(&blen,1);
    strval = string(blen,'\0');
    read((unsigned char*)&strval[0],blen);
    return *this;
}

#define BINARY_CODER_LOG syslog

int binary_coder::self_test() {
    const uint32_t expval = 10;
    buffer_coder buffer;
    binary_coder outcoder(buffer);

    outcoder <<= binary_coder::canary_begin;
    outcoder <<= expval;
    outcoder <<= binary_coder::canary_end;

	BINARY_CODER_LOG(LOG_NOTICE, "binary_coder::self_test:wrote %s %d %s", binary_coder::canary_begin.c_str(), expval, binary_coder::canary_end.c_str());

    buffer.set_rd_length(buffer.out_avail());
    binary_coder incoder(buffer);

    string in_begin,in_end;
    uint32_t inval;
    
    if( !(incoder >>= in_begin)
        || !(incoder >>= inval)
        || !(incoder >>= in_end)) {
	    BINARY_CODER_LOG(LOG_NOTICE, "binary_coder::self_test:read-failed %s %d %s", in_begin.c_str(), inval, in_end.c_str());
        throw std::range_error("binary_coder self-test failed");
    }

	BINARY_CODER_LOG(LOG_NOTICE, "binary_coder::self_test:read %s %d %s", in_begin.c_str(), inval, in_end.c_str());

    if(in_begin != binary_coder::canary_begin
        || inval != expval
        || in_end != binary_coder::canary_end) {
        throw std::range_error("binary_coder self-test failed(mismatch)");
    }

    // reread
    buffer.seekpos(0, std::ios_base::in);

    if( !(incoder >>= in_begin)
        || !(incoder >>= inval)
        || !(incoder >>= in_end)) {
	    BINARY_CODER_LOG(LOG_NOTICE, "binary_coder::self_test:reread-failed %s %d %s", in_begin.c_str(), inval, in_end.c_str());
        throw std::range_error("binary_coder self-test(reread) failed");
    }

	BINARY_CODER_LOG(LOG_NOTICE, "binary_coder::self_test:reread %s %d %s", in_begin.c_str(), inval, in_end.c_str());

    if(in_begin != binary_coder::canary_begin
        || inval != expval
        || in_end != binary_coder::canary_end) {
        throw std::range_error("binary_coder self-test(reread) failed(mismatch)");
    }
    return 0;
}

#if 0

int binary_coder::debug_dump(aroop_txt_t*buffer) {
	int skip = 0;
	do {
		aroop_txt_t x = {};
		if(binary_unpack_string(buffer, skip, &x)) {
			return -1;
		}
		skip++;
		if(aroop_txt_is_empty(&x))
			break;
		aroop_txt_t sandbox = {};
		aroop_txt_embeded_stackbuffer(&sandbox, 32);
		aroop_txt_concat(&sandbox, &x);
		aroop_txt_zero_terminate(&sandbox);
		printf("%s\n", aroop_txt_to_string(&sandbox));
		aroop_txt_destroy(&x);
	}while(1);
	return 0;
}



static int binary_coder_test(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	if(binary_coder_test_helper(32) || binary_coder_test_helper(0)) {
		//aroop_txt_printf(output, "FAILED [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
		aroop_txt_printf(output, "FAILED\n");
	} else {
		aroop_txt_concat_string(output, "successful\n");
	}
	return 0;
}

static int binary_coder_test_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "binary_coder_test", "test", plugin_space, __FILE__, "It is test code for binary coder.\n");
}

int binary_coder_module_init() {
	aroop_txt_embeded_buffer(&intbuf, 32);
	aroop_txt_t binary_coder_plug;
	aroop_txt_embeded_set_static_string(&binary_coder_plug, "test/binary_coder_test");
	pm_plug_callback(&binary_coder_plug, binary_coder_test, binary_coder_test_desc);
	return 0;
}

int binary_coder_module_deinit() {
	pm_unplug_callback(0, binary_coder_test);
	return 0;
}

#endif // TODO


