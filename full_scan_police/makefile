# Makefile for full_scan_police extension
EXTENSION = full_scan_police
DATA = full_scan_police--1.0.sql
PG_CONFIG = pg_config
MODULES =

all:
	@echo "No compilation required"

install:
	@echo "Installing extension"
	cp $(DATA) $(shell $(PG_CONFIG) --sharedir)/extension
	cp $(EXTENSION).control $(shell $(PG_CONFIG) --sharedir)/extension

uninstall:
	@echo "Uninstalling extension"
	rm -f $(shell $(PG_CONFIG) --sharedir)/extension/$(DATA)
	rm -f $(shell $(PG_CONFIG) --sharedir)/extension/$(EXTENSION).control
