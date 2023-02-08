//
// Created by young on 2023/2/7.
//
#include "ts.h"

int parseTS(unsigned char* rtpPayload, TS_PACKET* tsPacket)
{
	tsPacket->sync_byte = rtpPayload[0];
	tsPacket->transport_error_indicator = rtpPayload[1] >> 7;
	tsPacket->payload_unit_start_indicator = (rtpPayload[1] & 0x40) >> 6;
	tsPacket->transport_priority = (rtpPayload[1] & 0x20) >> 5;
	tsPacket->PID = ((rtpPayload[1] & 0x1F) << 8) | rtpPayload[2];
	tsPacket->scrambling_control = rtpPayload[3] >> 6;
	tsPacket->adaptation_field_control = (rtpPayload[3] & 0x30) >> 4;
	tsPacket->continuity_counter = rtpPayload[3] & 0x0F;

	if (tsPacket->sync_byte != 0x47 || tsPacket->transport_error_indicator == 1)
	{
		perror(
			"rtp payload = mp2t(33) but sync_code != 0x47 or transport_error_indicator == 1");
		return -1;
	}
	//预留
	if (tsPacket->adaptation_field_control == 0)
	{
		perror("adaptation_field_control == 0");
		return -1;
	}

	//查找荷载
	//先跳过TS头
	unsigned char payloadOffset = 4;

	//有调节字段，跳过调节字段
	if (tsPacket->adaptation_field_control == 3)
	{
		//printf("TS Package has adaptation field\n");
		unsigned char adaptation_field_length = 0;
		memcpy(&adaptation_field_length, rtpPayload + payloadOffset, 1);
		//printf("adaptation_field_length:[%d]\n", adaptation_field_length);
		//还要加上第一个调节字节+1
		payloadOffset = payloadOffset + adaptation_field_length + 1;
	}

	tsPacket->payload = rtpPayload + payloadOffset;

	//荷载长度
	tsPacket->payloadLen = 188 - payloadOffset;

	return 0;
}

//判断PID是否时PMT
//PMT在传送流中用于指示组成某一套节目的视频、音频和数据在传送流中的位置，即对应的TS包的PID值，以及每路节目的节目时钟参考（PCR）字段的位置

