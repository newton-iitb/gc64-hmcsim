/* 
 * _EXECUTE_TEST_C_ 
 * 
 * HMCSIM PHYSRAND TEST EXECUTION FUNCTIONS
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "hmc_sim.h"


/* ---------------------------------------------------- ZERO_PACKET */
/* 
 * ZERO_PACKET 
 * 
 */
static void zero_packet( uint64_t *packet )
{
	uint64_t i = 0x00ll;

	/* 
	 * zero the packet
	 * 
	 */
	for( i=0; i<HMC_MAX_UQ_PACKET; i++ ){
		packet[i] = 0x00ll;
	} 


	return ;
}

/* ---------------------------------------------------- EXECUTE_TEST */
/* 
 * EXECUTE TEST
 * 
 */
extern int execute_test(	struct hmcsim_t *hmc, 
				long num_req )
{
	/* vars */
	uint32_t z		= 0x00;
	uint32_t iter		= 0x00l;
	uint64_t head		= 0x00ll;
	uint64_t tail		= 0x00ll;
	uint64_t payload[8]	= {0x00ll,0x00ll,0x00ll,0x00ll,
				   0x00ll,0x00ll,0x00ll,0x00ll};
	uint8_t	cub		= 0;
	uint16_t tag		= 0;
	uint8_t link		= 0;
	int ret			= HMC_OK;
	
	FILE *ofile		= NULL;
	int *rtns		= NULL;
	long all_sent		= 0;
	long all_recv		= 0;
	int done		= 0;
	uint64_t *addr		= NULL;
	uint32_t *req		= NULL;
	uint64_t i;
	uint64_t packetSent[HMC_MAX_UQ_PACKET];
	uint64_t packetRecv[HMC_MAX_UQ_PACKET];
	uint64_t shiftamt = 7; /* 128 Byte 4GB */

    uint64_t d_response_head;
    uint64_t d_response_tail;
    hmc_response_t d_type;
    uint8_t d_length;
    uint16_t d_tag;
    uint8_t d_rtn_tag;
    uint8_t d_src_link;
    uint8_t d_rrp;
    uint8_t d_frp;
    uint8_t d_seq;
    uint8_t d_dinv;
    uint8_t d_errstat;
    uint8_t d_rtc;
    uint32_t d_crc;

	/* ---- */
	printf("NUMBER OF REQUESTS:%lu\n", num_req);

	rtns = malloc( sizeof( int ) * hmc->num_links );
	memset( rtns, 0, sizeof( int ) * hmc->num_links );
	
	req = malloc( sizeof( uint32_t ) * num_req );
	if( req == NULL ){ 
		printf( "FAILED TO ALLOCATE MEMORY FOR REQ\n" );
		free( addr ); addr = NULL;
		return -1;
	}
	
	for (int i = 0; i < num_req; i++)
	{
	    if (i % 2)
	    {
    	    req[i] = 1;  /* read */
	    }
	    else
	    {
    	    req[i] = 2; /* write */
	    }
	}


	addr = malloc( sizeof( uint64_t ) * num_req );
	if( addr == NULL ){ 
		printf( "FAILED TO ALLOCATE MEMORY FOR ADDR\n" );
		return -1;
	}

    for( i=0; i < num_req / 2; i++ ){ 

		addr[i] = i << (uint64_t)(shiftamt);
		printf("%2lu:Address: 0x%16lx\n", i, addr[i]);
	}
	
	int j = 0;
	for( i=num_req / 2; i < num_req; i++ ){ 

		addr[i] = j << (uint64_t)(shiftamt);
		printf("%2lu:Address: 0x%16lx\n", i, addr[i]);
		j++;
	}
	
	/* 
	 * Setup the tracing mechanisms
	 * 
	 */
	ofile = fopen( "physrand.out", "w" );	
	if( ofile == NULL ){ 
		printf( "FAILED : COULD NOT OPEN OUTPUT FILE physrand.out\n" );
		return -1;
	}

	hmcsim_trace_handle( hmc, ofile );
	hmcsim_trace_level( hmc, (HMC_TRACE_BANK|
				HMC_TRACE_QUEUE|
				HMC_TRACE_CMD|
				HMC_TRACE_STALL|
				HMC_TRACE_LATENCY) );

	printf( "SUCCESS : INITIALIZED TRACE HANDLERS\n" );				
	

	/* 
	 * zero the packet
	 * 
	 */
	zero_packet( &(packetSent[0]) );
	zero_packet( &(packetRecv[0]) );

	printf( "SUCCESS : ZERO'D PACKETS\n" );
	printf( "SUCCESS : BEGINNING TEST EXECUTION\n" );

	/* 
	 * Attempt to execute all the requests
	 * Push requests into the device
	 * until we get a stall signal 
 	 */ 
	while( done != 1 ){


		/* 
	 	 * attempt to push a request in 
		 * as long as we don't stall
		 *
	 	 */		
		if( iter >= num_req ){ 
			/* everything is sent, go to receive side */
			goto packet_recv;
		}

		printf( "....sending packets. Request Num %u\n", iter);
		while( ret != HMC_STALL ){

			/* 
			 * try to push another request 
			 * 
			 * Step 1: Build the Request
			 * 
			 */		
			if( req[(int)(iter)] == 1 ){
				/* 
			 	 * read request
				 *
				 */
				tag = 0xED; 
		        printf( "...building read Request Num %u\n", iter);
				printf( "...building read request for cube: %d link:%u\n", 0, link);
				hmcsim_build_memrequest( hmc, 
							0, 
							addr[iter], 
							tag, 
							RD64, 
							link, 
							&(payload[0]), 
							&head, 
							&tail );
				/* 
				 * read packets have: 
				 * head + 
				 * tail
				 * 
				 */
				packetSent[0] = head;
				packetSent[1] = tail;
			}else {
				/* 
				 * write request
				 *
				 */
				tag = 0xCD; 
		        printf( "...building write Request Num %u\n", iter);
				printf( "...building write request for cube: %d link:%u\n", 1, link);
				hmcsim_build_memrequest( hmc, 
							1, 
							addr[iter], 
							tag, 
							WR64, 
							link, 
							&(payload[0]), 
							&head, 
							&tail );
				/* 
				 * write packets have: 
				 * head + 
				 * data + 
				 * data + 
				 * data + 
				 * data + 
				 * data + 
				 * data + 
				 * data + 
				 * data + 
				 * tail
				 * 
				 */
				packetSent[0] = head;
				packetSent[1] = 0x05ll;
				packetSent[2] = 0x06ll;
				packetSent[3] = 0x07ll;
				packetSent[4] = 0x08l;
				packetSent[5] = 0x09ll;
				packetSent[6] = 0x0All;
				packetSent[7] = 0x0Bll;
				packetSent[8] = 0x0Cll;
				packetSent[9] = tail;
			}
			
			/* 
			 * Step 2: Send it 
			 *
			 */
			printf( "...sending packet : base addr=0x%016lx\n", addr[iter] );

            printf("PacketSent:");
			for (int i = 0; i < 16; i++)
			{
			    printf("%lx ", packetSent[i]);
			}
			
		    printf("\n");			
		    ret = hmcsim_send( hmc, &(packetSent[0]) );

			switch( ret ){ 
				case 0: 
					printf( "SUCCESS : PACKET WAS SUCCESSFULLY SENT\n" );
					all_sent++;
					iter++;
					break;
				case HMC_STALL:
					printf( "STALLED : PACKET WAS STALLED IN SENDING\n" );
					break;
				case -1:
				default:
					printf( "FAILED : PACKET SEND FAILED\n" );
					goto complete_failure;
					break;
			}

			/* 
			 * zero the packet 
			 * 
			 */
			zero_packet( &(packetSent[0]) );

			/* uniquely identifies each request */
			tag++;
			if( tag == 2048 ){
				tag = 0;
			}	

			/* distribute request across various links */
			link++;
			if( link == hmc->num_links){
				/* -- TODO : look at the number of connected links
				 * to the host processor
				 */
				link = 0;
			}

			/* 
			 * check to see if we're at the end of the packet queue
			 *
			 */
			if( iter >= num_req ){ 
				goto packet_recv;
			}

			/* DONE SENDING REQUESTS */
		}
		/*while( ret != HMC_STALL )*/

packet_recv:
		/* 
		 * reset the return code for receives
		 * 
		 */
		ret = HMC_OK;

		/* 
		 * We hit a stall or an error
		 * 
		 * Try to drain the responses off all the links
		 * 
		 */
		printf( "...reading responses\n" );
		while(all_recv != all_sent){ 

			for( z=0; z<hmc->num_links; z++)
			{ 
				printf( "...reading responses from cub:%u\n", cub);
				rtns[z] = hmcsim_recv( hmc, cub, z, &(packetRecv[0]) );

				if (rtns[z] != HMC_OK)
				{
				    continue;
				}
				
                printf("PacketRecv:");
			    for (int i = 0; i < 16; i++)
			    {
			        printf("%lx ", packetRecv[i]);
			    }
		        printf("\n");

				/* successfully received a packet */
				printf( "SUCCESS : RECEIVED A SUCCESSFUL PACKET RESPONSE\n" );	
                hmcsim_decode_memresponse( hmc,
                                          &(packetRecv[0]), 
                                          &d_response_head,
                                          &d_response_tail,
                                          &d_type,
                                          &d_length,
                                          &d_tag,
                                          &d_rtn_tag,
                                          &d_src_link,
                                          &d_rrp,
                                          &d_frp,
                                          &d_seq,
                                          &d_dinv,
                                          &d_errstat,
                                          &d_rtc,
                                          &d_crc );
                printf( "RECV tag=%x; rtn_tag=%x\n", d_tag, d_rtn_tag );

				all_recv++;
				printf( "ALL_RECV = %ld\n", all_recv );

				/* 
				 * zero the packet 
				 * 
				 */
				zero_packet( &(packetRecv[0]) );
			}

            /* As there are two cubes from which one should receive messages,
             * change the ID of cube from which you want to receive */            
            if (cub == 0)
			{
			    /* change the cub's ID */
			    cub = 1;
			}
			else if (cub == 1)
			{
			    /* change the cub's ID */
			    cub = 0;
			}
			
			printf("clocking\n");
            hmcsim_clock( hmc );
			
			
			if (all_recv == all_sent)
			{
		        done = 1;
			    break;
			}
		}

		printf( "ALL_SENT = %ld\n", all_sent );
		printf( "ALL_RECV = %ld\n", all_recv );
		fflush( stdout );
	}


complete_failure:

	fclose( ofile );
	ofile = NULL;

	free( rtns ); 
	rtns = NULL;

	return 0;
}

/* EOF */
