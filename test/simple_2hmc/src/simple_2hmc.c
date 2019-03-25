/* 
 * _SIMPLE_C_ 
 * 
 * HMCSIM SIMPLE TEST DRIVER 
 * 
 * Attempts to initialize a HMC instantiation
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hmc_sim.h"



/* ----------------------------------------------------- MAIN */
/* 
 * MAIN 
 * 
 */
extern int main( int argc, char **argv )
{
	/* vars */
	int ret			= 0;
	uint32_t num_devs	= 0;
	uint32_t num_links	= 0;
	uint32_t num_vaults	= 0;
	uint32_t queue_depth	= 0;
	uint32_t num_banks	= 0;
	uint32_t num_drams	= 0;
	uint32_t capacity	= 0;
	uint32_t xbar_depth	= 0;		
	struct hmcsim_t hmc;
	int i;
	long num_req		= 0x0Fl;	
	/* ---- */

	while(( ret = getopt( argc, argv, "b:c:d:h:l:n:q:v:x:" )) != -1 )
	{
		switch( ret )
		{
			case 'b': 
				num_banks = (uint32_t)(atoi(optarg));
				break;
			case 'c':
				capacity = (uint32_t)(atoi(optarg));
				break;
			case 'd': 
				num_drams = (uint32_t)(atoi(optarg));
				break;
			case 'h': 
				printf( "%s%s%s\n", "usage : ", argv[0], " -bcdhlnqvx" );
				printf( " -b <num_banks>\n" );
				printf( " -c <capacity>\n" );
				printf( " -d <num_drams>\n" );
				printf( " -h ...print help\n" );
				printf( " -l <num_links>\n" );
				printf( " -n <num_devs>\n" );
				printf( " -q <queue_depth>\n" );
				printf( " -v <num_vaults>\n" );
				printf( " -x <xbar_depth>\n" );
				printf( " -N <num_requests>\n" );
				return 0;
				break;
			case 'l':
				num_links = (uint32_t)(atoi(optarg));
				break;
			case 'n':
				num_devs = (uint32_t)(atoi(optarg));
				break;
			case 'q':
				queue_depth = (uint32_t)(atoi(optarg));
				break;
			case 'v': 
				num_vaults = (uint32_t)(atoi(optarg));
				break;
			case 'x': 
				xbar_depth = (uint32_t)(atoi(optarg));
				break;
			case 'N':
				num_req	= (long)(atol(optarg));
				break;
			case '?':
			default:
				printf( "%s%s%s\n", "Unknown option: see ", argv[0], " -bcdhlnqvx" );
				//return -1;
				break;
		}
	}

	/* 
	 * init the library 
	 * 
	 */

	ret = hmcsim_init(	&hmc, 
				num_devs, 
				num_links, 
				num_vaults, 
				queue_depth, 
				num_banks, 
				num_drams, 
				capacity, 
				xbar_depth );
	if( ret != 0 ){ 
		printf( "FAILED TO INIT HMCSIM\n" );	
		return -1;
	}else {
		printf( "SUCCESS : INITALIZED HMCSIM\n" );
	}
	
    /* 
	 * setup the device topology
	 * 
	 */
	if( num_devs > 1 )
	{ 
	    /* If there are multiple devices, then I need to connect them */
        /* HMC 0 link 1 and 2 connected to the host */
        ret = 0;
        
        // suppose num_links = 4
        // This is the link config between Host and Cube0, 
        // src_dev = num_devs + 1; dest_dev = 0; src_link = 0, 1; dest_link = 0, 1

        /* Host and HMC0 */
                                /* hmc,   src_dev, dest_dev, src_link, dest_link, type */
        ret = hmcsim_link_config(&hmc, (num_devs+1), 0, 0, 0, HMC_LINK_HOST_DEV);
        ret = hmcsim_link_config(&hmc, (num_devs+1), 0, 1, 1, HMC_LINK_HOST_DEV); 

        // This is the link config between Cube0 and Cube1,
        // src_dev = 0; dest_dev = 1; src_link = 2, 3; dest_link = 2, 3.
        ret = hmcsim_link_config(&hmc, 0, 1, 2, 2, HMC_LINK_DEV_DEV);
        ret = hmcsim_link_config(&hmc, 0, 1, 3, 3, HMC_LINK_DEV_DEV);

		if( ret != 0 ){ 
				printf( "FAILED TO INIT LINK %d\n", i );
				hmcsim_free( &hmc );
				return -1;
		}else{ 
				printf( "SUCCESS : INITIALIZED LINK %d\n", i );
			}
	}else{ 
		/* 
	 	 * single device, connect everyone 
		 *
		 */
		for( i=0; i<num_links; i++ ){ 
	
			ret = hmcsim_link_config( &hmc, (num_devs + 1), 0, i, i, HMC_LINK_HOST_DEV );
	
			if( ret != 0 ){ 
				printf( "FAILED TO INIT LINK %d\n", i );
				hmcsim_free( &hmc );
				return -1;
			}else{ 
				printf( "SUCCESS : INITIALIZED LINK %d\n", i );
			}
		}
	}	

	/* 
	 * set the maximum request size for all devices
	 * 
	 */
	ret = hmcsim_util_set_all_max_blocksize( &hmc, 128 );
	if( ret != 0 ){ 
		printf( "FAILED TO SET MAXIMUM BLOCK SIZE\n" );	
		hmcsim_free( &hmc );
		return -1;
	}else {
		printf( "SUCCESS : SET MAXIMUM BLOCK SIZE\n" );
	}
	
    if( execute_test( &hmc, num_req ) != 0 ) {
		printf( "FAILED TO EXECUTE TEST\n" );
		hmcsim_free( &hmc );
		return -1;
	}	

	/* 
	 * free the library data
	 * 
	 */	
	ret = hmcsim_free( &hmc );
	
	if( ret != 0 ){ 
		printf( "FAILED TO FREE HMCSIM\n" );
		return -1;
	}else {
		printf( "SUCCESS : FREE'D HMCSIM\n" );
	}
	
	return ret;
}

/* EOF */
