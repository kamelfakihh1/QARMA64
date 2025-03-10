// Copyright (c) 2019-2022 Phantom1003
#include "QARMA64.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>

#define RANDOM_PATH "/dev/urandom"

namespace QARMA64 {

	int m = MAX_LENGTH / 16;

	const_t alpha = 0xC0AC29B7C97C50DD;
	const_t c[8] = { 0x0000000000000000, 0x13198A2E03707344, 0xA4093822299F31D0, 0x082EFA98EC4E6C89,
					0x452821E638D01377, 0xBE5466CF34E90C6C, 0x3F84D5B5B5470917, 0x9216D5D98979FB1B };

	int sbox[16] = {11,  6,  8, 15, 12,  0,  9, 14,  3,  7,  4,  5, 13,  2,  1, 10};

	int sbox_inv[16] = { 5, 14, 13,  8, 10, 11,  1,  9,  2,  6, 15,  0,  4, 12,  7,  3};

	int t[16] = { 0, 11,  6, 13, 10,  1, 12,  7,  5, 14,  3,  8, 15,  4,  9,  2 };
	int t_inv[16] = { 0,  5, 15, 10, 13,  8,  2,  7, 11, 14,  4,  1,  6,  3,  9, 12 };
	int h[16] = { 6,  5, 14, 15,  0,  1,  2,  3,  7, 12, 13,  4,  8,  9, 10, 11 };
	int h_inv[16] = { 4,  5,  6,  7, 11,  1,  0,  8, 12, 13, 14, 15,  9, 10,  2,  3 };

	#define Q M
	#define M_inv M

	cell_t M[16] = { 0, 1, 2, 1,
					1, 0, 1, 2,
					2, 1, 0, 1,
					1, 2, 1, 0 };

	void text2cell(cell_t* cell, text_t is) {
		// for 64 bits
		char* byte_ptr = (char*)&is;
		for (int i = 0; i < MAX_LENGTH / 8; i++) {
			char byte = byte_ptr[i];
			cell[2 * (7 - i) + 0] = (byte & 0xF0) >> 4;
			cell[2 * (7 - i) + 1] = byte & 0xF;
		}
	}

	text_t cell2text(cell_t* cell) {
		text_t is = 0;
		for (int i = 0; i < MAX_LENGTH / 8; i++) {
			text_t byte = 0;
			byte = (cell[2 * i] << 4) | cell[2 * i + 1];
			is = is | (byte << (7 - i) * 8UL);
		}
		return is;
	}

