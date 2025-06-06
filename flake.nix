{
  description = "Flake for the development and compilation environment for the dam operating system";

  inputs = {
    # nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-24.11";
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixpkgs-unstable";
    pwndbg = {
      url = "github:pwndbg/pwndbg";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    nixpkgs,
    pwndbg,
    ...
  }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShells.x86_64-linux.default = let
      riscv64Toolchain = pkgs.pkgsCross.riscv64.buildPackages;
      ubootPython = pkgs.python3.withPackages (
        ps:
          with ps; [
            setuptools
            pip
            wheel
            pyyaml
            libfdt
            pytest
          ]
      );
    in
      pkgs.mkShell {
        buildInputs = with pkgs;
          [
            # Build essentials
            gnumake
            bison
            flex
            bc
            ncurses
            openssl
            ubootPython
            swig
            dtc # Device Tree Compiler
            ubootTools
            parted

            # Additional tools
            pkg-config
            libuuid

            # Fix for gnutls/gnutls.h
            gnutls
            gnutls.dev

            # RISC-V Toolchain
            riscv64Toolchain.gcc
            riscv64Toolchain.binutils

            coreboot-toolchain.riscv
            qemu
            clang-tools_19

            # Extra
            pwndbg.packages.x86_64-linux.default
            presenterm
            pandoc
            wget
          ]
          ++ (
            let
              alias = name: text: pkgs.writeShellApplication {inherit name text;};
            in [
              # Dev commands
              (alias "run" "make run")
              (alias "debug" "make debug")
              (alias "pwn" "pwndbg ./build/kernel.elf -ex 'target remote localhost:1234' -ex 'b *kmain' -ex 'c'")
            ]
          );

        # Environment variables for U-Boot build
        shellHook = ''
          export CROSS_COMPILE=riscv64-unknown-linux-gnu-
          export ARCH=riscv
          export OPENSBI=./opensbi/firmware/fw_dynamic.S
        '';
      };
  };
}
