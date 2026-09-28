#!/usr/bin/env bash

source common.sh

needLocalStore "this test configures the build sandbox"
requireSandboxSupport
requiresUnprivilegedUserNamespaces

if [[ ! $SHELL =~ /nix/store ]]; then skipTest "Shell is not from Nix store"; fi

# Make the hardcoded /bin/sh builder and its dependencies available in the sandbox.
cat >> "$test_nix_conf" <<EOF
sandbox = true
sandbox-fallback = false
# The test store can be under /build when running inside nix build.
sandbox-build-dir = /build-tmp
sandbox-paths = /nix/store /bin/sh=$SHELL
EOF

# Trigger a rebuild by deleting one output while keeping the output shared with an input.
nix build -f ./self-input-multi-output.nix 'multi^out' --no-link
different_path="$(nix build -f ./self-input-multi-output.nix 'multi^different' --no-link --print-out-paths)"
nix store delete "$different_path"
nix build -f ./self-input-multi-output.nix 'multi^*' --no-link
