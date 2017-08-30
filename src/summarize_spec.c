#include "summarize_spec.h"
#include "search_request.h"
#include "rmutil/util.h"
#include "rmutil/strings.h"

static int parseTags(RedisModuleString **argv, int argc, size_t *offset, const char **open,
                     const char **close) {
  if (argc - *offset < 3) {
    return REDISMODULE_ERR;
  }

  ++*offset;

  RMUtil_ParseArgs(argv, argc, *offset, "cc", open, close);
  *offset += 2;
  return REDISMODULE_OK;
}

static int parseFragsize(RedisModuleString **argv, int argc, size_t *offset, uint32_t *fragSize) {
  if (argc - *offset < 2) {
    return REDISMODULE_ERR;
  }
  ++*offset;
  long long tmp;
  if (RMUtil_ParseArgs(argv, argc, *offset, "l", &tmp) != REDISMODULE_OK) {
    return REDISMODULE_ERR;
  }
  *fragSize = tmp;
  ++*offset;
  return REDISMODULE_OK;
}

/**
* SUMMARIZE {num} .. FIELDnum
* SUMMARIZE [OPTIONS] {num} FIELDnum
*/
int ParseSummarizeSpecSimple(RedisModuleString **argv, int argc, size_t *offset,
                             FieldList *fields) {
  if (argc - *offset < 2) {
    return REDISMODULE_ERR;
  }

  const char *openTag = "";
  const char *closeTag = "";
  uint32_t fragSize = 0;

  ++*offset;

  while (*offset != argc) {
    if (RMUtil_StringEqualsCaseC(argv[*offset], "TAGS")) {
      if (parseTags(argv, argc, offset, &openTag, &closeTag) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
      if (fields->closeTag) {
        free(fields->closeTag);
      }
      if (fields->openTag) {
        free(fields->openTag);
      }
      fields->closeTag = strdup(closeTag);
      fields->openTag = strdup(openTag);

    } else if (RMUtil_StringEqualsCaseC(argv[*offset], "FRAGSIZE")) {
      if (parseFragsize(argv, argc, offset, &fragSize) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
    } else {
      long long nargs;
      if (RedisModule_StringToLongLong(argv[(*offset)++], &nargs) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      } else if (nargs < 1) {
        return REDISMODULE_ERR;
      } else if (nargs > argc - *offset) {
        return REDISMODULE_ERR;
      }

      // Otherwise, the fields are returned per spec.
      for (size_t ii = 0; ii < nargs; ++ii) {
        ReturnedField *field = FieldList_AddFieldR(fields, argv[*offset + ii]);
        field->contextLen = fragSize;
        field->openTag = fields->openTag;
        field->closeTag = fields->closeTag;
        field->mode = SUMMARIZE_MODE_DEFAULT;
      }
      fields->wantSummaries = 1;
      *offset += nargs;
      return REDISMODULE_OK;
    }
  }
  return REDISMODULE_OK;
}

static int parseSingleField(RedisModuleString **argv, int argc, size_t *offset, FieldList *fields) {
  // Parse all the field items
  ++*offset;
  const char *openTag = "";
  const char *closeTag = "";
  if (*offset == argc) {
    return REDISMODULE_ERR;
  }

  ReturnedField *field = FieldList_AddFieldR(fields, argv[(*offset)++]);
  fields->wantSummaries = 1;
  field->mode = SUMMARIZE_MODE_DEFAULT;

  while (*offset != argc) {
    if (RMUtil_StringEqualsCaseC(argv[*offset], "TAGS")) {
      if (parseTags(argv, argc, offset, &openTag, &closeTag) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
    } else if (RMUtil_StringEqualsCaseC(argv[*offset], "FRAGSIZE")) {
      if (parseFragsize(argv, argc, offset, &field->contextLen) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
    } else if (RMUtil_StringEqualsCaseC(argv[*offset], "ALL")) {
      field->mode = SummarizeMode_AllFragments;
      ++*offset;
    } else if (RMUtil_StringEqualsCaseC(argv[*offset], "BEST")) {
      field->mode = SummarizeMode_BestFragment;
      ++*offset;
    } else if (RMUtil_StringEqualsCaseC(argv[*offset], "NOTRUNCATE")) {
      field->mode = SummarizeMode_WholeField;
      ++*offset;
    } else {
      // Unknown field?
      return REDISMODULE_ERR;
    }
  }

  field->openTag = strdup(openTag);
  field->closeTag = strdup(closeTag);
  return REDISMODULE_OK;
}

int ParseSummarizeSpecDetailed(RedisModuleString **argv, int argc, size_t *offset,
                               FieldList *fields) {
  while (*offset != argc) {
    if (!RMUtil_StringEqualsCaseC(argv[*offset], "FIELD")) {
      return REDISMODULE_ERR;
    }
    if (parseSingleField(argv, argc, offset, fields) != REDISMODULE_OK) {
      return REDISMODULE_ERR;
    }
  }
  return REDISMODULE_OK;
}