	text_t pseudo_reflect(text_t is, key_t tk) {
		cell_t cell[16];
		text2cell(cell, is);

		// ShuffleCells
		cell_t perm[16];
		for (int i = 0; i < 16; i++)
			perm[i] = cell[t[i]];

		// MixColumns
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				cell_t temp = 0;
				for (int j = 0; j < 4; j++) {
					int b;
					if (b = Q[4 * x + j]) {
						cell_t a = perm[4 * j + y];
						temp ^= ((a << b) & 0x0F) | (a >> (4 - b));
					}
				}
				cell[4 * x + y] = temp;
			}
		}

		// AddRoundTweakey
		for (int i = 0; i < 16; i++)
			cell[i] ^= (tk >> (4 * (15 - i))) & 0xF;

		// ShuffleCells invert
		for (int i = 0; i < 16; i++)
			perm[i] = cell[t_inv[i]];

		return cell2text(perm);
	}

	text_t forward(text_t is, key_t tk, int r) {
		

		is ^= tk;
		cell_t cell[16];
		text2cell(cell, is);
		


		if (r != 0) {
			// ShuffleCells
			cell_t perm[16];
			for (int i = 0; i < 16; i++)
				perm[i] = cell[t[i]];

			// MixColumns
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; y++) {
					cell_t temp = 0;
					for (int j = 0; j < 4; j++) {
						int b;
						if (b = M[4 * x + j]) {
							cell_t a = perm[4 * j + y];
							temp ^= ((a << b) & 0x0F) | (a >> (4 - b));
						}
					}
					cell[4 * x + y] = temp;
				}
			}
		}

		// SubCells
		for (int i = 0; i < 16; i++) {
			cell[i] = subcells[cell[i]];
		}
		is = cell2text(cell);
		

		return is;
	}

	text_t backward(text_t is, key_t tk, int r) {
		cell_t cell[16];
		text2cell(cell, is);
		

		// SubCells
		for (int i = 0; i < 16; i++) {
			cell[i] = subcells_inv[cell[i]];
		}
		

		if (r != 0) {
			cell_t mixc[16];
			// MixColumns
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; y++) {
					cell_t temp = 0;
					for (int j = 0; j < 4; j++) {
						int b;
						if (b = M_inv[4 * x + j]) {
							cell_t a = cell[4 * j + y];
							temp ^= ((a << b) & 0x0F) | (a >> (4 - b));
						}
					}
					mixc[4 * x + y] = temp;
				}
			}

			// ShuffleCells
			for (int i = 0; i < 16; i++)
				cell[i] = mixc[t_inv[i]];
		}

		is = cell2text(cell);
		is ^= tk;
		
		return is;
	}

	cell_t LFSR(cell_t x) {
		cell_t b0 = (x >> 0) & 1;
		cell_t b1 = (x >> 1) & 1;
		cell_t b2 = (x >> 2) & 1;
		cell_t b3 = (x >> 3) & 1;

		return ((b0 ^ b1) << 3) | (b3 << 2) | (b2 << 1) | (b1 << 0);
	}

	cell_t LFSR_inv(cell_t x) {
		cell_t b0 = (x >> 0) & 1;
		cell_t b1 = (x >> 1) & 1;
		cell_t b2 = (x >> 2) & 1;
		cell_t b3 = (x >> 3) & 1;

		return ((b0 ^ b3) << 0) | (b0 << 1) | (b1 << 2) | (b2 << 3);
	}

	key_t forward_update_key(key_t T) {
		cell_t cell[16], temp[16];
		text2cell(cell, T);

		// h box
		for (int i = 0; i < 16; i++) {
			temp[i] = cell[h[i]];
		}

		// w LFSR
		temp[0] = LFSR(temp[0]);
		temp[1] = LFSR(temp[1]);
		temp[3] = LFSR(temp[3]);
		temp[4] = LFSR(temp[4]);
		temp[8] = LFSR(temp[8]);
		temp[11] = LFSR(temp[11]);
		temp[13] = LFSR(temp[13]);

		return cell2text(temp);
	}

	key_t backward_update_key(key_t T) {
		cell_t cell[16], temp[16];
		text2cell(cell, T);

		// w LFSR invert
		cell[0] = LFSR_inv(cell[0]);
		cell[1] = LFSR_inv(cell[1]);
		cell[3] = LFSR_inv(cell[3]);
		cell[4] = LFSR_inv(cell[4]);
		cell[8] = LFSR_inv(cell[8]);
		cell[11] = LFSR_inv(cell[11]);
		cell[13] = LFSR_inv(cell[13]);

		// h box
		for (int i = 0; i < 16; i++) {
			temp[i] = cell[h_inv[i]];
		}

		return cell2text(temp);
	}

	text_t qarma64_enc(text_t plaintext, tweak_t tweak, key_t w0, key_t k0, int rounds) {
		key_t w1 = ((w0 >> 1) | (w0 << (64 - 1))) ^ (w0 >> (16 * m - 1));
		key_t k1 = k0;

		text_t is = plaintext ^ w0;

		for (int i = 0; i < rounds; i++) {
			
			is = forward(is, k0 ^ tweak ^ c[i], i);
			tweak = forward_update_key(tweak);
		}

		
		is = forward(is, w1 ^ tweak, 1);
		is = pseudo_reflect(is, k1);
		is = backward(is, w0 ^ tweak, 1);

		for (int i = rounds - 1; i >= 0; i--) {
			
			tweak = backward_update_key(tweak);
			is = backward(is, k0 ^ tweak ^ c[i] ^ alpha, i);
		}

		is ^= w1;
		
		return is;
	}

	text_t qarma64_dec(text_t plaintext, tweak_t tweak, key_t w0, key_t k0, int rounds) {

		key_t w1 = w0;
		w0 = ((w0 >> 1) | (w0 << (64 - 1))) ^ (w0 >> (16 * m - 1));

		cell_t k0_cell[16], k1_cell[16];
		text2cell(k0_cell, k0);
		// MixColumns
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				cell_t temp = 0;
				for (int j = 0; j < 4; j++) {
					int b;
					if (b = Q[4 * x + j]) {
						cell_t a = k0_cell[4 * j + y];
						temp ^= ((a << b) & 0x0F) | (a >> (4 - b));
					}
				}
				k1_cell[4 * x + y] = temp;
			}
		}

		key_t k1 = cell2text(k1_cell);

		k0 ^= alpha;
		text_t is = plaintext ^ w0;

		for (int i = 0; i < rounds; i++) {
			
			is = forward(is, k0 ^ tweak ^ c[i], i);
			tweak = forward_update_key(tweak);
		}

		
		is = forward(is, w1 ^ tweak, 1);
		is = pseudo_reflect(is, k1);
		is = backward(is, w0 ^ tweak, 1);

		for (int i = rounds - 1; i >= 0; i--) {
			
			tweak = backward_update_key(tweak);
			is = backward(is, k0 ^ tweak ^ c[i] ^ alpha, i);
		}

		is ^= w1;	
		return is;
	}

	// generate random 128-bit key and returns it using struct Int128 
	struct Int128 generate_key(){

		// generate 128 bit value and split it into k0 and w0
		unsigned char buf[16];
		unsigned long v;
		FILE *fin;    

		Int128 k;

		if ((fin = fopen(RANDOM_PATH, "r")) == NULL) {
				fprintf(stderr, "%s: unable to open file\n", RANDOM_PATH);
				return {0,0};
		}
		if (fread(buf, 1, sizeof(buf), fin) != sizeof(buf)) {
				fprintf(stderr, "%s: not enough bytes (expected %u)\n",
						RANDOM_PATH, (unsigned) sizeof buf);
				return {0,0};
		}
		fclose(fin);
		k.w0 = 0;
		for(int i = 0; i<8; i++){
			k.w0 |= (((uint64_t) buf[i]) << i*8);		
		}

		k.k0 = 0;
		for(int i = 0; i<8; i++){
			k.k0 |= (((uint64_t) buf[i+8]) << i*8);
		}

		return k;
	}

	// adds PAC to a pointer
	text_t sign_pointer(text_t pointer, tweak_t tweak, key_t k0, key_t w0){	

		text_t ciphertext;
		text_t signed_pointer;
		text_t pac;		

		pointer = pointer & 0xFFFFFFFFFFFF; 
		ciphertext = qarma64_enc(pointer, tweak, w0, k0, 3);	

		// truncate PAC to only 16 bits of the ciphertext	
		pac = ciphertext & 0xFFFF;
		signed_pointer = pointer | (pac << 48);
		
		return signed_pointer;	
	}

	// verifies the integrity of the PAC associated with a pointer and 
	// returns the original pointer if the PAC is valid
	text_t verify_pointer(text_t signed_pointer, tweak_t tweak, key_t k0, key_t w0){
		
		text_t ciphertext;
		text_t pac;

		// generate PAC 
		text_t pointer = signed_pointer & 0xFFFFFFFFFFFF;
		ciphertext = qarma64_enc(pointer, tweak, w0, k0, 3);	
		pac = ciphertext & 0xFFFF;

		// compare the generated PAC with the one already in the pointer
		if((pointer | (pac << 48)) == signed_pointer){		
			return pointer;
		}else{		
			// causes a segmentation fault
			return 0xFFFFFFFFFFFFFFFF;
		}
	}

}