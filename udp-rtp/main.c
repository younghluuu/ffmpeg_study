//
// Created by young on 2023/2/4.
//
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>

/*
 * 	大端序 12 + 188*7
  	RTP 协议格式如下，+=+部分为 RTP 协议可选字段，RTP 协议最小为 12 byte。
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           timestamp                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           synchronization source (SSRC) identifier            |
   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   |            contributing source (CSRC) identifiers             |
   |                             ....                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct RTP_FIXED_HEADER
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	unsigned version: 2;
	unsigned padding: 1;
	unsigned extension: 1;
	unsigned csrc_count: 4;

	unsigned marker: 1;
	unsigned payload: 7;
#else
	//CSRC计数器，共享媒体源个数CSRC标识符的个数,一般用于混音和混屏中，例如某个音频流是混合了其它音频后的数据，那么其它音频源就是该音频源的 CSRC
	//CSRC大小为32位
	unsigned csrc_count: 4;
	//扩展标识(X),如果X=1，则在RTP报头后跟有一个扩展报头
	unsigned extension: 1;
	//填充标识(P),如果P=1,则在该报文的尾部填充一个或多个额外的八位组，它们不是有效载荷的一部分,包的最后一个字节表示填充字节数,一般在一些需要固定块大小的加密算法中才需要填充
	unsigned padding: 1;
	//版本号（V）：2比特，用来标志使用的RTP版本。MUST be 0x2
	unsigned version: 2;

	//有效荷载类型，如GSM音频、JPEM图像等,在流媒体中大部分是用来区分音频流和视频流的，这样便于客户端进行解析
	//rfc-3551 6.  Payload Type Definitions
	unsigned payload: 7;
	//标志位(M),对于不同的负载类型有不同含义，例如使用 RTP 荷载 H264 码流时，如果某个帧分成多个包进行传输，可以使用该位标记是否为帧的最后一个包
	//对于音频，标记会话的开始
	unsigned marker: 1;
#endif

	//序列号，每个 RTP 包序号递增加一，接收端根据序列号可以判断传输是否丢包，同时出现网络抖动的情况可以用来对数据进行重新排序,
	//序列号初始值是随机的,同时音频包和视频包的sequence是分别记数的
	unsigned seq_no: 16;

	//相对时间戳信息,反映 RTP 数据包数据采样时间，一个帧的数据可能被分成多个 RTP 包发送，同一个帧的时间戳是相同的，不同帧的时间戳是不相同的
	//该值初始值是随机的，单位的含义与数据采样频率有关,必须使用90 kHz 时钟频率
	//接收者使用时戳来计算延迟和延迟抖动，并进行同步控制。
	unsigned timestamp: 32;

	//同步信源，不同的 SSRC 标识不同的媒体源，例如不同用户的音频就属于不同的媒体源，具有不同的 SSRC
	//该标识符是随机选择的，参加同一视频会议的两个同步信源不能有相同的SSRC。
	unsigned ssrc: 32;

	//共享媒体源列表，表示对 RTP 包内载荷起作用的媒体源,参见 CC 解释，CSRC 最多 0~15 个
	//unsigned int csrc[16];
} RTP_FIXED_HEADER;

typedef struct RTP_EXT_HEADER
{
	//Profile = 0xBEDE 时表示使用 one-byte header，Profile = 0x1000 时表示使用 two-byte header
	unsigned profile: 16;
	//Extension header length 表示后面的 Extension header 共有几个，Extension header长度为 4 字节/32位，例如 length = 3 表示 Extension header 一共占 3*4=12 个字节
	//计算时，大端转小段端
	unsigned ext_count: 16;

	//one-byte header  格式如下，它由 ID，L，data 三部分组成。ID 和 L 分别占 4 bit，加起来等于 one-byte，ID 表示扩展头部 ID 标记，L 表示 extension data 所占字节数 -1，
	//例如 L = 0 时实际 data 占一个字节，由于头部需要按 4 字节对齐，因此中间补充了 padding 数据，最后一个 extension header data 占 4 字节。

	//two-byte header 格式如下，它也是由 ID, L, data 三部分组成，不同之处在于 two-byte header 中 ID，L 各占一字节，而 L 表示 extension data 所占的字节数（不同于 one-byte header 需要减一）。
} RTP_EXT_HEADER;

typedef struct RTP_PACKET
{
	boolean hasExtension;
	unsigned short extId;
	unsigned short extensionLen;
	char* pExtension;

	boolean hasMarker;
	unsigned int numCSRC;
	unsigned int payloadType;

	unsigned int seqNo;
	unsigned int timestamp;
	unsigned int SSRC;

	char* packet;
	unsigned int packetLen;

	char* payload;
	unsigned int payloadLen;
} RTP_PACKET;

/*
	TS包固定大小为188字节，分为TS header、Adaptation Field、Payload
	TS Header固定4个字节；Adaptation Field可能存在也可能不存在，主要作用是给不足188字节的数据做填充；Payload是PES数据。
  	TS Header 协议格式如下，协议为 4 byte。
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |	sync byte  |T|P|T| 			PID			   |SC |AF |	CC |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct MPEGTS_FIXED_HEADER
{
	//同步字节，固定为0x47，
	unsigned sync_byte: 8;
	//传输错误标志。 值为1表示在相关的传输包中至少有一个不可纠正的错误位。当被置1后，在错误被纠正之前不能重置为0
	unsigned transport_error_indicator: 1;
	//负载起始标志。为1时，表示当前TS包的有效载荷中包含PES或者PSI的起始位置；在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。因此有效载荷开始的位置应再偏移1+[length]个字节
	unsigned payload_unit_start_indicator: 1;
	//传输优先级标志。1’表明当前TS包的优先级比其他具有相同PID， 但此位没有被置‘1’的TS包高。
	unsigned transport_priority: 1;
	//指示存储与分组有效负载中数据的类型。
	//PAT一般对应的pid为0x0，而在PAT中会指明PMT的pid。
	//而解析完PMT的时候，他也会指明对应接收的音视频的pid。而之后接收的ts包中就是根据这个PID来区分音视频，并从中取出对应的数据放到PES的数据中。
	//PID值0x0000—0x000F保留。其中0x0000为PAT保留；0x0001为CAT保留；0x1fff为分组保留，即空包
	/*
	 * PID值			描述
		0				PAT(Program Association Table)
		1				CAT(Conditional Access Table)
		3-0xF			Reserved
		0x10-0x1FFE		自定义PID，可用于PMT的pid、network的pid或者其他目标
		0x1FFF			空包
	 */
	unsigned PID: 13;
	//加扰控制标志。表示TS流分组有效负载的加密模式。空包为‘00’，如果传输包包头中包括调整字段，不应被加密。其他取值含义是用户自定义的。
	//一般用来标识加密使用。
	unsigned scrambling_control: 2;
	//适配域控制标志。表示包头是否有调整字段或有效负载。
	//在ts包中，payload 区可能是数据，也有可能不是数据，当不是数据的时候就需要adaptation field来填充0xff来达到188字节
	//而对于PAT和PMT来说是没有adaptation field 的。不够的长度直接补0xff即可
	//同时可以通过adaptation field control为不同的值来区分payload中为什么样的值。
	// 0x0 :  为ISO/IEC未来使用保留
	// 0x1 ： proload为纯数据（为音视频数据）.含有效载荷，无调整字段
	// 0x2 ： 为adaptation field数据，无有效载荷，仅含调整字段,空包应为0x2
	// 0x3 ： 既有音视频数据又有adaptation field数据。调整字段后为有效载荷，调整字段中的前一个字节表示调整字段的长度length，有效载荷开始的位置应再偏移[length]个字节
	unsigned adaptation_field_control: 2;
	unsigned continuity_counter: 4;
} MPEGTS_FIXED_HEADER;

