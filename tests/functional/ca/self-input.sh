#!/usr/bin/env bash

source common.sh

nix build -f ./self-input.nix second
nix build -f ./self-input.nix second --rebuild

nix build -f ./self-input-multi-output.nix 'multi^out' --no-link
different_path="$(nix build -f ./self-input-multi-output.nix 'multi^different' --no-link --print-out-paths)"
nix store delete "$different_path"
nix build -f ./self-input-multi-output.nix 'multi^*' --no-link
