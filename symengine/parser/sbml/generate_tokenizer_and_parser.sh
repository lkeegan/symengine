#!/usr/bin/env bash

# commands to generate the files tokenizer.cpp, parser.tab.cc, parser.tab.hh
# requires re2c and bison to be installed

re2c sbml_tokenizer.re -s -b --no-generation-date -o sbml_tokenizer.cpp
bison sbml_parser.yy -d -o sbml_parser.tab.cc
