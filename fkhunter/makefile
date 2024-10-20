EXTENSION = fkhunter
MODULE_big = fkhunter
PGFILEDESC = "Prevent unindexed foreign keys"
DATA = fkhunter--1.0.sql

OBJS = fkhunter.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
