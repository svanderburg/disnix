#!/bin/bash -e
# Usage: $0 command.section

param=$1
command=${param:0:-2}
section=${param: -1}
id=$(echo $command | sed "s/-//g")

doclifter -x $param

sed -i -e "s|&copy;|(C)|" \
    -e "s|xml:id='$id$section'|xml:id='sec-$command'|" \
    -e "s|xml:id='synopsis'||" \
    -e "s|xml:id='description'||" \
    -e "s|xml:id='options'||" \
    -e "s|xml:id='environment'||" \
    -e "s|xml:id='copyright'||" \
    -e "s|xml:id='operations'||" \
    -e "s|xml:id='general_options'||" \
    -e "s|xml:id='importexportimport_snapshotsexport_snaps'||" \
    -e "s|xml:id='setquery_installedlockunlock_options'||" \
    -e "s|xml:id='collect_garbage_options'||" \
    -e "s|xml:id='activationdeactivationsnapshotrestoredel'||" \
    -e "s|xml:id='query_all_snapshotsquery_latest_snapshot'||" \
    -e "s|xml:id='clean_snapshots_options'||" \
    -e "s|xml:id='shell_options'||" \
    -e "s|xml:id='exit_status'||" \
    -e "s|${command^^}|$command|" \
    $command.$section.xml
