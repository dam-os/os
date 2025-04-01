{
  description = "U-Boot build environment for RISC-V 64";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { 
          inherit system;
          overlays = [];
        };
        
        # RISC-V 64 Toolchain
        riscv64Toolchain = pkgs.pkgsCross.riscv64.buildPackages;
        
        # Python with necessary packages
        pythonWithPackages = pkgs.python3.withPackages (ps: with ps; [
          setuptools
          pip
          wheel
          pyyaml
          libfdt
          pytest
        ]);
        
        # Create a shell with all the necessary build tools
        ubootShell = pkgs.mkShell {
          buildInputs = with pkgs; [
            # Build essentials
            gnumake
            bison
            flex
            bc
            ncurses
            openssl
            pythonWithPackages
            swig
            dtc # Device Tree Compiler
            
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

      in {
        devShells.default = ubootShell;
      }
    );
}
