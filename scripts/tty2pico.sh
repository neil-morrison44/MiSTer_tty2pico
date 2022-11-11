#!/usr/bin/env bash

# Adapted from https://gist.github.com/cubedtear/54434fc66439fc4e04e28bd658189701

VERSION="2.0.0"
FIRMWARE_VERSION="2.0.0"
SCRIPT_URL='https://github.com/neil-morrison44/MiSTer_tty2pico/releases/latest/download/tty2pico.sh'
SCRIPT_LOCATION="${BASH_SOURCE[@]}"

rm -f tty_pico_updater_temp.sh

function update() {
    TMP_FILE=$(mktemp -t "" "XXXXX.sh")
    curl -s -L "$SCRIPT_URL" > "$TMP_FILE"
    NEW_VER=$(grep "^VERSION" "$TMP_FILE" | awk -F'[="]' '{print $3}')
    ABS_SCRIPT_PATH=$(readlink -f "$SCRIPT_LOCATION")
    if [ "$VERSION" \< "$NEW_VER" ]
    then
        printf "Updating script \e[31;1m%s\e[0m -> \e[32;1m%s\e[0m\n" "$VERSION" "$NEW_VER"

        echo "cp \"$TMP_FILE\" \"$ABS_SCRIPT_PATH\"" > tty_pico_updater_temp.sh
        echo "rm -f \"$TMP_FILE\"" >> tty_pico_updater_temp.sh
        echo "echo Running script again: `basename ${BASH_SOURCE[@]}` $@" >> tty_pico_updater_temp.sh
        echo "exec \"$ABS_SCRIPT_PATH\" \"$@\"" >> tty_pico_updater_temp.sh

        chmod +x tty_pico_updater_temp.sh
        chmod +x "$TMP_FILE"
        exec "./tty_pico_updater_temp.sh"
        exit 0
    else
        rm -f "$TMP_FILE"
    fi
}

update "$@"


function maybe_update_firmware() {
    # can put this off until there's a new firmware version since this script auto-updates

    # find the tty for the board (from tty2old's inis)

    # send a "CMDGETSYS" to the board
    # echo "CMDGETSYS" > /dev/ttyACM0

    # read response
    # read INPUT </dev/ttyACM0
    # echo $INPUT

    # parse response

    # compare vs `FIRMWARE_VERSION` (which'll need to be kept up to date)

    # check there's a firmware available for the board
    # if not then tell the user they've got to update manually & exit

    # if it's older, send a `CMDENOTA` to the tty

    # wait for the usb device to get mounted

    # copy over the downloaded uf2 file to the board

    # wait for the tty to come back up

    # done

}

maybe_update_firmware
