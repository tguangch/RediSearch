#ifndef RS_CONFIG_H_
#define RS_CONFIG_H_

#include "redismodule.h"

/* RSConfig is a global configuration struct for the module, it can be included from each file, and
 * is initialized with user config options during module statrtup */
typedef struct {
  // Use concurrent serach (default: 1, disable with SAFEMODE)
  int concurrentMode;
  // If not null, this points at a .so file of an extension we try to load (default: NULL)
  const char *extLoad;
  // If this is set, GC is enabled on all indexes (default: 1, disable with NOGC)
  int enableGC;
} RSConfig;

// global config extern reference
extern RSConfig RSGlobalConfig;

/* Read configuration from redis module arguments into the global config object. Return
 * REDISMODULE_ERR and sets an error message if something is invalid */
int ReadConfig(RedisModuleString **argv, int argc, const char **err);

// default configuration
#define RS_DEFAULT_CONFIG                                  \
  (RSConfig){                                              \
      .concurrentMode = 1, .extLoad = NULL, .enableGC = 1, \
  };

#endif