#!/usr/bin/env bash

output_dir=""

echo >&2 "[info] PWD='$(pwd)'"

while [ -n "$1" ] ; do
    opt="$1"
    echo >&2 "[info] opt='${opt}'..."

    if [ "${opt}" = "--output-dir" ]; then
	shift 1
	output_dir="$1"
    fi
    shift 1
done

echo >&2 "[info] output_dir='${output_dir}'"

list="dependencies-1 \
experiments-1 \
geometry-1 \
expsetups-1 \
expsetups-2 \
expsetups-3 \
expsetups-4 \
simsetups-1 \
simsetups-2 \
recsetups-1 \
confres-1 \
variants-1 \
varprofile-1 \
simconf-1 \
simconf-2 \
simconf-3
"

mkdir -p ${output_dir}/

for l in ${list}; do
    echo >&2 "[info] Processing Graphviz DOT file 'fltags_${l}.dot'..."
    dot -Tpng ./fltags_${l}.dot > ${output_dir}/fltags_${l}.png
done

exit 0

# end
