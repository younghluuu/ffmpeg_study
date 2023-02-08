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
	FILE* fp1 = fopen("./output_dump.h264", "wb+");
	if (fp1 == NULL)
	{
		perror("fp1 open fail");
		return -1;
	}

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
	unsigned char streamListSize = 16;
	STREAM streamList[streamListSize];
	memset(streamList, 0, streamListSize * sizeof(STREAM));
	//STREAM* streamList = malloc(streamListSize * sizeof(STREAM));
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

							int k = 0;
							for (k = 0; k < streamListSize; ++k)
							{
								if (streamList[k].PID == ele_PID)
								{
									break;
								}
							}

							if (k == streamListSize)
							{
								STREAM stream = { 0 };
								stream.PID = ele_PID;
								for (k = 0; k < streamListSize; ++k)
								{
									if (streamList[k].PID == 0)
									{
										streamList[k] = stream;
										break;
									}
								}
								if (k == streamListSize)
								{
									printf("streamList full !!!\n");
									break;
								}
							}
							K = K + 5 + pmt.pmt_stream[V].ES_info_length;
							V++;
						}
						break;
					}
				}

				//PES
				for (int j = 0; j < streamListSize; ++j)
				{
					if (streamList[j].PID == tsPacket.PID)
					{
						if (tsPacket.payload_unit_start_indicator == 1)
						{
							if (streamList[j].buf != NULL)
							{
								if (streamList[j].stream_type == 0xe0)
								{
									fwrite(streamList[j].buf, 1, streamList[j].bufLen, fp1);
								}
								free(streamList[j].buf);
								streamList[j].buf = NULL;
							}

							PES pes;
							ret = parsePES(tsPacket.payload, &pes);
							if (ret < 0)
								break;
							streamList[j].stream_type = pes.stream_id;
							streamList[j].packet_length = pes.packet_length;
							streamList[j].continuity_counter = (tsPacket.continuity_counter + 1) % 16;
							streamList[j].buf = malloc(pes.packet_length);

							memcpy(streamList[j].buf,
								tsPacket.payload + pes.payload_offset,
								tsPacket.payloadLen - pes.payload_offset);
							streamList[j].bufLen = tsPacket.payloadLen - pes.payload_offset;
						}
						else
						{
							if (streamList[j].continuity_counter == tsPacket.continuity_counter)
							{
								if (streamList[j].bufLen + tsPacket.payloadLen > streamList[j].packet_length)
								{
									printf("packet size error\n");
									free(streamList[j].buf);
									streamList[j].buf = NULL;
									streamList[j].PID = 0;
									break;
								}
								if (streamList[j].stream_type == 0xe0)
								{
									int a = 3;
								}
								memcpy(streamList[j].buf + streamList[j].bufLen, tsPacket.payload, tsPacket.payloadLen);
								streamList[j].bufLen += tsPacket.payloadLen;
								streamList[j].continuity_counter = (tsPacket.continuity_counter + 1) % 16;
							}
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
		if (cnt == 2000)
			break;
	}

	//Close
	fclose(fp1);
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