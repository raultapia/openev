#!/bin/sh
set -e
cd "$(dirname "$0")"
for svg in logo.svg logo_no_text.svg icon.svg; do
  png="${svg%.svg}.png"
  dpi=$(sed -n 's/.*inkscape:export-xdpi="\([^"]*\)".*/\1/p' "$svg" | head -1)
  inkscape "$svg" --export-type=png --export-area-page --export-dpi="$dpi" --export-filename="$png" 2>/dev/null
  echo "$png $(identify -format '%wx%h' "$png")"
done
