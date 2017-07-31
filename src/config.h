#ifndef __CONFIG_H
#define __CONFIG_H

 /* 
  * This constant determines the contributions of space utilization
  * (UTIL_WEIGHT) and throughput (1 - UTIL_WEIGHT) to the performance
  * index.  
  */
#define UTIL_WEIGHT .60

/* 
 * Alignment requirement in bytes (either 4 or 8) 
 */
#define ALIGNMENT 8  

/* 
 * Maximum heap size in bytes 
 */
#define MAX_HEAP (20*(1<<20))  /* 20 MB */


#endif
