//
// Created by young on 2023/2/6.
//

#ifndef UDP_RTP_UDP_RTP_RTP_H_
#define UDP_RTP_UDP_RTP_RTP_H_
/*
 * 	�����
  	RTP Э���ʽ���£�+=+����Ϊ RTP Э���ѡ�ֶΣ�RTP Э����СΪ 12 byte��
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
	//CSRC������������ý��Դ����CSRC��ʶ���ĸ���,һ�����ڻ����ͻ����У�����ĳ����Ƶ���ǻ����������Ƶ������ݣ���ô������ƵԴ���Ǹ���ƵԴ�� CSRC
	//CSRC��СΪ32λ
	unsigned csrc_count: 4;
	//��չ��ʶ(X),���X=1������RTP��ͷ�����һ����չ��ͷ
	unsigned extension: 1;
	//����ʶ(P),���P=1,���ڸñ��ĵ�β�����һ����������İ�λ�飬���ǲ�����Ч�غɵ�һ����,�������һ���ֽڱ�ʾ����ֽ���,һ����һЩ��Ҫ�̶����С�ļ����㷨�в���Ҫ���
	unsigned padding: 1;
	//�汾�ţ�V����2���أ�������־ʹ�õ�RTP�汾��MUST be 0x2
	unsigned version: 2;

	//��Ч�������ͣ���GSM��Ƶ��JPEMͼ���,����ý���д󲿷�������������Ƶ������Ƶ���ģ��������ڿͻ��˽��н���
	//rfc-3551 6.  Payload Type Definitions
	unsigned payload: 7;
	//��־λ(M),���ڲ�ͬ�ĸ��������в�ͬ���壬����ʹ�� RTP ���� H264 ����ʱ�����ĳ��֡�ֳɶ�������д��䣬����ʹ�ø�λ����Ƿ�Ϊ֡�����һ����
	//������Ƶ����ǻỰ�Ŀ�ʼ
	unsigned marker: 1;
#endif

	//���кţ�ÿ�� RTP ����ŵ�����һ�����ն˸������кſ����жϴ����Ƿ񶪰���ͬʱ�������綶��������������������ݽ�����������,
	//���кų�ʼֵ�������,ͬʱ��Ƶ������Ƶ����sequence�Ƿֱ������
	unsigned seq_no: 16;

	//���ʱ�����Ϣ,��ӳ RTP ���ݰ����ݲ���ʱ�䣬һ��֡�����ݿ��ܱ��ֳɶ�� RTP �����ͣ�ͬһ��֡��ʱ�������ͬ�ģ���ͬ֡��ʱ����ǲ���ͬ��
	//��ֵ��ʼֵ������ģ���λ�ĺ��������ݲ���Ƶ���й�,����ʹ��90 kHz ʱ��Ƶ��
	//������ʹ��ʱ���������ӳٺ��ӳٶ�����������ͬ�����ơ�
	unsigned timestamp: 32;

	//ͬ����Դ����ͬ�� SSRC ��ʶ��ͬ��ý��Դ�����粻ͬ�û�����Ƶ�����ڲ�ͬ��ý��Դ�����в�ͬ�� SSRC
	//�ñ�ʶ�������ѡ��ģ��μ�ͬһ��Ƶ���������ͬ����Դ��������ͬ��SSRC��
	unsigned ssrc: 32;

	//����ý��Դ�б���ʾ�� RTP �����غ������õ�ý��Դ,�μ� CC ���ͣ�CSRC ��� 0~15 ��
	//unsigned int csrc[16];
} RTP_FIXED_HEADER;

typedef struct RTP_EXT_HEADER
{
	//Profile = 0xBEDE ʱ��ʾʹ�� one-byte header��Profile = 0x1000 ʱ��ʾʹ�� two-byte header
	unsigned profile: 16;
	//Extension header length ��ʾ����� Extension header ���м�����Extension header����Ϊ 4 �ֽ�/32λ������ length = 3 ��ʾ Extension header һ��ռ 3*4=12 ���ֽ�
	//����ʱ�����תС�ζ�
	unsigned ext_count: 16;

	//one-byte header  ��ʽ���£����� ID��L��data ��������ɡ�ID �� L �ֱ�ռ 4 bit������������ one-byte��ID ��ʾ��չͷ�� ID ��ǣ�L ��ʾ extension data ��ռ�ֽ��� -1��
	//���� L = 0 ʱʵ�� data ռһ���ֽڣ�����ͷ����Ҫ�� 4 �ֽڶ��룬����м䲹���� padding ���ݣ����һ�� extension header data ռ 4 �ֽڡ�

	//two-byte header ��ʽ���£���Ҳ���� ID, L, data ��������ɣ���֮ͬ������ two-byte header �� ID��L ��ռһ�ֽڣ��� L ��ʾ extension data ��ռ���ֽ�������ͬ�� one-byte header ��Ҫ��һ����
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
	//��ȡrtpͷ
	memcpy(&rtp_header, buf, sizeof(RTP_FIXED_HEADER));
	if (rtp_header.version != 0x02)
	{
		perror("rtp version is not 0x02");
		return -1;
	}

	//��䵽RTPPacket��
	rtpPacket->hasMarker = rtp_header.marker;
	rtpPacket->payloadType = rtp_header.payload;
	rtpPacket->numCSRC = rtp_header.csrc_count;
	rtpPacket->seqNo = ntohs(rtp_header.seq_no);
	rtpPacket->timestamp = ntohl(rtp_header.timestamp);
	rtpPacket->SSRC = ntohl(rtp_header.ssrc);

	rtpPacket->packet = buf;
	rtpPacket->packetLen = bufLen;

	//������rtpЭ��ͷ�͹���ý��Դ
	unsigned int payloadOffset = 0;
	payloadOffset = rtp_header.csrc_count * 4 + sizeof(RTP_FIXED_HEADER);

	//��������չͷ
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

	//����ָ��
	//������rtpͷ������Դ����չͷ
	rtpPacket->payload = buf + payloadOffset;

	//���س���
	//��ȡ����ֽ���
	unsigned int numPaddingBytes = 0;
	if (rtp_header.padding)
	{
		//���һ���ֽ�Ϊ����С
		numPaddingBytes = buf[bufLen - 1];
		if (numPaddingBytes <= 0)
		{
			perror("padding size <= 0");
			return -1;
		}
	}
	//������س���
	unsigned int payloadLen = bufLen - numPaddingBytes - payloadOffset;
	if (payloadLen < 0)
	{
		perror("payloadLen < 0");
		return -1;
	}

	//���س���
	rtpPacket->payloadLen = payloadLen;

	return 0;
}

#endif //UDP_RTP_UDP_RTP_RTP_H_
