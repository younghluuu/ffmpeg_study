//
// Created by young on 2023/2/6.
//

#ifndef UDP_RTP_UDP_RTP_TS_H_
#define UDP_RTP_UDP_RTP_TS_H_
/*
	TS包固定大小为188字节，分为TS header、Adaptation Field、Payload
	TS Header固定4个字节；Adaptation Field可能存在也可能不存在，主要作用是给不足188字节的数据做填充；Payload是PES数据。
  	TS Header 协议格式如下，协议为 4 byte。
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |	sync byte  |T|P|T| 			PID			   |SC |AF |	CC |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct MPEGTS_FIXED_HEADER_BIT
{
	//同步字节，固定为0x47，
	unsigned sync_byte: 8;
	//传输错误标志。 值为1表示在相关的传输包中至少有一个不可纠正的错误位。当被置1后，在错误被纠正之前不能重置为0
	unsigned transport_error_indicator: 1;
	//负载起始标志标识。为1时，表示当前TS包的有效载荷中包含PES或者PSI头的起始位置；
	//1+[length],最后多了一个point, 在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。因此有效载荷开始的位置应再偏移1+[length]个字节
	//如果是PSI/SI的话，会多一个point.PointField 指出了 SI、PSI 表头在有效净荷中的具体位置
	unsigned payload_unit_start_indicator: 1;
	//传输优先级标志。1’表明当前TS包的优先级比其他具有相同PID， 但此位没有被置‘1’的TS包高。
	unsigned transport_priority: 1;
	//指示存储与分组有效负载中数据的类型。
	//PAT一般对应的pid为0x0，而在PAT中会指明PMT的pid。
	//而解析完PMT的时候，他也会指明对应接收的音视频的pid。而之后接收的ts包中就是根据这个PID来区分音视频，并从中取出对应的数据放到PES的数据中。
	//PID值0x0000—0x000F保留。其中0x0000为PAT保留；0x0001为CAT保留；0x1fff为分组保留，即空包
	/*
	 * PID值			描述
		00				PAT(Program Association Table)
		01				CAT(Conditional Access Table)
	 	11				SDT主要用于视频的描述信息，比如创建者是ffmpeg,注意：不是一个pid=0x11了就一定是SDT，还得看table-id,若table-id,=0x4A，则是BAT表
		3-0xF			Reserved
		0x10-0x1FFE		自定义PID，可用于PMT的pid、network的pid或者其他目标
		0x1FFF			空包
	 */
	unsigned PID: 13;
	//加扰控制标志。表示TS流分组有效负载的加密模式。空包为‘00’，如果传输包包头中包括调整字段，不应被加密。其他取值含义是用户自定义的。
	//一般用来标识加密使用。
	unsigned scrambling_control: 2;
	//自适应区标识。表示包头是否有调整字段或有效负载。√√√√√
	//在ts包中，payload 区可能是数据，也有可能不是数据，当不是数据的时候就需要adaptation field来填充0xff来达到188字节
	//而对于PAT和PMT来说是没有adaptation field 的。不够的长度直接补0xff即可
	//同时可以通过adaptation field control为不同的值来区分payload中为什么样的值。
	// 0x00 :  为ISO/IEC未来使用保留
	// 0x01 ： proload为纯数据（为音视频数据）.含有效载荷，无调整字段
	// 0x02 ： 为adaptation field数据，无有效载荷，仅含调整字段,空包应为0x2
	// 0x03 ： 既有音视频数据又有adaptation field数据。
	unsigned adaptation_field_control: 2;
	//计数器.随着每一个具有相同PID的TS流分组而增加，当它达到最大值后又回复到0。范围为0~15。
	//0x02,表示当前传送的相同类型的包是第3个
	unsigned continuity_counter: 4;
} MPEGTS_FIXED_HEADER_BIT;

typedef struct TS_PACKET
{
	unsigned char sync_byte;
	unsigned char transport_error_indicator;
	unsigned char payload_unit_start_indicator;
	unsigned char transport_priority;
	unsigned short PID;
	unsigned char scrambling_control;
	unsigned char adaptation_field_control;
	unsigned char continuity_counter;

	unsigned char* payload;
	int payloadLen;
} TS_PACKET;

