//
// Created by young on 2023/2/21.
//

#ifndef DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_ADTS_H_
#define DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_ADTS_H_

#include <cstdint>
#include <string>
class ADTS
{
 public:
	ADTS(int profile, int id, int sample_rate, int channel, int pkt_size);
 private:
	void set_sampling_frequency_index(int sample_rate);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	//fixed header
	//1 byte
	uint8_t sync_word_l: 8;    //0xff
	//2 byte
	//sync_word_h + id + layer + protection_absent
	uint8_t protection_absent: 1;    // 1 no CRC, 0 has CRC
	uint8_t layer: 2;    //00
	uint8_t id: 1;    //0 MPEG-4 1:MPEG-2
	uint8_t sync_word_h: 4;

	//3rd byte
	// profile + sampling_frequency_index + private_bit + channel_configuration_l
	uint8_t channel_configuration_l: 1;    // ��������չ
	uint8_t private_bit: 1;        //0
	uint8_t sampling_frequency_index: 4; //�����ʱ�ʶ
	uint8_t profile: 2;  // 0:main, 1: LC, 2: SSR, 3: reserved

	// 4th byte
	uint8_t aac_frame_length_l: 2;    // ��ǰ��Ƶ֡���ֽ���������Ԫ�����ֽ��� + �ļ�ͷ�ֽ���(0 == protection_absent ? 7: 9)
	uint8_t copyright_identification_start: 1;
	uint8_t copyright_identification_bit: 1;
	uint8_t home: 1;
	uint8_t original_copy: 1;
	uint8_t channel_configuration_h: 2;    // ��������ʶ

	// 5th byte
	uint8_t aac_frame_length_m: 8;
	// 6th byte
	uint8_t adts_buffer_fullness_l: 5;    // ������Ϊ0x7FFʱ��ʾʱ�ɱ�����
	uint8_t aac_frame_length_h: 3;
	// 7th byte
	uint8_t number_of_raw_data_blocks_in_frame: 2;    // ��ǰ��Ƶ�������������Ƶ����֡���� ��Ϊ aac_nums - 1, ��ֻ��һ֡��Ƶʱ��0
	uint8_t adts_buffer_fullness_h: 6;  // adts_buffer_fullness 0x7ff VBR
#endif
};

#endif //DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_ADTS_H_
