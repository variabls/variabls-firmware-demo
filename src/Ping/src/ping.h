/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __PING_H__
#define __PING_H__

#include <Arduino.h>

typedef void (* ping_recv_function)(void* arg, void *pdata);
typedef void (* ping_sent_function)(void* arg, void *pdata);

#if defined(ARDUINO_ARCH_ESP8266)

	#ifdef __cplusplus
	extern "C" {
	#endif

	struct ping_option{
		uint32 count;
		uint32 ip;
		uint32 coarse_time;
		ping_recv_function recv_function;
		ping_sent_function sent_function;
		void* reverse;
	};

	struct ping_resp{
		uint32 total_count;
		uint32 resp_time;
		uint32 seqno;
		uint32 timeout_count;
		uint32 bytes;
		uint32 total_bytes;
		uint32 total_time;
		sint8  ping_err;
	};

	bool ping_start(struct ping_option *ping_opt);
	bool ping_regist_recv(struct ping_option *ping_opt, ping_recv_function ping_recv);
	bool ping_regist_sent(struct ping_option *ping_opt, ping_sent_function ping_sent);

	#ifdef __cplusplus
	}
#endif

#elif defined(ARDUINO_ARCH_ESP32)

	struct ping_option {
		uint32_t count;
		uint32_t ip;
		uint32_t coarse_time;
		ping_recv_function recv_function;
		ping_sent_function sent_function;
		void* reverse;
	};

	struct ping_resp {
		uint32_t total_count;
		float resp_time;
		uint32_t seqno;
		uint32_t timeout_count;
		uint32_t bytes;
		uint32_t total_bytes;
		float total_time;
		int8_t  ping_err;
	};

	bool ping_start(struct ping_option *ping_opt);
	void ping(const char *name, int count, int interval, int size, int timeout);
	bool ping_start(IPAddress adr, int count, int interval, int size, int timeout, struct ping_option *ping_o = NULL);

#endif
	

#endif /* __PING_H__ */
