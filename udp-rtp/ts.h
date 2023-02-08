//
// Created by young on 2023/2/6.
//

#ifndef UDP_RTP_UDP_RTP_TS_H_
#define UDP_RTP_UDP_RTP_TS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#endif

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
	//举个例子，一个PES包可能由多个TS包构成，第一个TS包的负载单元起始指标位才会被置位
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

	//该传输流的ID，区别于一个网络中其它多路复用的流,作为一个标签，该字段指出在网络中与
	//其它复用流的区别标志，其值由用户定义。
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//范围0-31，表示PAT的版本号
	unsigned version_number: 5;
	//当前未来标志，表示当前有效还是未来有效，置0时，表明该传送的表分段不能使用，下一个表分段才有效。
	unsigned current_next_indicator: 1;
	//分段的号码，表示当前是分段的第几段。一般为0
	// PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	unsigned section_number: 8;
	//最后一个分段的号码.表明最后一个分段号，同时表明该PAT的最大分段数目。一般，一个PAT表由一个TS包传送
	unsigned last_section_number: 8;

	//单独存储NIT
	//NIT表的PID值。
	unsigned program_number: 16;
	unsigned reserved_3: 3; // 保留位
	unsigned network_PID: 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID

	//只存储PMT
	//PMT表的PID值
	//program_number == 0 then NIT_PID else PMT_PID
	struct TS_PAT_Program* program;
	unsigned int program_len;

	//校验码
	unsigned CRC_32: 32;
} PAT;

typedef struct TS_PAT_Program
{
	//节目的编号
	unsigned program_number: 16;
	unsigned reserved: 3;
	//PMT表的PID
	unsigned program_map_PID: 13;
} TS_PAT_Program;

typedef struct PMT
{
	//标识字段。指明了一个TS中PSI分段的内容是节目关联分段、条件访问分段、TS节目映射分段。（PAT/CAT/PMT）
	//固定为0x02, 表示PMT表
	unsigned table_id: 8;
	unsigned section_syntax_indicator: 1; //固定为0x01
	unsigned reserved_1: 3; //0x03
	//指示分段的字节数，长度等于该 字段之后到 CRC32 （ 包括在内）的字节数。
	unsigned section_length: 12;

	unsigned program_number: 16;// 指出该节目对应于可应用的Program map PID
	unsigned reserved_2: 2; //0x03
	unsigned version_number: 5; //指出TS流中Program map section的版本号
	//当该位置1时，当前传送的Program map section可用；
	unsigned current_next_indicator: 1;
	//分段的号码，表示当前是分段的第几段。一般为0
	//固定为0x00
	unsigned section_number: 8;
	//最后一个分段的号码.表明最后一个分段号，同时表明该PAT的最大分段数目。
	//固定为0x00
	unsigned last_section_number: 8;

	unsigned reserved_3: 3; //0x07
	//指明TS包的PID值，该TS包含有PCR域，该PCR值对应于由节目号指定的对应节目。
	//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
	unsigned PCR_PID: 13;
	unsigned reserved_4: 4; //预留为0x0F
	//表明跟随其后的对节目信息描述的字节数，也就是第一个N loop descriptors的字节数。
	unsigned program_info_length: 12;

	struct PMT_STREAM* pmt_stream;
	unsigned int pmt_stream_len;

	//在CEDARX代码中仅对DVB的场景下作校验。
	//校验码
	unsigned CRC_32: 32;
} PMT;

typedef struct PMT_STREAM
{
	//指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	//表明PES流的类型。譬如，0x01表明是MPEG-1视频，0X03表明是MPEG-1音频。
	//0X1b H.254 0X0F AAC
	unsigned stream_type: 8;
	unsigned reserved_1: 3;
	//该域指示TS包的PID值。这些TS包含有相关的节目元素
	//表明该负载有该PES流的TS包的PID值
	unsigned elementary_PID: 13;
	unsigned reserved_2: 4;
	//表明跟随其后的描述相关节目元素的字节数，也就是第二个N loop descriptors的字节数。
	//前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned ES_info_length: 12;
} PMT_STREAM;

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
	unsigned packet_length: 16;

	unsigned reserved_0: 2;
	unsigned scrambling_control: 2;
	unsigned priority: 1;
	unsigned data_alignment_indicator: 1;
	unsigned copyright: 1;
	unsigned original_or_copy: 1;

	/*
	 * 0x00	PES包头中既无任何PTS字段也无任何DTS字段存在。
	 * 0x01	禁用
	 * 0x02	PES包头中PTS字段存在。
	 * 0x03	PES包头中PTS字段和DTS字段均存在。
	 */
	unsigned pts_dts_flags: 2;
	//0:指示无任何ESCR字段存在
	//1:指示PES包头中ESCR基准字段和ESCR扩展字段均存在
	unsigned escr_flag: 1;
	//置于‘1’时指示PES包头中ES_rate 字段存在。置于‘0’时指示无任何 ES_rate 字段存在
	unsigned es_rate_flag: 1;
	//置于‘1’时指示8比特特技方式字段存在。置于‘0’时指示此字段不存在
	unsigned dsm_trick_mode_flag: 1;
	//置于‘1’时指示additional_copy_info 存在。置于‘0’时指示此字段不存在
	unsigned additional_copy_info_flag: 1;
	//置于‘1’时指示PES包中CRC字段存在。置于‘0’时指示此字段不存在
	unsigned crc_flag: 1;
	//置于‘1’时指示PES包头中扩展字段存在。置于‘0’时指示此字段不存在
	unsigned extension_flag: 1;

	//指示在此PES包头中包含的由任选字段和任意填充字节所占据的字节总数。
	//任选字段的存在由前导 PES_header_data_length 字段的字节来指定
	unsigned header_data_length: 8;

	unsigned char payload_offset;

	unsigned pts_flag: 4;
	unsigned pts_marker: 3;
	unsigned pts: 33;

} PES;

int parseTS(unsigned char* rtpPayload, TS_PACKET* tsPacket);

int parsePES(unsigned char* payload, unsigned char type);

int parseSDT(TS_PACKET* tsPacket);

int parsePAT(unsigned char* payload, PAT* pPat);

int parsePMT(unsigned char* payload, PMT* pPMT);

#endif //UDP_RTP_UDP_RTP_TS_H_
