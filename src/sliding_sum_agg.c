#include "postgres.h"

#include "fmgr.h"

#include "utils/hsearch.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(sliding_sum_sfunc);
PG_FUNCTION_INFO_V1(sliding_sum_finalfunc);

Datum sliding_sum_sfunc(PG_FUNCTION_ARGS);
Datum sliding_sum_finalfunc(PG_FUNCTION_ARGS);

struct hash_item {
  int key;
  double value;
};

struct state {
  double sum;
  HTAB *htab;
};

Datum sliding_sum_sfunc(PG_FUNCTION_ARGS) {
  struct state *state;

  if (PG_ARGISNULL(0)) {
    MemoryContext aggcontext;
    AggCheckCallContext(fcinfo, &aggcontext);
    MemoryContext oldcontext = MemoryContextSwitchTo(aggcontext);

    HASHCTL ctl;
    memset(&ctl, 0, sizeof(ctl));
    ctl.keysize = sizeof(int);
    ctl.entrysize = sizeof(struct hash_item);
    ctl.hash = tag_hash;
    HTAB *htab = hash_create("sliding window hash table", 2, &ctl, HASH_ELEM | HASH_FUNCTION);

    state = palloc(sizeof(struct state));
    state->sum = 0;
    state->htab = htab;

    MemoryContextSwitchTo(oldcontext);
  } else {
    state = (struct state *) PG_GETARG_POINTER(0);
  }

  int key = PG_GETARG_INT32(1);
  double value = PG_GETARG_FLOAT8(2);

  bool found;
  struct hash_item *hash_item = hash_search(state->htab, (void *)&key, HASH_ENTER, &found);

  if (found)
    state->sum -= hash_item->value;
  state->sum += value;

  hash_item->value = value;

  PG_RETURN_POINTER(state);
}

Datum sliding_sum_finalfunc(PG_FUNCTION_ARGS) {
  struct state *state = (struct state *) PG_GETARG_POINTER(0);

  PG_RETURN_FLOAT8(state->sum);
}
