#! /bin/sh

xxd -i doc/tinc_cache_schema.json src/tinc_cache_schema.cpp
/usr/bin/cp doc/tinc_cache_schema.json tinc-python/tinc/