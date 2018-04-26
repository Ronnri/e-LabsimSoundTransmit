#ifndef _G_711_ENCODE_H_
#define _G_711_ENCODE_H_


unsigned char ulaw2alaw(unsigned char uval);
unsigned char alaw2ulaw(unsigned char aval);
short ulaw2linear(unsigned char	u_val);
short alaw2linear(unsigned char	a_val);
unsigned char linear2ulaw(short pcm_val);
unsigned char linear2alaw(short pcm_val);
short swap_linear(short pcm_val);

extern int g711encode();

#endif

#pragma once
