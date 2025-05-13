#!/bin/bash

mkdir -p pandoc_build
cd pandoc_build

wget -O DAM-OS.md "https://hackmd.io/Fk70BI6qS-mr3AI3BT8CfQ/download"

pandoc -f markdown-implicit_figures DAM-OS.md \
  -o out.pdf \
  --csl ../ieee.csl \
  --citeproc \
  --number-sections \
  --pdf-engine=pdflatex