//11byte + n loop + 4
typedef struct SDT
{
	unsigned table_id: 8;
	unsigned section_syntax_indicator: 1;
	//固定为0
	unsigned zero: 1;
	unsigned reserved_1: 2;
	unsigned section_length: 12;
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	unsigned version_number: 5;
	unsigned current_next_indicator: 1;
	unsigned section_number: 8;
	unsigned last_section_number: 8;

	unsigned original_network_id: 16;
	unsigned reserved_future_use: 8;

	struct SDT_Program** N_loop;
	unsigned int N_loop_len;

	//校验码
	unsigned CRC_32: 32;
} SDT;

//固定大小为5
typedef struct SDT_Program
{
	//服务器ID,实际上就是PMT段中的program_number
	unsigned service_id: 16;
	unsigned reserved_future_used: 6;
	//1表示当前流实现了该节目的EIT传送
	unsigned EIT_schedule_flag: 1;
	//1表示当前流实现了该节目的EIT传送
	unsigned EIT_present_following_flag: 1;
	//运行状态信息.1-还未播放2-几分钟后马上开始,3-被暂停播出,4-正在播放,其他---保留
	unsigned running_status: 3;
	//''1''表示该节目被加密. 紧 接着的是描述符,一般是Service descriptor,
	// 分析此描述符可以获取servive_id指定的节目的节目名称.具体格式请参考 EN300468中的Service descriptor部分.
	unsigned free_CA_mode: 1;
	//当前service_id所有描述子的长度
	unsigned descriptors_loop_length: 12;

	struct SDT_Program_desc** N_loop;
	unsigned int N_loop_len;
} SDT_Program;

typedef struct SDT_Program_desc
{
	unsigned Descriptor_tag: 8;
	unsigned Descriptor_length: 8;
	unsigned Service_type: 8;
	unsigned Service_provider_name_length: 8;
	unsigned char* Service_provider_name;
	unsigned Service_name_length: 8;
	unsigned char* Service_name;
} SDT_Program_desc;

//TS包中Payload所传输的信息包括两种类型：视频、音频的PES包以及辅助数据；节目专用信息PSI。
/*			PSI
 * 	节目关联表Program Association Table (PAT) 0x0000				PAT中会指明PMT的pid
 *	节目映射表Program Map Tables (PMT)							PMT中定义了与特定节目相关的PID信息，比如音频包pid、视频包pid以及pcr的pid
 *	条件接收表Conditional Access Table (CAT) 0x0001				CAT表格用于流加扰情况下配置参数
 *	TS_program_map_section	0x02
 *	TS_description_section	0x03
 *	ISO_IEC_14496_scene_description_section	0x04
 *	ISO_IEC_14496_object_descriptor_section	0x05
 *	ITU-T Rec. H.222.0 / ISO/IEC 13818-1 reserved	0x06~0x37
 *	Defined in ISO/IEC 13818-6	0x38~0x3F
 *	User private	0x40~0xFE
 *	forbidden		0XFF
 *	网络信息表Network Information Table(NIT) 0x0010				NIT是可选的，标准中未详细定义
 *	传输流描述表Transport Stream Description Table(TSDT) 0x02		TSDT也是可选的
 */
//视频、音频的ES流需进行打包形成视频、音频的 PES流。辅助数据（如图文电视信息）不需要打成PES包。
//adaptation_field_control为0x1或者0x3时，通常是0x1，而当PES首个包或最后一个包时，可能为0x3。有效荷载


typedef struct TS_PAT_Program
{
	unsigned program_number: 16;
	unsigned reserved: 3;
	unsigned program_map_PID: 13;
} TS_PAT_Program;

