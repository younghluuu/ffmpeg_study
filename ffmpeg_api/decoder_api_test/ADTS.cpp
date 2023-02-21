//
// Created by young on 2023/2/21.
//

#include <cstring>
#include "ADTS.h"
ADTS::ADTS(int profile, int id, int sample_rate, int channel, int pkt_size)
{
	memset(this, 0, sizeof(ADTS));
	int length = pkt_size + 7;

	sync_word_l = 0xff;

	sync_word_h = 0xf;
	this->id = id;
	layer = 0;
	protection_absent = 1;

	this->profile = profile;
	set_sampling_frequency_index(sample_rate);
	private_bit = 0;
	channel_configuration_l = 0;

	channel_configuration_h = channel;
	original_copy = 0;
	home = 0;
	copyright_identification_bit = 0;
	copyright_identification_start = 0;
	aac_frame_length_l = (length >> 11) & 0x03;

	aac_frame_length_m = (length >> 3) & 0x00ff;

	aac_frame_length_h = length & 0x0007;
	adts_buffer_fullness_l = 0x1f;

	adts_buffer_fullness_h = 0xfc >> 2;
	number_of_raw_data_blocks_in_frame = 0;
}

void ADTS::set_sampling_frequency_index(int sample_rate)
{
	//默认48000hz
	sampling_frequency_index = 3;
	const int sampling_frequencies[13] = {
		96000,  // 0x0
		88200,  // 0x1
		64000,  // 0x2
		48000,  // 0x3
		44100,  // 0x4
		32000,  // 0x5
		24000,  // 0x6
		22050,  // 0x7
		16000,  // 0x8
		12000,  // 0x9
		11025,  // 0xa
		8000,  // 0xb
		7350
		// 0xc d e f是保留的
	};
	for (int i = 0; i < 13; ++i)
	{
		if (sampling_frequencies[i] == sample_rate)
		{
			sampling_frequency_index = i;
			break;
		}
	}
}