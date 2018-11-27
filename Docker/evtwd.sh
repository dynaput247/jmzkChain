#!/bin/sh
cd /opt/evt/bin

if [ -f '/opt/evt/etc/evtwd/config.ini' ]; then
    echo
  else
    mkdir /opt/evt/etc/evtwd
fi

while :; do
    case $1 in
        --config-dir=?*)
            CONFIG_DIR=${1#*=}
            ;;
        *)
            break
    esac
    shift
done

if [ ! "$CONFIG_DIR" ]; then
    CONFIG_DIR="--config-dir=/opt/evt/etc/evtwd"
else
    CONFIG_DIR=""
fi

DATA_DIR="--data-dir=/opt/evt/data/wallet"

exec /opt/evt/bin/evtwd $CONFIG_DIR $DATA_DIR $@
