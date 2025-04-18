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
        packages = with pkgs;
          [
            coreboot-toolchain.riscv
            qemu
            clang-tools_19
          ]
          ++ [
            pwndbg.packages.x86_64-linux.default
          ]
          ++ (let
            alias = name: text: pkgs.writeShellApplication {inherit name text;};
          in [
            # Dev commands
            (alias "run" "make run")
            (alias "debug" "make debug")
            (alias "pwn" "pwndbg ./build/kernel.elf -ex 'target remote localhost:1234' -ex 'b *kmain' -ex 'c'")
          ]);
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
