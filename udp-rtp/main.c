//
// Created by young on 2023/2/4.
//
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>

/*
 * 	����� 12 + 188*7
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

	char* payload;
	unsigned int payloadLen;
} RTP_PACKET;

/*
	TS���̶���СΪ188�ֽڣ���ΪTS header��Adaptation Field��Payload
	TS Header�̶�4���ֽڣ�Adaptation Field���ܴ���Ҳ���ܲ����ڣ���Ҫ�����Ǹ�����188�ֽڵ���������䣻Payload��PES���ݡ�
  	TS Header Э���ʽ���£�Э��Ϊ 4 byte��
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |	sync byte  |T|P|T| 			PID			   |SC |AF |	CC |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct MPEGTS_FIXED_HEADER
{
	//ͬ���ֽڣ��̶�Ϊ0x47��
	unsigned sync_byte: 8;
	//��������־�� ֵΪ1��ʾ����صĴ������������һ�����ɾ����Ĵ���λ��������1���ڴ��󱻾���֮ǰ��������Ϊ0
	unsigned transport_error_indicator: 1;
	//������ʼ��־��Ϊ1ʱ����ʾ��ǰTS������Ч�غ��а���PES����PSI����ʼλ�ã���ǰ4���ֽ�֮�����һ�������ֽڣ������ֵΪ��������ֶεĳ���length�������Ч�غɿ�ʼ��λ��Ӧ��ƫ��1+[length]���ֽ�
	unsigned payload_unit_start_indicator: 1;
	//�������ȼ���־��1��������ǰTS�������ȼ�������������ͬPID�� ����λû�б��á�1����TS���ߡ�
	unsigned transport_priority: 1;
	//ָʾ�洢�������Ч���������ݵ����͡�
	//PATһ���Ӧ��pidΪ0x0������PAT�л�ָ��PMT��pid��
	//��������PMT��ʱ����Ҳ��ָ����Ӧ���յ�����Ƶ��pid����֮����յ�ts���о��Ǹ������PID����������Ƶ��������ȡ����Ӧ�����ݷŵ�PES�������С�
	//PIDֵ0x0000��0x000F����������0x0000ΪPAT������0x0001ΪCAT������0x1fffΪ���鱣�������հ�
	/*
	 * PIDֵ			����
		0				PAT(Program Association Table)
		1				CAT(Conditional Access Table)
		3-0xF			Reserved
		0x10-0x1FFE		�Զ���PID��������PMT��pid��network��pid��������Ŀ��
		0x1FFF			�հ�
	 */
	unsigned PID: 13;
	//���ſ��Ʊ�־����ʾTS��������Ч���صļ���ģʽ���հ�Ϊ��00��������������ͷ�а��������ֶΣ���Ӧ�����ܡ�����ȡֵ�������û��Զ���ġ�
	//һ��������ʶ����ʹ�á�
	unsigned scrambling_control: 2;
	//��������Ʊ�־����ʾ��ͷ�Ƿ��е����ֶλ���Ч���ء�
	//��ts���У�payload �����������ݣ�Ҳ�п��ܲ������ݣ����������ݵ�ʱ�����Ҫadaptation field�����0xff���ﵽ188�ֽ�
	//������PAT��PMT��˵��û��adaptation field �ġ������ĳ���ֱ�Ӳ�0xff����
	//ͬʱ����ͨ��adaptation field controlΪ��ͬ��ֵ������payload��Ϊʲô����ֵ��
	// 0x0 :  ΪISO/IECδ��ʹ�ñ���
	// 0x1 �� proloadΪ�����ݣ�Ϊ����Ƶ���ݣ�.����Ч�غɣ��޵����ֶ�
	// 0x2 �� Ϊadaptation field���ݣ�����Ч�غɣ����������ֶ�,�հ�ӦΪ0x2
	// 0x3 �� ��������Ƶ��������adaptation field���ݡ������ֶκ�Ϊ��Ч�غɣ������ֶ��е�ǰһ���ֽڱ�ʾ�����ֶεĳ���length����Ч�غɿ�ʼ��λ��Ӧ��ƫ��[length]���ֽ�
	unsigned adaptation_field_control: 2;
	unsigned continuity_counter: 4;
} MPEGTS_FIXED_HEADER;

//ͨ��ts���ĵ�5�ֽ�����ȡadaptation field�ĳ���,��payload�ĵ�һ���ֽ�,��Ч�غɿ�ʼ��λ��Ӧ��ƫ��[length]���ֽ�
unsigned char adaptation_field_len;