//通过ts包的第5字节来获取adaptation field的长度,即payload的第一个字节,有效载荷开始的位置应再偏移[length]个字节
unsigned char adaptation_field_len;

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
	unsigned zero: 1;
	unsigned reserved_1: 2;
	//表示从下一个字段开始到CRC32(含)之间有用的字节数。计算N loop的时候要-12. for ( n = 0; n < packet->section_length - 12; n += 4 )
	unsigned section_length: 12;
	//该传输流的ID，区别于一个网络中其它多路复用的流
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//范围0-31，表示PAT的版本号
	unsigned version_number: 5;
	//发送的PAT是当前有效还是下一个PAT有效
	unsigned current_next_indicator: 1;
	//分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
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

	//校验码
	unsigned CRC_32: 32;
} PAT;

typedef struct TS_PAT_Program
{
	unsigned program_number: 16;
	unsigned reserved: 3;
	unsigned program_map_PID: 13;
} TS_PAT_Program;

typedef struct PES
{

} PES;

int simplest_udp_parser(int port)
{

	//FILE *myout=fopen("output_log.txt","wb+");
	FILE* myout = stdout;
	FILE* fp1 = fopen("output_dump.ts", "wb+");

#ifdef _WIN32
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;//WSADATA结构体变量的地址值
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return -1;
	}
#endif

	//Socket
