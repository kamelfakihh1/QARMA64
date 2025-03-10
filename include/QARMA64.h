#ifndef QARMA_64_H
#define QARMA_64_H

// Copyright (c) 2019-2022 Phantom1003

#include <iostream>
#include <cstdio>
#include <cstdlib>

#define MAX_LENGTH 64
#define subcells sbox
#define subcells_inv sbox_inv

namespace QARMA64 {

	typedef unsigned long long int const_t;
	typedef unsigned long long int tweak_t;
	typedef unsigned long long int text_t;
	typedef unsigned long long int key_t;
	typedef unsigned char          cell_t;

	struct Int128 {
		uint64_t w0;
		uint64_t k0;
	};	

	void text2cell(cell_t* cell, text_t is);

	text_t cell2text(cell_t* cell);

	text_t pseudo_reflect(text_t is, key_t tk);

	text_t forward(text_t is, key_t tk, int r);

	text_t backward(text_t is, key_t tk, int r);

	cell_t LFSR(cell_t x);

	cell_t LFSR_inv(cell_t x);

	key_t forward_update_key(key_t T);

	key_t backward_update_key(key_t T);

	text_t qarma64_enc(text_t plaintext, tweak_t tweak, key_t w0, key_t k0, int rounds);

	text_t qarma64_dec(text_t plaintext, tweak_t tweak, key_t w0, key_t k0, int rounds);

	struct Int128 generate_key();

	text_t sign_pointer(text_t pointer, tweak_t tweak, key_t k0, key_t w0);

	text_t verify_pointer(text_t pointer, tweak_t tweak, key_t k0, key_t w0);
}

#endif