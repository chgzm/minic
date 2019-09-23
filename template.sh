#!/bin/bash

for node in $(cat nodes.txt)
do
    echo "static void dump_${node}(const ${node}* node, int indent) {" 
    echo "    make_indent(indent);"
    echo "    printf(\"${node}\\n\");"
    echo "}"
    echo ""
done
