
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "ngincc_config.h"
//#include "module.hxx"
//#include "plugin_manager.hxx"
#include "binary_coder.hxx"

using std::string;
using std::overflow_error;
using ngincc::core::binary_coder;
using ngincc::core::buffer_coder;

string binary_coder::canary_begin = "[canary_end]";
string binary_coder::canary_end = "[canary_end]";

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
    write(buf,len+1);
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

static int binary_coder_test_helper(int expval) {
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, expval);
	binary_pack_int(&bin, expval);
	aroop_txt_t str = {};
	aroop_txt_embeded_set_static_string(&str, "test"); 
	binary_pack_string(&bin, &str);
	binary_coder_debug_dump(&bin);

	int intval = 0;
	int intval2 = 0;
	aroop_txt_t strval = {};
	binary_unpack_int(&bin, 0, &intval);
	binary_unpack_int(&bin, 1, &intval2);
	binary_unpack_string(&bin, 2, &strval);
	
	aroop_txt_zero_terminate(&strval);
	aroop_txt_zero_terminate(&str);
	//printf(" [%s!=%s] and [%d!=%d]\n", aroop_txt_to_string(&strval), aroop_txt_to_string(&str), intval, expval);
	return !(intval == expval && intval2 == expval && aroop_txt_equals(&strval, &str));
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