int parsePES(unsigned char* payload, PES* pes)
{
	unsigned char* ptr = payload;

	pes->packet_start_code_prefix = (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
	if (pes->packet_start_code_prefix != 0x000001)
	{
		perror("not pes packet!!!");
		return -1;
	}
	pes->stream_id = ptr[3];
	pes->packet_length = (ptr[4] << 8) | ptr[5];

	pes->reserved_0 = (ptr[6] & 0xc0) >> 6;
	pes->scrambling_control = (ptr[6] & 0x30) >> 4;
	pes->priority = (ptr[6] & 0x08) >> 3;
	pes->data_alignment_indicator = (ptr[6] & 0x04) >> 2;
	pes->copyright = (ptr[6] & 0x02) >> 1;
	pes->original_or_copy = ptr[6] & 0x01;

	pes->pts_dts_flags = (ptr[7] & 0xc0) >> 6;
	pes->escr_flag = (ptr[7] & 0x20) >> 5;
	pes->es_rate_flag = (ptr[7] & 0x10) >> 4;
	pes->dsm_trick_mode_flag = (ptr[7] & 0x08) >> 3;
	pes->additional_copy_info_flag = (ptr[7] & 0x04) >> 2;
	pes->crc_flag = (ptr[7] & 0x02) >> 1;
	pes->extension_flag = ptr[7] & 0x01;

	pes->header_data_length = ptr[8];           /* 8 uimsbf*/
	pes->payload_offset = 9 + pes->header_data_length;

	//移动到任选字段
	ptr += 9;
	switch (pes->pts_dts_flags)
	{
	case 0x02:
		pes->pts = (ptr[0] & 0x0E) << 30 | ptr[1] << 22 | (ptr[2] & 0xFE) << 15 | ptr[3] << 7 | (ptr[4] & 0xFE) >> 1;
		pes->dts = 0;
		break;
	case 0x03:
		pes->pts = (ptr[0] & 0x0E) << 30 | ptr[1] << 22 | (ptr[2] & 0xFE) << 15 | ptr[3] << 7 | (ptr[4] & 0xFE) >> 1;
		ptr += 5;
		pes->dts = (ptr[0] & 0x0E) << 30 | ptr[1] << 22 | (ptr[2] & 0xFE) << 15 | ptr[3] << 7 | (ptr[4] & 0xFE) >> 1;
		break;
	default:
		pes->pts = 0;
		pes->dts = 0;
	}
	return 0;
}

int parseSDT(TS_PACKET* tsPacket)
{
	//SDT or BAT
	SDT sdt = { 0 };
	memcpy(&sdt, tsPacket->payload, 11);
	int CRC_32;
	//-4 ,减去CRC_32，指针移动到CRC_32前
	memcpy(&CRC_32, tsPacket->payload + sdt.section_length - 4, sizeof(CRC_32));
	sdt.CRC_32 = CRC_32;
	//BAT
	if (sdt.table_id == 0x4a)
	{
		printf("BAT\n");
		return -1;
	}


	//program
	char* crc_32 = tsPacket->payload + sdt.section_length - 4;
	tsPacket->payload += 11;
	//for (int n = 0; n < sdt.section_length - 4 - 8; n += 4)
	while (crc_32 != tsPacket->payload)
	{
		SDT_Program* sdtProgram = malloc(sizeof(SDT_Program));
		memcpy(sdtProgram, tsPacket->payload, 5);
		tsPacket->payload += 5;

		for (int i = 0; i < sdtProgram->descriptors_loop_length; ++i)
		{
			SDT_Program_desc* sdtProgramDesc = malloc(sizeof(SDT_Program_desc));
			memcpy(sdtProgramDesc, tsPacket->payload, 4);
			tsPacket->payload += 4;
			unsigned char* provider_name = malloc(sdtProgramDesc->Service_provider_name_length);
			memcpy(provider_name, tsPacket->payload, sdtProgramDesc->Service_provider_name_length);
			tsPacket->payload += sdtProgramDesc->Service_provider_name_length;
			sdtProgramDesc->Service_provider_name = provider_name;

			unsigned char service_name_len;
			memcpy(&service_name_len, tsPacket->payload, 1);
			tsPacket->payload += 1;
			sdtProgramDesc->Service_name_length = service_name_len;
			unsigned char* service_name = malloc(sdtProgramDesc->Service_name_length);
			memcpy(service_name, tsPacket->payload, sdtProgramDesc->Service_name_length);
			tsPacket->payload += sdtProgramDesc->Service_name_length;
			sdtProgramDesc->Service_name = service_name;

			sdtProgram->N_loop[sdtProgram->N_loop_len] = sdtProgramDesc;
			sdtProgram->N_loop_len++;
		}
		sdt.N_loop[sdt.N_loop_len] = sdtProgram;
		sdt.N_loop_len++;
	}
}

int parsePAT(unsigned char* payload, PAT* pPat)
{
	pPat->table_id = payload[0];
	//PAT固定为0x00
	if (pPat->table_id != 0x00)
	{
		perror("PAT table_id is not 0x00");
		return -1;
	}
	pPat->section_syntax_indicator = payload[1] >> 7;
	pPat->reserved_1 = (payload[1] & 0x70) >> 4;
	pPat->section_length = ((payload[1] & 0x000F) << 8) | payload[2];
	pPat->transport_stream_id = (payload[3] << 8) | payload[4];
	pPat->reserved_2 = (payload[5] & 0xc0) >> 6;
	pPat->version_number = (payload[5] & 0x3E) >> 1;
	pPat->current_next_indicator = payload[5] & 0x01;
	pPat->section_number = payload[6];
	pPat->last_section_number = payload[7];
	unsigned int CRC_32;
	//+3移动到下一位，-4 ,减去CRC_32，指针移动到CRC_32前
	memcpy(&CRC_32, payload + 3 + pPat->section_length - 4, sizeof(CRC_32));
	pPat->CRC_32 = ntohl(CRC_32);


	//长度为pPat->section_length - crc_32长度 - loop之前字段长度。
	pPat->program_len = pPat->section_length - 4 - 5;
	pPat->program = malloc(pPat->program_len);

	//固定4字节
	unsigned char* N_loop = payload + 8;
	for (int i = 0, index = 0; i < pPat->program_len; i += 4)
	{
		unsigned program_num = N_loop[i] << 8 | N_loop[i + 1];
		unsigned reserved = N_loop[i + 2] >> 5;
		unsigned nidOrMid = (N_loop[i + 2] & 0X1F) << 8 | N_loop[i + 3];

		//NIT
		if (program_num == 0x00)
		{
			pPat->program_number = program_num;
			pPat->reserved_3 = reserved;
			pPat->network_PID = nidOrMid;
			printf(" packet->network_PID %0x /n/n", pPat->network_PID);
		}
		else
		{
			TS_PAT_Program tsPatProgram = { 0 };
			tsPatProgram.program_number = program_num;
			tsPatProgram.reserved = reserved;
			tsPatProgram.program_map_PID = nidOrMid;
			memcpy(pPat->program + index, &tsPatProgram, 4);
			index++;
		}
	}

	return 0;
}

int parsePMT(unsigned char* payload, PMT* pPMT)
{
	pPMT->table_id = payload[0];
	if (pPMT->table_id != 0x02)
		return -1;

	pPMT->section_syntax_indicator = payload[1] >> 7;
	pPMT->reserved_1 = (payload[1] & 0x70) >> 4;
	pPMT->section_length = ((payload[1] & 0x000F) << 8) | payload[2];
	pPMT->program_number = (payload[3] << 8) | payload[4];
	pPMT->reserved_2 = (payload[5] & 0xc0) >> 6;
	pPMT->version_number = (payload[5] & 0x3E) >> 1;
	pPMT->current_next_indicator = payload[5] & 0x01;
	pPMT->section_number = payload[6];
	pPMT->last_section_number = payload[7];
	pPMT->PCR_PID = ((payload[8] & 0x1F) << 8) | payload[9];
	pPMT->program_info_length = ((payload[10] & 0X0F) << 8) | payload[11];
	unsigned int CRC_32;
	//+3移动到下一位，+ pPMT->section_length移动到末尾，-4减去CRC_32，指针移动到CRC_32前
	memcpy(&CRC_32, payload + 3 + pPMT->section_length - 4, sizeof(CRC_32));
	pPMT->CRC_32 = ntohl(CRC_32);

	//长度为pPMT->section_length - crc_32长度 - program_info_length之前字段长度 -program_info_length的长度。
	pPMT->pmt_stream_len = pPMT->section_length - 4 - 9 - pPMT->program_info_length;
	pPMT->pmt_stream = malloc(pPMT->pmt_stream_len);

	unsigned char* N_loop = payload + 8 + 4 + pPMT->program_info_length;
	unsigned int index = 0;
	for (int i = 0; i < pPMT->pmt_stream_len;)
	{
		PMT_STREAM pmtStream;
		pmtStream.stream_type = N_loop[i + 0];
		pmtStream.elementary_PID = (N_loop[i + 1] & 0x1F) << 8 | N_loop[i + 2];
		pmtStream.ES_info_length = (N_loop[i + 3] & 0x0F) << 8 | N_loop[i + 4];

		memcpy(pPMT->pmt_stream + index, &pmtStream, 5 + pmtStream.ES_info_length);
		index++;
		i = i + 5 + pmtStream.ES_info_length;
	}
	return 0;
}

int parseES()
{
	return 0;
}

