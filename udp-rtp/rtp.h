//
// Created by young on 2023/2/6.
//

#ifndef UDP_RTP_UDP_RTP_RTP_H_
#define UDP_RTP_UDP_RTP_RTP_H_
/*
 * 	大端序
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

	unsigned char* payload;
	unsigned int payloadLen;
} RTP_PACKET;

int parseRTP(char* buf, int bufLen, RTP_PACKET* rtpPacket)
{
	if (bufLen < sizeof(RTP_FIXED_HEADER))
	{
		perror("rtp size < rtp_fixed_header size");
		return -1;
	}

	RTP_FIXED_HEADER rtp_header;
	//获取rtp头
	memcpy(&rtp_header, buf, sizeof(RTP_FIXED_HEADER));
	if (rtp_header.version != 0x02)
	{
		perror("rtp version is not 0x02");
		return -1;
	}

	//填充到RTPPacket中
	rtpPacket->hasMarker = rtp_header.marker;
	rtpPacket->payloadType = rtp_header.payload;
	rtpPacket->numCSRC = rtp_header.csrc_count;
	rtpPacket->seqNo = ntohs(rtp_header.seq_no);
	rtpPacket->timestamp = ntohl(rtp_header.timestamp);
	rtpPacket->SSRC = ntohl(rtp_header.ssrc);

	rtpPacket->packet = buf;
	rtpPacket->packetLen = bufLen;

	//先跳过rtp协议头和共享媒体源
	unsigned int payloadOffset = 0;
	payloadOffset = rtp_header.csrc_count * 4 + sizeof(RTP_FIXED_HEADER);

	//再跳过扩展头
	rtpPacket->hasExtension = rtp_header.extension;
	if (rtp_header.extension)
	{
		RTP_EXT_HEADER rtp_ext_header;
		memcpy(&rtp_ext_header, buf + payloadOffset, sizeof(RTP_EXT_HEADER));

		rtpPacket->extId = ntohs(rtp_ext_header.profile);
		rtpPacket->extensionLen = ntohs(rtp_ext_header.ext_count) * 4;
		rtpPacket->pExtension = buf + payloadOffset + sizeof(RTP_EXT_HEADER);

		payloadOffset += sizeof(RTP_EXT_HEADER) + rtp_ext_header.ext_count * 4;
	}

	//荷载指针
	//跳过了rtp头，共享源，扩展头
	rtpPacket->payload = buf + payloadOffset;

	//荷载长度
	//获取填充字节树
	unsigned int numPaddingBytes = 0;
	if (rtp_header.padding)
	{
		//最后一个字节为填充大小
		numPaddingBytes = buf[bufLen - 1];
		if (numPaddingBytes <= 0)
		{
			perror("padding size <= 0");
			return -1;
		}
	}
	//计算荷载长度
	unsigned int payloadLen = bufLen - numPaddingBytes - payloadOffset;
	if (payloadLen < 0)
	{
		perror("payloadLen < 0");
		return -1;
	}

	//荷载长度
	rtpPacket->payloadLen = payloadLen;

	return 0;
}

#endif //UDP_RTP_UDP_RTP_RTP_H_