//该结构占用12字节，N loop = section_length - 12 .
//先分析PAT.TS流中会定期出现PAT表。PAT表提供了节目号和对应PMT表格的PID的对应关系。
//节目关联表(PAT Program Association Table)是数字电视系统中节目指示的根节点。
// 其包标识符(Packet IDentifier、简称PID)为0。终端设备（如机顶盒）搜索节目时最先都是从这张表开始搜索的。
// 从PAT中解析出节目映射表(PMT)，再从PMT解析出基本元素(如视频、音频、数据等)的PID及节目号、再根据节目从节目业务描述表(Service Description Table、简称SDT)中搜索出节目名称。
typedef struct PAT
{
	//PSI section负载数据的类型, PAT固定为0x00;
	unsigned table_id: 8;
	//段语法标志位，固定为1
	unsigned section_syntax_indicator: 1;
	//固定为0
	unsigned reserved_1: 3;
	//表示从下一个字段开始到CRC32(含) [下一个字段，CRC_32]，闭空间。之间有用的字节数。计算N loop的时候要-12. for ( n = 0; n < packet->section_length - 12; n += 4 )
	unsigned section_length: 12;
	//该传输流的ID，区别于一个网络中其它多路复用的流
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//范围0-31，表示PAT的版本号
	unsigned version_number: 5;
	//当前未来标志，表示当前有效还是未来有效，一般是0
	unsigned current_next_indicator: 1;
	//分段的号码，表示当前是分段的第几段。一般为0
	// PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	unsigned section_number: 8;
	//最后一个分段的号码
	unsigned last_section_number: 8;

	//单独存储NIT
	unsigned program_number: 16;
	unsigned reserved_3: 3; // 保留位
	unsigned network_PID: 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID

	//只存储PMT
	//program_number == 0 then NIT_PID else PMT_PID
	struct TS_PAT_Program* N_loop;
	unsigned int N_loop_len;

	//校验码
	unsigned CRC_32: 32;
} PAT;

//PES包非定长，音频的PES包小于等于64K，视频的一般为一帧一个PES包。
//一帧图象的PES包通常要由许多个TS包来传输。
// MPEG-2中规定，一个PES包必须由整数个TS包来传输。如果承载一个PES包的最后一个TS包没能装满，则用填充字节来填满；当下一个新的PES包形成时，需用新的TS包来开始传输。
typedef struct PES
{
	//24位起始码.固定0x000001
	unsigned packet_start_code_prefix: 24;
	//在PS流中该字段标识其存储的基本流的类型和索引号，在TS流中该字段仅标识其存储的基本流的类型
	//音频取值（0xc0-0xdf），通常为0xc0；视频取值（0xe0-0xef），通常为0xe0。
	unsigned stream_id: 8;
	//16位，用于存储后面pes数据的长度。
	//0表示长度不限制，只有视频数据长度会超过0xffff。
	unsigned PES_packet_length: 16;
} PES;

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
	unsigned int payloadOffset = 4;

	//有调节字段，跳过调节字段
	if (tsPacket->adaptation_field_control == 3)
	{
		printf("TS Package has adaptation field\n");
		unsigned char adaptation_field_length = 0;
		memcpy(&adaptation_field_length, rtpPayload + payloadOffset, 1);
		printf("adaptation_field_length:[%d]\n", adaptation_field_length);
		//还要加上第一个调节字节+1
		payloadOffset = payloadOffset + adaptation_field_length + 1;
	}

	tsPacket->payload = rtpPayload + payloadOffset;

	//荷载长度
	tsPacket->payloadLen = 0;

	return 0;
}

//判断PID是否时PMT
//PMT在传送流中用于指示组成某一套节目的视频、音频和数据在传送流中的位置，即对应的TS包的PID值，以及每路节目的节目时钟参考（PCR）字段的位置

int parsePES(unsigned char* ts_payload)
{
	PES pes;
	memcpy(&pes, ts_payload, sizeof(pes));
	if (pes.packet_start_code_prefix == 0x000001)
	{
		printf("[PES] | code:[%x] | stream id:[%x] | pes package len:[%d]\n",
			pes.packet_start_code_prefix,
			pes.stream_id,
			pes.PES_packet_length);
	}
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
	memcpy(pPat, payload, 64);
	pPat->table_id = payload[0];
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
	pPat->CRC_32 = CRC_32;
	return 0;
}

int parseES()
{
	return 0;
}

#endif //UDP_RTP_UDP_RTP_TS_H_
