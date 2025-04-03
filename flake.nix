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
    self,
    nixpkgs,
    pwndbg,
  }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShells.x86_64-linux = {
      default = self.outputs.devShells.x86_64-linux.os;
      os = pkgs.mkShellNoCC {
        ENVIRONMENT_VARIABLE = "value";

        packages = with pkgs;
          [
            coreboot-toolchain.riscv
            qemu
            clang-tools_19
          ]
          ++ [
            pwndbg.packages.x86_64-linux.default
          ];
        shellHook = "echo 'Happy hacking'";
      };

      uboot = let
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
          buildInputs = with pkgs; [
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
          ];

          # Environment variables for U-Boot build
          shellHook = ''
            export CROSS_COMPILE=riscv64-unknown-linux-gnu-
            export ARCH=riscv
            export OPENSBI=./opensbi/firmware/fw_dynamic.S
          '';
        };
    };

    packages.x86_64-linux = {
      damos = pkgs.callPackage ./nix/package.nix {src = ./.;};
      default = pkgs.writeShellApplication {
        name = "damos-vm";
        text = ''
          make run
        '';
      };
      debug = pkgs.writeShellApplication {
        name = "damos-vm";
        text = ''
          make debug
        '';
      };
      vm = self.packages.x86_64-linux.default;
    };
  };
}
