#!/bin/bash

mkdir -p pandoc_build
cd pandoc_build

wget -O DAM-OS.md "https://hackmd.io/Fk70BI6qS-mr3AI3BT8CfQ/download"

wget -O ieee.csl "https://cdn.discordapp.com/attachments/1342049915911671811/1369980198782632029/ieee.csl?ex=6822723c&is=682120bc&hm=61f3b6707dc19aab118b72147da7f5a07985f15c7a7aa5e54c3e0d6da8b05087"

pandoc -f markdown-implicit_figures DAM-OS.md \
  -o out.pdf \
  --csl ieee.csl \
  --citeproc \
  -V mainfont="Liberation Serif" \
  -V fontsize=12pt \
  --toc \
  --number-sections \
  --pdf-engine=pdflatex