//TS����Payload���������Ϣ�����������ͣ���Ƶ����Ƶ��PES���Լ��������ݣ���Ŀר����ϢPSI��
/*			PSI
 * 	��Ŀ������Program Association Table (PAT) 0x0000				PAT�л�ָ��PMT��pid
 *	��Ŀӳ���Program Map Tables (PMT)							PMT�ж��������ض���Ŀ��ص�PID��Ϣ��������Ƶ��pid����Ƶ��pid�Լ�pcr��pid
 *	�������ձ�Conditional Access Table (CAT) 0x0001				CAT���������������������ò���
 *	TS_program_map_section	0x02
 *	TS_description_section	0x03
 *	ISO_IEC_14496_scene_description_section	0x04
 *	ISO_IEC_14496_object_descriptor_section	0x05
 *	ITU-T Rec. H.222.0 / ISO/IEC 13818-1 reserved	0x06~0x37
 *	Defined in ISO/IEC 13818-6	0x38~0x3F
 *	User private	0x40~0xFE
 *	forbidden		0XFF
 *	������Ϣ��Network Information Table(NIT) 0x0010				NIT�ǿ�ѡ�ģ���׼��δ��ϸ����
 *	������������Transport Stream Description Table(TSDT) 0x02		TSDTҲ�ǿ�ѡ��
 */
//��Ƶ����Ƶ��ES������д���γ���Ƶ����Ƶ�� PES�����������ݣ���ͼ�ĵ�����Ϣ������Ҫ���PES����
//adaptation_field_controlΪ0x1����0x3ʱ��ͨ����0x1������PES�׸��������һ����ʱ������Ϊ0x3����Ч����


//�ýṹռ��12�ֽڣ�N loop = section_length - 12 .
//�ȷ���PAT.TS���лᶨ�ڳ���PAT��PAT���ṩ�˽�Ŀ�źͶ�ӦPMT����PID�Ķ�Ӧ��ϵ��
//��Ŀ������(PAT Program Association Table)�����ֵ���ϵͳ�н�Ŀָʾ�ĸ��ڵ㡣
// �����ʶ��(Packet IDentifier�����PID)Ϊ0���ն��豸��������У�������Ŀʱ���ȶ��Ǵ����ű�ʼ�����ġ�
// ��PAT�н�������Ŀӳ���(PMT)���ٴ�PMT����������Ԫ��(����Ƶ����Ƶ�����ݵ�)��PID����Ŀ�š��ٸ��ݽ�Ŀ�ӽ�Ŀҵ��������(Service Description Table�����SDT)����������Ŀ���ơ�
typedef struct PAT
{
	//PSI section�������ݵ�����, PAT�̶�Ϊ0x00;
	unsigned table_id: 8;
	//���﷨��־λ���̶�Ϊ1
	unsigned section_syntax_indicator: 1;
	//�̶�Ϊ0
	unsigned zero: 1;
	unsigned reserved_1: 2;
	//��ʾ����һ���ֶο�ʼ��CRC32(��)֮�����õ��ֽ���������N loop��ʱ��Ҫ-12. for ( n = 0; n < packet->section_length - 12; n += 4 )
	unsigned section_length: 12;
	//�ô�������ID��������һ��������������·���õ���
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//��Χ0-31����ʾPAT�İ汾��
	unsigned version_number: 5;
	//���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч
	unsigned current_next_indicator: 1;
	//�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	unsigned section_number: 8;
	//���һ���ֶεĺ���
	unsigned last_section_number: 8;

	//�����洢NIT
	unsigned program_number: 16;
	unsigned reserved_3: 3; // ����λ
	unsigned network_PID: 13; //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID

	//ֻ�洢PMT
	//program_number == 0 then NIT_PID else PMT_PID
	struct TS_PAT_Program* N_loop;

	//У����
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
	//��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;//WSADATA�ṹ������ĵ�ֵַ
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

				//��ȡrtpͷ
				memcpy(&rtp_header, recvData, sizeof(rtp_header));
				if (rtp_header.version != 0x02)
				{
					perror("rtp version is not 0x02");
					return -1;
				}

				//����rtpЭ��ͷ�͹���ý��Դ
				payloadOffset = rtp_header.csrc_count * 32 + sizeof(rtp_header);
				//������չͷ
				if (rtp_header.extension)
				{
					rtp_ext_header = (RTP_EXT_HEADER*)(recvData + payloadOffset);
					unsigned short exthdrlen = ntohs(rtp_ext_header->ext_count);

					payloadOffset += sizeof(rtp_ext_header) + exthdrlen * 32;

				}
				//��ȡ����ֽ���
				if (rtp_header.padding)
				{
					//���һ���ֽ�Ϊ����С
					numPadBytes = recvData[pktsize - 1];
					if (numPadBytes <= 0)
					{
						perror("padding size <= 0");
						return -1;
					}
				}
				//������س���
				payloadLen = pktsize - numPadBytes - payloadOffset;
				if (payloadLen < 0)
				{
					perror("payloadLen < 0");
					return -1;
				}

				//��䵽RTPPacket��
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

				//������rtpͷ������Դ����չͷ
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