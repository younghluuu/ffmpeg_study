//
// Created by young on 2023/2/1.
//

#ifndef H264DECODE_H264DECODER_NALUPPS_H_
#define H264DECODE_H264DECODER_NALUPPS_H_

#include "Nalu.h"
class NaluPPS : public Nalu
{
 public:
	explicit NaluPPS(Nalu& nalu);
	int Parse() override;
};

#endif //H264DECODE_H264DECODER_NALUPPS_H_
