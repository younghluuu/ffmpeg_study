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
	//�ٸ����ӣ�һ��PES�������ɶ��TS�����ɣ���һ��TS���ĸ��ص�Ԫ��ʼָ��λ�Żᱻ��λ
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

	//�ô�������ID��������һ��������������·���õ���,��Ϊһ����ǩ�����ֶ�ָ������������
	//�����������������־����ֵ���û����塣
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	//��Χ0-31����ʾPAT�İ汾��
	unsigned version_number: 5;
	//��ǰδ����־����ʾ��ǰ��Ч����δ����Ч����0ʱ�������ô��͵ı�ֶβ���ʹ�ã���һ����ֶβ���Ч��
	unsigned current_next_indicator: 1;
	//�ֶεĺ��룬��ʾ��ǰ�Ƿֶεĵڼ��Ρ�һ��Ϊ0
	// PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	unsigned section_number: 8;
	//���һ���ֶεĺ���.�������һ���ֶκţ�ͬʱ������PAT�����ֶ���Ŀ��һ�㣬һ��PAT����һ��TS������
	unsigned last_section_number: 8;

	//�����洢NIT
	//NIT���PIDֵ��
	unsigned program_number: 16;
	unsigned reserved_3: 3; // ����λ
	unsigned network_PID: 13; //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID

	//ֻ�洢PMT
	//PMT���PIDֵ
	//program_number == 0 then NIT_PID else PMT_PID
	struct TS_PAT_Program* program;
	unsigned int program_len;

	//У����
	unsigned CRC_32: 32;
} PAT;

typedef struct TS_PAT_Program
{
	//��Ŀ�ı��
	unsigned program_number: 16;
	unsigned reserved: 3;
	//PMT���PID
	unsigned program_map_PID: 13;
} TS_PAT_Program;

typedef struct PMT
{
	//��ʶ�ֶΡ�ָ����һ��TS��PSI�ֶε������ǽ�Ŀ�����ֶΡ��������ʷֶΡ�TS��Ŀӳ��ֶΡ���PAT/CAT/PMT��
	//�̶�Ϊ0x02, ��ʾPMT��
	unsigned table_id: 8;
	unsigned section_syntax_indicator: 1; //�̶�Ϊ0x01
	unsigned reserved_1: 3; //0x03
	//ָʾ�ֶε��ֽ��������ȵ��ڸ� �ֶ�֮�� CRC32 �� �������ڣ����ֽ�����
	unsigned section_length: 12;

	unsigned program_number: 16;// ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID
	unsigned reserved_2: 2; //0x03
	unsigned version_number: 5; //ָ��TS����Program map section�İ汾��
	//����λ��1ʱ����ǰ���͵�Program map section���ã�
	unsigned current_next_indicator: 1;
	//�ֶεĺ��룬��ʾ��ǰ�Ƿֶεĵڼ��Ρ�һ��Ϊ0
	//�̶�Ϊ0x00
	unsigned section_number: 8;
	//���һ���ֶεĺ���.�������һ���ֶκţ�ͬʱ������PAT�����ֶ���Ŀ��
	//�̶�Ϊ0x00
	unsigned last_section_number: 8;

	unsigned reserved_3: 3; //0x07
	//ָ��TS����PIDֵ����TS������PCR�򣬸�PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ��
	//�������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��
	unsigned PCR_PID: 13;
	unsigned reserved_4: 4; //Ԥ��Ϊ0x0F
	//�����������ĶԽ�Ŀ��Ϣ�������ֽ�����Ҳ���ǵ�һ��N loop descriptors���ֽ�����
	unsigned program_info_length: 12;

	struct PMT_STREAM* pmt_stream;
	unsigned int pmt_stream_len;

	//��CEDARX�����н���DVB�ĳ�������У�顣
	//У����
	unsigned CRC_32: 32;
} PMT;

typedef struct PMT_STREAM
{
	//ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��
	//����PES�������͡�Ʃ�磬0x01������MPEG-1��Ƶ��0X03������MPEG-1��Ƶ��
	//0X1b H.254 0X0F AAC
	unsigned stream_type: 8;
	unsigned reserved_1: 3;
	//����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��
	//�����ø����и�PES����TS����PIDֵ
	unsigned elementary_PID: 13;
	unsigned reserved_2: 4;
	//������������������ؽ�ĿԪ�ص��ֽ�����Ҳ���ǵڶ���N loop descriptors���ֽ�����
	//ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
	unsigned ES_info_length: 12;
} PMT_STREAM;

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
	unsigned packet_length: 16;

	unsigned reserved_0: 2;
	unsigned scrambling_control: 2;
	unsigned priority: 1;
	unsigned data_alignment_indicator: 1;
	unsigned copyright: 1;
	unsigned original_or_copy: 1;

	/*
	 * 0x00	PES��ͷ�м����κ�PTS�ֶ�Ҳ���κ�DTS�ֶδ��ڡ�
	 * 0x01	����
	 * 0x02	PES��ͷ��PTS�ֶδ��ڡ�
	 * 0x03	PES��ͷ��PTS�ֶκ�DTS�ֶξ����ڡ�
	 */
	unsigned pts_dts_flags: 2;
	//0:ָʾ���κ�ESCR�ֶδ���
	//1:ָʾPES��ͷ��ESCR��׼�ֶκ�ESCR��չ�ֶξ�����
	unsigned escr_flag: 1;
	//���ڡ�1��ʱָʾPES��ͷ��ES_rate �ֶδ��ڡ����ڡ�0��ʱָʾ���κ� ES_rate �ֶδ���
	unsigned es_rate_flag: 1;
	//���ڡ�1��ʱָʾ8�����ؼ���ʽ�ֶδ��ڡ����ڡ�0��ʱָʾ���ֶβ�����
	unsigned dsm_trick_mode_flag: 1;
	//���ڡ�1��ʱָʾadditional_copy_info ���ڡ����ڡ�0��ʱָʾ���ֶβ�����
	unsigned additional_copy_info_flag: 1;
	//���ڡ�1��ʱָʾPES����CRC�ֶδ��ڡ����ڡ�0��ʱָʾ���ֶβ�����
	unsigned crc_flag: 1;
	//���ڡ�1��ʱָʾPES��ͷ����չ�ֶδ��ڡ����ڡ�0��ʱָʾ���ֶβ�����
	unsigned extension_flag: 1;

	//ָʾ�ڴ�PES��ͷ�а���������ѡ�ֶκ���������ֽ���ռ�ݵ��ֽ�������
	//��ѡ�ֶεĴ�����ǰ�� PES_header_data_length �ֶε��ֽ���ָ��
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
