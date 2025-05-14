#!/bin/bash

mkdir pandoc_build
WORKDIR=pandoc_build

wget -O ${WORKDIR}/panDAM-OS.md "https://hackmd.io/Fk70BI6qS-mr3AI3BT8CfQ/download"

pandoc -f markdown-implicit_figures ${WORKDIR}/panDAM-OS.md \
  -o ${WORKDIR}/out.pdf \
  --csl ieee.csl \
  --citeproc \
  --number-sections \
  --pdf-engine=pdflatex
