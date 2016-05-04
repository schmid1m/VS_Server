/**************************************************************
**  File        : main.c (server)                            **
**  Version     : 0.1                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 03.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdio.h>
#include <string.h>

#include "VS_LAB/Macros.h"
#include "VS_LAB/serverAPI.h"
#include "VS_LAB/commonAPI.h"

#include "plreglib.h"

int main(void)
{
    msg packet;
    uint32_t seq_num = 0, blk_ID = 0, target_client_ip, *gp_ptr, *data_ptr, data_len;
    FID fid;
    uint8_t err, prio, *ip_ptr;
    uint16_t gp, cid, *data, bid, temp_result;

    if (SUCCESS == init_server())
    {
    	printf("Server started successfully!\n\n");
    }
    else
    {
    	printf("Server failed starting!\n\n");
    	return -1;
    }

	PLREG_Open(str2ul(PLREG_ADDR_GP), &gp_ptr);
	PLREG_Open(str2ul(PLREG_ADDR_DATA), &data_ptr);


	do {

        err = recv_msg(&packet, &target_client_ip);
		if(err != NO_ERROR)
		{
			printf("Error recv_msg: %d\n", err);
		}

        ip_ptr = (uint8_t*)&target_client_ip;

        fid = get_msg_type(&packet);

		switch (fid) {
			case GP_REQ:
                err = extract_gp_req(&packet,&gp,&cid,&prio);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, 0/*blk_ID*/, target_client_ip, fid);
					break;
				}

				if (PLREG_SetGeneratorPolynom(gp_ptr, gp) < 0)
				{
					send_error_rsp(ERROR, 0/*blk_ID*/, target_client_ip, fid);
					break;
				}
				printf("Generator Polynom is set: %x\n",gp);

				err = send_gp_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
				}
                printf("GP Response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
				break;

			case DECRYPT_REQ:
                err = extract_dec_req(&packet,&cid,&bid,&data,&data_len);
				printf("Decrypt started!\n");
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
					break;
				}

				// Scrambling
                for(uint32_t i=0; i<data_len;i++)
				{
					PLREG_Scramble(data_ptr, data[i], (int*)&temp_result);
					data[i] = temp_result;
				}
				printf("Scrambling done!\n");
				// sequence number inkrementieren
                //
                //
                // TODO copy relevant bytes together
                //
                //
                err = send_dec_rsp(bid,cid,(uint8_t*)data,data_len,target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
				}
                printf("Decrypt response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
				break;

			case UNLOCK_REQ:
                err = extract_unlock_req(&packet, &cid);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
					break;
				}

				err = send_unlock_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
				}
                printf("Unlock response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;

			case BROADCAST_REQ:
                err = extract_brdcst_req(&packet);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
					break;
				}

				err = send_brdcst_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
				}
                printf("Broadcast response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;

			case STATUS_REQ:
                err = extract_status_req(&packet);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
					break;
				}

                err = send_status_rsp(cid, seq_num, target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
					send_error_rsp(err, blk_ID, target_client_ip,fid);
				}
                printf("Status response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;
			case UNKNOWN:
				send_error_rsp(ERR_UNKNOWN, blk_ID, target_client_ip,fid);
				break;
			default:
				break;
		}
        free_msg(&packet);
	} while (1);

	PLREG_Close(str2ul(PLREG_ADDR_GP), gp_ptr);
	PLREG_Close(str2ul(PLREG_ADDR_DATA), data_ptr);
	deinit_server();

	printf("Server closed! \nBye Bye\n\n");
    return 0;
}