#ifdef _WIN32
	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
	int serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
	if (serSocket <= 0)
	{
		printf("socket error !");
		return -1;
	}

	//Bind
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(serSocket, (struct sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind error !");
		return -1;
	}

	//How to parse?
	int parse_rtp = 1;
	int parse_mpegts = 1;

	//Recv
	struct sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	char recvData[10000];
	printf("Accepting connections ...\n");
	int cnt = 0;
	while (1)
	{
		int pktsize = recvfrom(serSocket, recvData, 10000, 0, (struct sockaddr*)&remoteAddr, &nAddrLen);
		if (pktsize > 0)
		{
			printf("recv dataLen:%d from %s:%d \r\n",
				pktsize,
				inet_ntoa(remoteAddr.sin_addr),
				ntohs(remoteAddr.sin_port));

			//Parse RTP
			if (parse_rtp)
			{

				if (pktsize < sizeof(RTP_FIXED_HEADER))
				{
					perror("rtp size < rtp_fixed_header size");
					return -1;
				}

				RTP_FIXED_HEADER rtp_header;
				unsigned int payloadLen = 0;
				unsigned int payloadOffset = 0;
				unsigned int numPadBytes = 0;
				RTP_EXT_HEADER* rtp_ext_header;

				//获取rtp头
				memcpy(&rtp_header, recvData, sizeof(rtp_header));
				if (rtp_header.version != 0x02)
				{
					perror("rtp version is not 0x02");
					return -1;
				}

				//跳过rtp协议头和共享媒体源
				payloadOffset = rtp_header.csrc_count * 32 + sizeof(rtp_header);
				//跳过扩展头
				if (rtp_header.extension)
				{
					rtp_ext_header = (RTP_EXT_HEADER*)(recvData + payloadOffset);
					unsigned short exthdrlen = ntohs(rtp_ext_header->ext_count);

					payloadOffset += sizeof(rtp_ext_header) + exthdrlen * 32;

				}
				//获取填充字节树
				if (rtp_header.padding)
				{
					//最后一个字节为填充大小
					numPadBytes = recvData[pktsize - 1];
					if (numPadBytes <= 0)
					{
						perror("padding size <= 0");
						return -1;
					}
				}
				//计算荷载长度
				payloadLen = pktsize - numPadBytes - payloadOffset;
				if (payloadLen < 0)
				{
					perror("payloadLen < 0");
					return -1;
				}

				//填充到RTPPacket中
				RTP_PACKET rtpPacket;
				rtpPacket.hasExtension = rtp_header.extension;
				if (rtpPacket.hasExtension)
				{
					rtpPacket.extId = ntohs(rtp_ext_header->profile);
					rtpPacket.extensionLen = ntohs(rtp_ext_header->ext_count) * 32;
					rtpPacket.pExtension = (char*)rtp_ext_header + sizeof(RTP_EXT_HEADER);
				}

				rtpPacket.hasMarker = rtp_header.marker;
				rtpPacket.numCSRC = rtp_header.csrc_count;
				rtpPacket.payloadType = rtp_header.payload;

				rtpPacket.seqNo = ntohs(rtp_header.seq_no);
				rtpPacket.timestamp = ntohs(rtp_header.timestamp);
				rtpPacket.SSRC = ntohs(rtp_header.ssrc);

				rtpPacket.packet = recvData;
				rtpPacket.packetLen = pktsize;

				//跳过了rtp头，共享源，扩展头
				rtpPacket.payload = recvData + payloadOffset;
				rtpPacket.payloadLen = payloadLen;

				//rfc3551 	6.  Payload Type Definitions
				char payload_str[10] = { 0 };
				switch (rtpPacket.payloadType)
				{
				case 18:
					sprintf(payload_str, "Audio");
					break;
				case 31:
					sprintf(payload_str, "H.261");
					break;
				case 32:
					sprintf(payload_str, "MPV");
					break;
				case 33:
					sprintf(payload_str, "MP2T");
					break;
				case 34:
					sprintf(payload_str, "H.263");
					break;
				case 96:
					sprintf(payload_str, "H.264");
					break;
				default:
					sprintf(payload_str, "other");
					break;
				}

				fprintf(myout,
					"[RTP Pkt] %5d| %5s| %10u| %5d| %5d|\n",
					cnt,
					payload_str,
					rtpPacket.timestamp,
					rtpPacket.seqNo,
					rtpPacket.payloadLen);

				fwrite(rtpPacket.payload, 1, rtpPacket.payloadLen, fp1);

				//mpegTS
				if (parse_mpegts && rtp_header.payload == 33)
				{

				}
			}
		}

		cnt++;
	}

	//Close
#ifdef _WIN32
	closesocket(serSocket);
	WSACleanup();
#else
	close(serSocket);
#endif
}

int main()
{
	simplest_udp_parser(8888);
}