#include <stdio.h>
#include <string.h>
#include "VS_LAB/Macros.h"
#include "VS_LAB/serverAPI.h"
#include "VS_LAB/commonAPI.h"

int main(void)
{
    printf("Hello World!\n\n");

    printf("init: %d\n",init_server());

    msg* packet = malloc(sizeof(msg));
    uint32_t src_ip;
    FID fid;
    uint8_t err, prio;
    uint16_t gp, cid;


    err = recv_msg(packet, &src_ip);
    printf("Error: %d\n", err);

    fid = get_msg_type(packet);

    if(fid == GP_REQ)
    {
        printf("Msg Received\n");
    }
    else
    {
        printf("Wrong decoded?\n");
        free_msg(packet);
        return 0;
    }

    extract_gp_req(packet,&gp,&cid,&prio);

    printf("GP: %x; CID: %x; Prio: %x\n", gp, cid, prio);

    free_msg(packet);
    return 0;
}

