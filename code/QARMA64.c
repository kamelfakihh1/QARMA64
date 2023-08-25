// Copyright (c) 2019-2022 Phantom1003
#include "QARMA64.h"

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