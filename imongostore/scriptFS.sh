#!/bin/bash

PUNTO_MONTAJE=/home/utnso/fs-imongo
rm -rf $PUNTO_MONTAJE
[[ -d "$PUNTO_MONTAJE" ]] || { echo "creando $PUNTO_MONTAJE"; mkdir -p "$PUNTO_MONTAJE"; }
[[ -d "$PUNTO_MONTAJE/files/bitacora" ]] || { echo "creando $PUNTO_MONTAJE/files/Bitacora"; mkdir -p "$PUNTO_MONTAJE/files/bitacora"; }
