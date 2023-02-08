//
// Created by young on 2023/2/4.
//
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "rtp.h"
#include "ts.h"

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
	char recvData[1500];
	printf("Accepting connections ...\n");
	int cnt = 0;

	PAT pat = { 0 };
	PMT pmt = { 0 };
	while (1)
	{
		int pktsize = recvfrom(serSocket, recvData, 1500, 0, (struct sockaddr*)&remoteAddr, &nAddrLen);
		/*printf("recv dataLen:%d from %s:%d \r\n",
			pktsize,
			inet_ntoa(remoteAddr.sin_addr),
			ntohs(remoteAddr.sin_port));*/

		RTP_PACKET rtpPacket;
		int ret = parseRTP(recvData, pktsize, &rtpPacket);
		if (ret < 0)
		{
			perror("parseRTP error");
			continue;
		}

/*		fprintf(myout,
			"[RTP Pkt] | num:[%d] | pt:[%u] | ts:[%10u] | no:[%5d] | %5d|\n",
			cnt,
			rtpPacket.payloadType,
			rtpPacket.timestamp,
			rtpPacket.seqNo,
			rtpPacket.payloadLen);*/

		//fwrite(rtpPacket.payload, 1, rtpPacket.payloadLen, fp1);

		if (rtpPacket.payloadType == 33)
		{
			for (unsigned int i = 0; i < rtpPacket.payloadLen; i += 188)
			{
				TS_PACKET tsPacket = { 0 };
				ret = parseTS(rtpPacket.payload + i, &tsPacket);
				if (ret < 0)
				{
					perror("parseTS error");
					continue;
				}
/*				printf("TS header | payload_unit:[%x] | PID:[%x] | filed:[%x] | cc:[%d]\n",
					tsPacket.payload_unit_start_indicator,
					tsPacket.PID,
					tsPacket.adaptation_field_control,
					tsPacket.continuity_counter);*/

				if (tsPacket.payload_unit_start_indicator == 1)
				{
					int offset = 0;
					if (tsPacket.PID == 0)
					{
						memcpy(&offset, tsPacket.payload, 1);
						tsPacket.payload = tsPacket.payload + 1 + offset;
					}

					for (int j = 0, index = 0; j < pat.program_len; j += 4)
					{
						if (tsPacket.PID == pat.program[index].program_map_PID)
						{
							memcpy(&offset, tsPacket.payload, 1);
							tsPacket.payload = tsPacket.payload + 1 + offset;
							break;
						}
						index++;
					}

				}


				//PAT
				if (tsPacket.PID == 0x00)
				{
					parsePAT(tsPacket.payload, &pat);
					printf("PAT Program |");
					for (int j = 0, index = 0; j < pat.program_len; j += 4)
					{
						printf(" program_num:[%x]  program_map_PID:[%x] \n",
							pat.program[index].program_number,
							pat.program[index].program_map_PID);
						index++;
					}
					continue;
				}

				//PMT
				for (int j = 0, index = 0; j < pat.program_len; j += 4)
				{
					if (pat.program[index].program_map_PID == tsPacket.PID)
					{
						parsePMT(tsPacket.payload, &pmt);
						for (int K = 0, V = 0; K < pmt.pmt_stream_len;)
						{
							unsigned char stream_type = pmt.pmt_stream[V].stream_type;
							unsigned short ele_PID = pmt.pmt_stream[V].elementary_PID;
							printf("PMT Program | stream_type:[%x]  elementary_PID:[%x] \n",
								stream_type,
								ele_PID);
							K = K + 5 + pmt.pmt_stream[V].ES_info_length;
							V++;
						}
						continue;
					}
				}

				/*switch (tsPacket.PID)
				{
				case 0x00:
					parsePAT(tsPacket.payload, &pat);
					printf("PAT Program |");
					for (int j = 0, index = 0; j < pat.N_loop_len; j += 4)
					{
						printf(" program_num:[%x]  program_map_PID:[%x] \n",
							pat.N_loop[index].program_number,
							pat.N_loop[index].program_map_PID);
						index++;
					}
					break;
				case 0x20:
					parsePMT(tsPacket.payload, &pmt);
					printf("PMT Program |");
					for (int j = 0, index = 0; j < pmt.pmt_stream_len;)
					{
						printf(" stream_type:[%x]  elementary_PID:[%x] \n",
							pmt.pmt_stream[index].stream_type,
							pmt.pmt_stream[index].elementary_PID);
						j = j + 4 + pmt.pmt_stream[index].ES_info_length;
						index++;
					}
					break;
				case 0x11:
					//parseSDT(&tsPacket);
					break;
				default:
					//parsePES(tsPacket.payload);
					//parseES();
				}*/
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