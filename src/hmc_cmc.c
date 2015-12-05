/*
 * _HMC_CMC_C_
 *
 * Hybrid memory cube simulation library
 *
 * Custom memory cube functionality
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "hmc_sim.h"


/* conversion table for cmc request enums, opcodes and struct indices */
struct cmc_table{
  hmc_rqst_t type;
  uint32_t cmd;
  uint32_t idx;
};

struct cmc_table ctable[HMC_MAX_CMC] = {

  {CMC04,4,0},
  {CMC05,5,1},
  {CMC06,6,2},
  {CMC07,7,3},
  {CMC20,20,4},
  {CMC21,21,5},
  {CMC22,22,6},
  {CMC23,23,7},
  {CMC32,32,8},
  {CMC36,36,9},
  {CMC37,37,10},
  {CMC38,38,11},
  {CMC39,39,12},
  {CMC41,41,13},
  {CMC42,42,14},
  {CMC43,43,15},
  {CMC44,44,16},
  {CMC45,45,17},
  {CMC46,46,18},
  {CMC47,47,19},
  {CMC56,56,20},
  {CMC57,57,21},
  {CMC58,58,22},
  {CMC59,59,23},
  {CMC60,60,24},
  {CMC61,61,25},
  {CMC62,62,26},
  {CMC63,63,27},
  {CMC69,69,28},
  {CMC70,70,29},
  {CMC71,71,30},
  {CMC72,72,31},
  {CMC73,73,32},
  {CMC74,74,33},
  {CMC75,75,34},
  {CMC76,76,35},
  {CMC77,77,36},
  {CMC78,78,37},
  {CMC85,85,38},
  {CMC86,86,39},
  {CMC87,87,40},
  {CMC88,88,41},
  {CMC89,89,42},
  {CMC90,90,43},
  {CMC91,91,44},
  {CMC92,92,45},
  {CMC93,93,46},
  {CMC94,94,47},
  {CMC102,102,48},
  {CMC103,103,49},
  {CMC107,107,50},
  {CMC108,108,51},
  {CMC109,109,52},
  {CMC110,110,53},
  {CMC111,111,54},
  {CMC112,112,55},
  {CMC113,113,56},
  {CMC114,114,57},
  {CMC115,115,58},
  {CMC116,116,59},
  {CMC117,117,60},
  {CMC118,118,61},
  {CMC120,120,62},
  {CMC121,121,63},
  {CMC122,122,64},
  {CMC123,123,65},
  {CMC124,124,66},
  {CMC125,125,67},
  {CMC126,126,68},
  {CMC127,127,69}

};

/* Prototypes of the library functions */
/* int hmcsim_register_cmc( hmc_cmcop_t, hmc_rqst_t, uint32_t cmd ) */

/* ----------------------------------------------------- HMCSIM_CMC_RAWTOIDX */
extern uint32_t hmcsim_cmc_rawtoidx( uint32_t raw ){
  uint32_t i = 0;

  for( i=0; i<HMC_MAX_CMC; i++ ){
    if( ctable[i].cmd == raw ){
      return i;
    }
  }
  return HMC_MAX_CMC; /* redundant, but squashes gcc warning */
}

/* ----------------------------------------------------- HMCSIM_CMC_IDXTOCMD */
extern hmc_rqst_t hmcsim_cmc_idxtocmd( uint32_t idx ){
  return ctable[idx].type;
}

/* ----------------------------------------------------- HMCSIM_CMC_CMDTOIDX */
extern uint32_t hmcsim_cmc_cmdtoidx( hmc_rqst_t rqst ){
  uint32_t i = 0;

  for( i=0; i<HMC_MAX_CMC; i++ ){
    if( ctable[i].type == rqst ){
      return i;
    }
  }
  return HMC_MAX_CMC; /* redundant, but squashes gcc warning */
}

/* ----------------------------------------------------- HMCSIM_REGISTER_FUNCTIONS */
/*
 * HMCSIM_REGISTER_FUNCTIONS
 *
 */
