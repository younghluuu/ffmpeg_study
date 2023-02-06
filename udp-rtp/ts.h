//
// Created by young on 2023/2/6.
//

#ifndef UDP_RTP_UDP_RTP_TS_H_
#define UDP_RTP_UDP_RTP_TS_H_
/*
	TS���̶���СΪ188�ֽڣ���ΪTS header��Adaptation Field��Payload
	TS Header�̶�4���ֽڣ�Adaptation Field���ܴ���Ҳ���ܲ����ڣ���Ҫ�����Ǹ�����188�ֽڵ���������䣻Payload��PES���ݡ�
  	TS Header Э���ʽ���£�Э��Ϊ 4 byte��
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |	sync byte  |T|P|T| 			PID			   |SC |AF |	CC |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
typedef struct MPEGTS_FIXED_HEADER_BIT
{
	//ͬ���ֽڣ��̶�Ϊ0x47��
	unsigned sync_byte: 8;
	//��������־�� ֵΪ1��ʾ����صĴ������������һ�����ɾ����Ĵ���λ��������1���ڴ��󱻾���֮ǰ��������Ϊ0
	unsigned transport_error_indicator: 1;
	//������ʼ��־��ʶ��Ϊ1ʱ����ʾ��ǰTS������Ч�غ��а���PES����PSIͷ����ʼλ�ã�
	//1+[length],������һ��point, ��ǰ4���ֽ�֮�����һ�������ֽڣ������ֵΪ��������ֶεĳ���length�������Ч�غɿ�ʼ��λ��Ӧ��ƫ��1+[length]���ֽ�
	//�����PSI/SI�Ļ������һ��point.PointField ָ���� SI��PSI ��ͷ����Ч�����еľ���λ��
	unsigned payload_unit_start_indicator: 1;
	//�������ȼ���־��1��������ǰTS�������ȼ�������������ͬPID�� ����λû�б��á�1����TS���ߡ�
	unsigned transport_priority: 1;
	//ָʾ�洢�������Ч���������ݵ����͡�
	//PATһ���Ӧ��pidΪ0x0������PAT�л�ָ��PMT��pid��
	//��������PMT��ʱ����Ҳ��ָ����Ӧ���յ�����Ƶ��pid����֮����յ�ts���о��Ǹ������PID����������Ƶ��������ȡ����Ӧ�����ݷŵ�PES�������С�
	//PIDֵ0x0000��0x000F����������0x0000ΪPAT������0x0001ΪCAT������0x1fffΪ���鱣�������հ�
	/*
	 * PIDֵ			����
		00				PAT(Program Association Table)
		01				CAT(Conditional Access Table)
	 	11				SDT��Ҫ������Ƶ��������Ϣ�����紴������ffmpeg,ע�⣺����һ��pid=0x11�˾�һ����SDT�����ÿ�table-id,��table-id,=0x4A������BAT��
		3-0xF			Reserved
		0x10-0x1FFE		�Զ���PID��������PMT��pid��network��pid��������Ŀ��
		0x1FFF			�հ�
	 */
	unsigned PID: 13;
	//���ſ��Ʊ�־����ʾTS��������Ч���صļ���ģʽ���հ�Ϊ��00��������������ͷ�а��������ֶΣ���Ӧ�����ܡ�����ȡֵ�������û��Զ���ġ�
	//һ��������ʶ����ʹ�á�
	unsigned scrambling_control: 2;
	//����Ӧ����ʶ����ʾ��ͷ�Ƿ��е����ֶλ���Ч���ء��̡̡̡̡�
	//��ts���У�payload �����������ݣ�Ҳ�п��ܲ������ݣ����������ݵ�ʱ�����Ҫadaptation field�����0xff���ﵽ188�ֽ�
	//������PAT��PMT��˵��û��adaptation field �ġ������ĳ���ֱ�Ӳ�0xff����
	//ͬʱ����ͨ��adaptation field controlΪ��ͬ��ֵ������payload��Ϊʲô����ֵ��
	// 0x00 :  ΪISO/IECδ��ʹ�ñ���
	// 0x01 �� proloadΪ�����ݣ�Ϊ����Ƶ���ݣ�.����Ч�غɣ��޵����ֶ�
	// 0x02 �� Ϊadaptation field���ݣ�����Ч�غɣ����������ֶ�,�հ�ӦΪ0x2
	// 0x03 �� ��������Ƶ��������adaptation field���ݡ�
	unsigned adaptation_field_control: 2;
	//������.����ÿһ��������ͬPID��TS����������ӣ������ﵽ���ֵ���ֻظ���0����ΧΪ0~15��
	//0x02,��ʾ��ǰ���͵���ͬ���͵İ��ǵ�3��
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
	//�̶�Ϊ0
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

	//У����
	unsigned CRC_32: 32;
} SDT;

