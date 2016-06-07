/**************************************************************
**  File        : main.c (server)                            **
**  Version     : 2.0                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 19.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdio.h>
#include <string.h>

#include "VS_LAB/Macros.h"
#include "VS_LAB/serverAPI.h"
#include "VS_LAB/commonAPI.h"

#include "plreglib.h"

#define TIMETODEATH 1500


int main(int argc, char **argv)
{
    msg packet;
    FID fid;
    uint8_t err, prio, *ip_ptr, my_prio;
    uint16_t gp, *data, bid, pkt_bid, pkt_port, my_client_port = 0;
    uint32_t seq_num = 0, target_client_ip, my_client_ip=0, *gp_ptr, *data_ptr, data_len, i;
    int16_t cid = -1, pkt_cid, last_logged_cid = -1;
    uint32_t t1 = 0;

    int server_num = 0;

    if (argc > 1)
        server_num = atoi(argv[1]);

    int temp_result;

    if(SUCCESS == init_server())
    {
        printf("\nServer %d: Started successfully!\n\n",server_num);
    }
    else
    {
        printf("\nServer %d: Starting failed!\n\n",server_num);
    	return -1;
    }

	PLREG_Open(str2ul(PLREG_ADDR_GP), &gp_ptr);
	PLREG_Open(str2ul(PLREG_ADDR_DATA), &data_ptr);

	/********** SUPER LOOP ***************************************************/

	do {

        err = recv_msg(&packet, &target_client_ip, &pkt_port);
        if(err == ERR_NO_PACKET)
		{
            if(t1 % 100 == 0)
                printf("Server %d: Timeoutcounter %d\n",server_num,(t1 / 100));
            t1++;
        	if(t1 >= TIMETODEATH)
        	{
    			t1 = 0;
                printf("Server %d: >> Timeout!\n",server_num);
    			if(my_client_ip != 0)
				{
//					err = send_unlock_rsp(my_client_ip,my_client_port);
                    err = send_error_rsp(ERR_LOCK_TIMEOUT, 0, GP_REQ,my_client_ip, my_client_port);
					if(err != NO_ERROR)
					{
                        printf("Server %d: Sending unlock / timeout response failed: %d\n",server_num,err);
					}
					ip_ptr = (uint8_t*)&my_client_ip;
                    printf("Server %d: Unlock response sent to client %d @ %d.%d.%d.%d on port %d\n",server_num,cid,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],my_client_port);
					my_client_ip = 0;
					my_client_port = 0;
				}
    			printf("\n");
    			cid = -1;
    			bid = 0;
        	}
        	continue;
		}

        if(err != NO_ERROR)
		{
            printf("Server %d: Error recv_msg: %d\n",server_num,err);
			if(err == ERR_INVALIDVERSION)
				send_error_rsp(err, 0, 0,target_client_ip,pkt_port);
			else if(err == ERR_NOSUCHFUNCTION)
				send_error_rsp(err,0,0,target_client_ip,pkt_port);
			free_msg(&packet);
			continue;
		}

        ip_ptr = (uint8_t*)&target_client_ip;
        fid = get_msg_type(&packet);
		switch(fid) {

		/********** GENERATOR POLYNOM REQUEST ********************************/

			case GP_REQ:
                printf("\nServer %d: Received GP Request!\n",server_num);
                err = extract_gp_req(&packet,&gp,&pkt_cid,&prio);
				if(err != NO_ERROR)
				{
                    printf("Server %d: Extracting GP_REQ-Packet was not possible: %d\n",server_num,err);
                    send_error_rsp(err, 0/*bid*/, fid, target_client_ip, pkt_port);
					break;
                }
                if(pkt_cid < 0)
                {
                    printf("Server %d: Client ID was below 0: %d\n",server_num,pkt_cid);
                    send_error_rsp(ERR_UNKNOWN,0,GP_REQ,target_client_ip, pkt_port);
                    break;
                }
                if((cid != -1) && (cid != pkt_cid))	// locked and another one tries to lock
                {
                	if(prio < my_prio)
                	{
                		my_prio = prio;
                		err = send_unlock_rsp(my_client_ip,my_client_port);
                        printf("Server %d: New Client locked, CID: %d, GP: %x\n",server_num,pkt_cid,gp);
                	}
                	else
                	{
                        printf("Server %d: GP_REQ not possible client %d, I'm already locked by %d\n",server_num,pkt_cid,cid);
						err = send_error_rsp(ERR_SERVERINUSE, 0, fid, target_client_ip, pkt_port);
						break;
                	}
                }
				if(PLREG_SetGeneratorPolynom(gp_ptr, gp) < 0)
				{
                    printf("Server %d: Setting GP failed internal\n",server_num);
                    send_error_rsp(ERR_FUNCTIONEXEC, 0/*bid*/, fid, target_client_ip, pkt_port);
					break;
				}
                my_client_ip = target_client_ip;
                my_client_port = pkt_port;
                my_prio = prio;
                cid = pkt_cid;
				last_logged_cid = cid;
                printf("Server %d: Client (ID: %d) set generator polynom: %x\n",server_num,cid,gp);
                // Sequence number initialisieren
                seq_num = 0;
				err = send_gp_rsp(my_client_ip,my_client_port);
				if(err != NO_ERROR)
				{
                    printf("Server %d: An error occured on sending ack for GP_REQ: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
                    break;
				}
                printf("Server %d: GP response sent to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],my_client_port);
                t1=0;
                break;

		/********** DECRYPT REQUEST ******************************************/

			case DECRYPT_REQ:
                printf("\nServer %d: Received Data Request!\n",server_num);
                err = extract_dec_req(&packet,&pkt_cid,&pkt_bid,&data,&data_len);
				if(err != NO_ERROR)
				{
                    printf("Server %d: Extracting DECRYPT_REQ-Packet was not possible: %d\n",server_num,err);
                    send_error_rsp(err, pkt_bid, fid, target_client_ip, pkt_port);
					break;
				}
                if(cid == -1)
                {
                    printf("Server %d: DECRYPT_REQ not possible, server not locked: %d\n",server_num,cid);
                    send_error_rsp(ERR_NO_GP, pkt_bid, fid, target_client_ip, pkt_port);
                    break;
                }
                if(cid != pkt_cid)
                {
                    printf("Server %d: DECRYPT_REQ not possible, server locked by %d, received request by %d\n",server_num,cid,pkt_cid);
                    send_error_rsp(ERR_SERVERINUSE, pkt_bid, fid, target_client_ip, pkt_port);
                    break;
                }
                bid = pkt_bid;

                // Scrambling
                printf("Server %d: Start decrypting block %d for client %d\n",server_num,bid,cid);
                for(i=0; i<data_len;i++)
				{
                    PLREG_Scramble(data_ptr, data[i], &temp_result);
                    ((uint8_t*)data)[i] = (uint8_t)(temp_result & 0xFF);
                    seq_num++;
                    t1=0;
				}
                ((uint8_t*)data)[data_len] = '\0';
                printf("Server %d: Finished scrambling block %d\n",server_num,bid);
                //printf("Decrypted Sequence: \"%s\"\n", (uint8_t*)data);
                err = send_dec_rsp(bid,cid,(uint8_t*)data,data_len,my_client_ip,my_client_port);
				if(err != NO_ERROR)
				{
                    printf("Server %d: An error occured on sending ack for DECRYPT_REQ: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
                    break;
				}
                printf("Server %d: Sent decrypt response to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],my_client_port);
                free(data);
                t1=0;
                break;

		/********** UNLOCK REQUEST *******************************************/

			case UNLOCK_REQ:
                printf("\nServer %d: Received Unlock Request!\n",server_num);
                err = extract_unlock_req(&packet, &cid);
				if(err != NO_ERROR)
				{
                    printf("Server %d: Extracting UNLOCK_REQ-Packet not possible: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
					break;
				}
                if(cid == -1)
                {
                    printf("Server %d: UNLOCK_REQ not possible, server is not locked\n",server_num);
                    send_error_rsp(ERR_NOTFORME, bid, fid, target_client_ip, pkt_port);
                    break;
                }
                if(cid != pkt_cid)
                {
                    printf("Server %d: UNLOCK_REQ not possible, server locked by %d, received request from %d\n",server_num,cid,pkt_cid);
                    send_error_rsp(ERR_NOTFORME, bid, fid, target_client_ip, pkt_port);
                    break;
                }

                // unlock server now
                cid = -1;
                my_client_ip = 0;
                my_client_port = 0;
                err = send_unlock_rsp(target_client_ip,pkt_port);
				if(err != NO_ERROR)
				{
                    printf("Server %d: An error occured on sending ack for UNLOCK_REQ: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
                    break;
				}
                printf("Server %d: Unlock response sent to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],pkt_port);
                break;

        /********** BROADCAST REQUEST ****************************************/

			case BROADCAST_REQ:
                printf("\nServer %d: Received Broadcast Request!\n",server_num);
                err = extract_brdcst_req(&packet);
				if(err != NO_ERROR)
				{
                    printf("Server %d: Extracting BROADCAST_REQ-Packet not possible: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
					break;
				}
				err = send_brdcst_rsp(target_client_ip,pkt_port);
				if(err != NO_ERROR)
				{
                    printf("Server %d: An error occured on sending ack for BROADCAST_REQ: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
                    break;
				}
                printf("Server %d: Broadcast response sent to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],pkt_port);
                break;

        /********** STATUS REQUEST *******************************************/

			case STATUS_REQ:
                printf("\nServer %d: Received Status Request!\n",server_num);
                err = extract_status_req(&packet);
				if(err != NO_ERROR)
				{
                    printf("Server %d: Extracting Status_REQ-Packet not possible: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
					break;
				}
                err = send_status_rsp(cid, seq_num, target_client_ip, pkt_port);
				if(err != NO_ERROR)
				{
                    printf("Server %d: An error occured on sending ack for STATUS_REQ: %d\n",server_num,err);
                    send_error_rsp(err, bid, fid, target_client_ip, pkt_port);
                    break;
				}
                printf("Server %d: Status response sent to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],pkt_port);
                break;

        /********** UNKNOWN FUNCTION ID **************************************/

			case UNKNOWN:
                printf("\nServer %d: Received packet that calls unknown function!\n",server_num);
                send_error_rsp(ERR_NOSUCHFUNCTION, bid, fid, target_client_ip, pkt_port);
                printf("Server %d: 'No such function'-Frame sent to %d.%d.%d.%d on port %d\n",server_num,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],pkt_port);
                break;

			default:
				break;
		}
        free_msg(&packet);
	} while(1);

	/********** END SUPER LOOP ***************************************************/

	PLREG_Close(str2ul(PLREG_ADDR_GP), gp_ptr);
	PLREG_Close(str2ul(PLREG_ADDR_DATA), data_ptr);
	deinit_server();

    printf("Server %d: Closed! Bye Bye\n\n",server_num);
    return 0;
}
