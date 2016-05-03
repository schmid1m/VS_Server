#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "VS_LAB/Macros.h"
#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"

int main(void)
{
    // Parameters: client ID, client priority, broadcast address

	msg* packet = malloc(sizeof(msg));
	uint32_t seq_num, blk_ID, target_client_ip, *gp_ptr, *data_ptr;
	FID fid;
	uint8_t err, prio;
	uint16_t gp, cid, *data, data_len, bid, temp_result;

    init_client(1, 1, inet_network(BROADCAST_ADDRESS));

    // Parameters: generator polynom, server ip address
    send_gp_req(0x52AC, inet_network("10.0.3.129"));
    printf("GP_REQ sent!\n");
    err = recv_msg(packet, &target_client_ip);
    fid = get_msg_type(packet);

    if (fid == GP_RSP)
    {
    	err = extract_gp_rsp(packet);
    	printf("Received GP_RSP!\n");
    }

    free_msg(packet);

    data = calloc(1,sizeof(uint16_t));
    data[0] = 1;
    printf("data ready for transmit!\n");

    sleep(12);

    send_dec_req(1,data,1,inet_network("10.0.3.129"));
    printf("data transmitted!\n");

    err = recv_msg(packet, &target_client_ip);
    fid = get_msg_type(packet);
    printf("received FID %d\n",fid);

    if (fid == DECRYPT_RSP)
    {
    	err = extract_dec_rsp(packet,&bid,&data,&data_len);
    	printf("Received DECRYPT_RSP! \n");
    	printf("%d %d %d",bid,data_len,data[0]);
    }
    free(packet);


    return 0;
}

