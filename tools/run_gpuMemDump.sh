#!/bin/bash 

path_to_vma_dump="../build/debug/"
vma_dump_file="${path_to_vma_dump}vma_dump.json"
vma_out_file="${path_to_vma_dump}vma_dump_out.png"
printf "$vma_dump_file\n"
printf "$path_to_vma_dump\n"
python GpuMemDumpVis/GpuMemDumpVis.py -o "${vma_out_file}" "$vma_dump_file"

cp "$vma_out_file" .
