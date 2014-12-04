CREATE OR REPLACE FUNCTION sliding_sum_sfunc(internal, int4, float8)
RETURNS internal
AS '$libdir/sliding_sum_agg', 'sliding_sum_sfunc'
LANGUAGE C;

CREATE OR REPLACE FUNCTION sliding_sum_finalfunc(internal)
RETURNS float8
AS '$libdir/sliding_sum_agg', 'sliding_sum_finalfunc'
LANGUAGE C;

DROP AGGREGATE IF EXISTS sliding_sum(int4, float8);

CREATE AGGREGATE sliding_sum(int4, float8) (
  FINALFUNC = sliding_sum_finalfunc,
  SFUNC = sliding_sum_sfunc,
  STYPE = internal
);