static int    hmcsim_register_functions( struct hmcsim_t *hmc, char *cmc_lib ){

  /* vars */
  hmc_cmcop_t op;
  hmc_rqst_t rqst;
  uint32_t cmd;
  uint32_t idx;
  uint32_t rsp_len;
  hmc_response_t rsp_cmd;
  uint8_t rsp_cmd_code;

  void *handle = NULL;
  int (*cmc_register)(hmc_cmcop_t *,
                      hmc_rqst_t *,
                      uint32_t *,
                      uint32_t *,
                      hmc_response_t *,
                      uint8_t *) = NULL;
  /* ---- */

  /* attempt to load the library */
  handle = dlopen( cmc_lib, RTLD_NOW );

  if( handle == NULL ){
#ifdef HMC_DEBUG
    HMCSIM_PRINT_TRACE(dlerror());
#endif
    return -1;
  }

  /* library is loaded, resolve the functions */
  /* -- hmcsim_register_cmc */
  cmc_register = (int (*)(hmc_cmcop_t *,
                          hmc_rqst_t *,
                          uint32_t *,
                          uint32_t *,
                          hmc_response_t *,
                          uint8_t *))dlsym(handle,"hmcsim_register_cmc");
  if( cmc_register == NULL ){
    dlclose( handle );
    return -1;
  }

  if( (*cmc_register)(&op,
                      &rqst,
                      &cmd,
                      &rsp_len,
                      &rsp_cmd,
                      &rsp_cmd_code) != 0 ){
    dlclose( handle );
    return -1;
  }

  idx = hmcsim_cmc_cmdtoidx( cmd );

  if( hmc->cmcs[idx].active == 1 ){
    /* previously actived, this is an error */
    dlclose( handle );
    return -1;
  }

  /* write the necessary references into the structure */
  hmc->cmcs[idx].op           = op;
  hmc->cmcs[idx].type         = rqst;
  hmc->cmcs[idx].cmd          = cmd;
  hmc->cmcs[idx].rsp_len      = rsp_len;
  hmc->cmcs[idx].rsp_cmd      = rsp_cmd;

  hmc->cmcs[idx].active       = 1;
  hmc->cmcs[idx].handle       = handle;
  hmc->cmcs[idx].cmc_register = cmc_register;

  return 0;
}

/* ----------------------------------------------------- HMCSIM_PROCESS_CMC */
extern int  hmcsim_process_cmc( struct hmcsim_t *hmc,
                                uint32_t rawcmd,
                                uint32_t dev,
                                uint32_t quad,
                                uint32_t vault,
                                uint32_t bank,
                                uint64_t addr,
                                uint32_t length,
                                uint64_t head,
                                uint64_t tail,
                                uint64_t rqst_payload[16],
                                uint64_t rsp_payload[16] ){

  /* vars */
  uint32_t idx = 0;
  /* ---- */

  idx = hmcsim_cmc_rawtoidx( rawcmd );

  if( idx == HMC_MAX_CMC ){
    /* erroneous request */
    return -1;
  }else if( hmc->cmcs[idx].active == 0 ){
    /* command not active */
    return -1;
  }

  /* command is active, process it */

  return 0;
}

/* ----------------------------------------------------- HMCSIM_FREE_CMC */
/*
 * HMCSIM_FREE_CMC
 *
 */
extern int    hmcsim_free_cmc( struct hmcsim_t *hmc ){
  uint32_t i = 0;

  if( hmc == NULL ){
    return -1;
  }

  if( hmc->cmcs == NULL ){
    return -1;
  }

  for( i=0; i<HMC_MAX_CMC; i++ ){
    if( hmc->cmcs[i].active == 1 ){
      dlclose( hmc->cmcs[i].handle );
    }
  }

  return 0;
};

/* ----------------------------------------------------- HMCSIM_LOAD_CMC */
/*
 * HMCSIM_LOAD_CMC
 *
 */
extern int      hmcsim_load_cmc( struct hmcsim_t *hmc, char *cmc_lib ){

  if((hmc == NULL) || (cmc_lib == NULL)){
    return -1;
  }

  /* register the library functions */
  if( hmcsim_register_functions( hmc, cmc_lib ) != 0 ){
    return -1;
  }

  return 0;
}

/* EOF */