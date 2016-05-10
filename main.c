/**************************************************************
**  File        : main.c (server)                            **
**  Version     : 1.0                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 10.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdio.h>
#include <string.h>

#include "VS_LAB/Macros.h"
#include "VS_LAB/serverAPI.h"
#include "VS_LAB/commonAPI.h"

#include "plreglib.h"
#include "timeoutlib.h"

int main(void)
{
    msg packet;
    FID fid;
    uint8_t err, prio, *ip_ptr;
    uint16_t gp, *data, bid, pkt_bid;
    uint32_t seq_num = 0, target_client_ip, my_client_ip, *gp_ptr, *data_ptr, data_len, i;
    int16_t cid = -1, pkt_cid;
    int temp_result;

    if(SUCCESS == init_server())
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
        if (tol_is_timed_out()) {
            tol_reset_timeout();
            printf("Timeout: unlocking client\n");
            cid = -1;
            bid = 0;
            err = send_unlock_rsp(my_client_ip);
            if(err != NO_ERROR)
            {
                printf("An error occured: %d\n",err);
            }
            else
            {
                printf("Unlock response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
            }
            continue;
        }
        //printf("Packet received\n");
        if(err != NO_ERROR)
		{
			printf("Error recv_msg: %d\n", err);
			free_msg(&packet);
			continue;
		}

        ip_ptr = (uint8_t*)&target_client_ip;

        fid = get_msg_type(&packet);

		switch (fid) {
			case GP_REQ:
                err = extract_gp_req(&packet,&gp,&pkt_cid,&prio);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, 0/*bid*/, fid, target_client_ip);
					break;
                }
                if((cid != -1) && (cid != pkt_cid))
                {
                    printf("ERROR: Server locked by %d\n", cid);
                    err = send_error_rsp(ERR_SERVERINUSE, 0, fid, target_client_ip);
                    break;
                }
				if(PLREG_SetGeneratorPolynom(gp_ptr, gp) < 0)
				{
                    send_error_rsp(ERR_FUNCTIONEXEC, 0/*bid*/, fid, target_client_ip);
					break;
				}
                my_client_ip = target_client_ip;
                tol_start_timeout(TOL_TIMEOUT_SECS);
                cid = pkt_cid;
                printf("Client %d set Generator Polynom: %x\n",cid, gp);
                // Sequence number initialisieren
                seq_num = 0;
				err = send_gp_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
                    break;
				}
                printf("GP Response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
				break;

			case DECRYPT_REQ:
                err = extract_dec_req(&packet,&pkt_cid,&pkt_bid,&data,&data_len);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, pkt_bid, fid, target_client_ip);
					break;
				}
                if(cid == -1)
                {
                    printf("ERROR: Server not locked.\n");
                    send_error_rsp(ERR_NO_GP, pkt_bid, fid, target_client_ip);
                    break;
                }
                if(cid != pkt_cid)
                {
                    printf("ERROR: Server locked by %d\n", cid);
                    send_error_rsp(ERR_SERVERINUSE, pkt_bid, fid, target_client_ip);
                    break;
                }
                bid = pkt_bid;
                printf("Decrypt started!\n");
                // Scrambling
                tol_stop_timeout();
                for(i=0; i<data_len;i++)
				{
                    PLREG_Scramble(data_ptr, data[i], &temp_result);
                    ((uint8_t*)data)[i] = (uint8_t)(temp_result & 0xFF);
                    seq_num++;
				}
                ((uint8_t*)data)[data_len] = '\0';
                my_client_ip = target_client_ip;
                tol_start_timeout(TOL_TIMEOUT_SECS);
				printf("Scrambling done!\n");
                //printf("Decrypted Sequence: \"%s\"\n", (uint8_t*)data);
                err = send_dec_rsp(bid,cid,(uint8_t*)data,data_len,target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
                    break;
				}
                printf("Decrypt response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
				break;

			case UNLOCK_REQ:
                err = extract_unlock_req(&packet, &cid);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
					break;
				}
                if((cid == -1) || (cid != pkt_cid))
                {
                    printf("ERROR: Cannot unlock! Server locked by %d\n", cid);
                    send_error_rsp(ERR_NOTFORME, bid, fid, target_client_ip);
                    break;
                }
                tol_stop_timeout();
                cid = -1;
                err = send_unlock_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
                    break;
				}
                printf("Unlock response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;

			case BROADCAST_REQ:
                err = extract_brdcst_req(&packet);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
					break;
				}
				err = send_brdcst_rsp(target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
                    break;
				}
                printf("Broadcast response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;

			case STATUS_REQ:
                err = extract_status_req(&packet);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
					break;
				}

                err = send_status_rsp(cid, seq_num, target_client_ip);
				if(err != NO_ERROR)
				{
					printf("An error occured: %d\n",err);
                    send_error_rsp(err, bid, fid, target_client_ip);
                    break;
				}
                printf("Status response sent to %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
                break;

			case UNKNOWN:
                send_error_rsp(ERR_UNKNOWN, bid, fid, target_client_ip);
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