//�̶���СΪ5
typedef struct SDT_Program
{
	//������ID,ʵ���Ͼ���PMT���е�program_number
	unsigned service_id: 16;
	unsigned reserved_future_used: 6;
	//1��ʾ��ǰ��ʵ���˸ý�Ŀ��EIT����
	unsigned EIT_schedule_flag: 1;
	//1��ʾ��ǰ��ʵ���˸ý�Ŀ��EIT����
	unsigned EIT_present_following_flag: 1;
	//����״̬��Ϣ.1-��δ����2-�����Ӻ����Ͽ�ʼ,3-����ͣ����,4-���ڲ���,����---����
	unsigned running_status: 3;
	//''1''��ʾ�ý�Ŀ������. �� ���ŵ���������,һ����Service descriptor,
	// ���������������Ի�ȡservive_idָ���Ľ�Ŀ�Ľ�Ŀ����.�����ʽ��ο� EN300468�е�Service descriptor����.
	unsigned free_CA_mode: 1;
	//��ǰservice_id���������ӵĳ���
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


typedef struct TS_PAT_Program
{
	unsigned program_number: 16;
	unsigned reserved: 3;
	unsigned program_map_PID: 13;
} TS_PAT_Program;

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
	unsigned reserved_1: 3;
	//��ʾ����һ���ֶο�ʼ��CRC32(��) [��һ���ֶΣ�CRC_32]���տռ䡣֮�����õ��ֽ���������N loop��ʱ��Ҫ-12. for ( n = 0; n < packet->section_length - 12; n += 4 )
	unsigned section_length: 12;
	//�ô�������ID��������һ��������������·���õ���
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//��Χ0-31����ʾPAT�İ汾��
	unsigned version_number: 5;
	//��ǰδ����־����ʾ��ǰ��Ч����δ����Ч��һ����0
	unsigned current_next_indicator: 1;
	//�ֶεĺ��룬��ʾ��ǰ�Ƿֶεĵڼ��Ρ�һ��Ϊ0
	// PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
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
	unsigned int N_loop_len;

	//У����
	unsigned CRC_32: 32;
} PAT;

//PES���Ƕ�������Ƶ��PES��С�ڵ���64K����Ƶ��һ��Ϊһ֡һ��PES����
//һ֡ͼ���PES��ͨ��Ҫ������TS�������䡣
// MPEG-2�й涨��һ��PES��������������TS�������䡣�������һ��PES�������һ��TS��û��װ������������ֽ�������������һ���µ�PES���γ�ʱ�������µ�TS������ʼ���䡣
typedef struct PES
{
	//24λ��ʼ��.�̶�0x000001
	unsigned packet_start_code_prefix: 24;
	//��PS���и��ֶα�ʶ��洢�Ļ����������ͺ������ţ���TS���и��ֶν���ʶ��洢�Ļ�����������
	//��Ƶȡֵ��0xc0-0xdf����ͨ��Ϊ0xc0����Ƶȡֵ��0xe0-0xef����ͨ��Ϊ0xe0��
	unsigned stream_id: 8;
	//16λ�����ڴ洢����pes���ݵĳ��ȡ�
	//0��ʾ���Ȳ����ƣ�ֻ����Ƶ���ݳ��Ȼᳬ��0xffff��
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
	//Ԥ��
	if (tsPacket->adaptation_field_control == 0)
	{
		perror("adaptation_field_control == 0");
		return -1;
	}

	//���Һ���
	//������TSͷ
	unsigned int payloadOffset = 4;

	//�е����ֶΣ����������ֶ�
	if (tsPacket->adaptation_field_control == 3)
	{
		printf("TS Package has adaptation field\n");
		unsigned char adaptation_field_length = 0;
		memcpy(&adaptation_field_length, rtpPayload + payloadOffset, 1);
		printf("adaptation_field_length:[%d]\n", adaptation_field_length);
		//��Ҫ���ϵ�һ�������ֽ�+1
		payloadOffset = payloadOffset + adaptation_field_length + 1;
	}

	tsPacket->payload = rtpPayload + payloadOffset;

	//���س���
	tsPacket->payloadLen = 0;

	return 0;
}

//�ж�PID�Ƿ�ʱPMT
//PMT�ڴ�����������ָʾ���ĳһ�׽�Ŀ����Ƶ����Ƶ�������ڴ������е�λ�ã�����Ӧ��TS����PIDֵ���Լ�ÿ·��Ŀ�Ľ�Ŀʱ�Ӳο���PCR���ֶε�λ��

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
	//-4 ,��ȥCRC_32��ָ���ƶ���CRC_32ǰ
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
	//+3�ƶ�����һλ��-4 ,��ȥCRC_32��ָ���ƶ���CRC_32ǰ
	memcpy(&CRC_32, payload + 3 + pPat->section_length - 4, sizeof(CRC_32));
	pPat->CRC_32 = CRC_32;
	return 0;
}

int parseES()
{
	return 0;
}

#endif //UDP_RTP_UDP_RTP_TS_H_
