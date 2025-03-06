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
    };

    packages.x86_64-linux = {
<<<<<<< HEAD
      damos = pkgs.callPackage ./nix/package.nix { src = ./.; };
      default = let
        inherit (pkgs) qemu;
        inherit (self.packages.x86_64-linux) damos;
      in pkgs.writeShellApplication {
        name = "damos-vm";
        text = ''
          ${qemu}/bin/qemu-system-riscv64 \
            -machine virt \
            -bios none \
            -drive id=drive0,file=file.txt,format=raw,if=none \
            -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
            -kernel ${damos}/kernel.elf \
            -serial mon:stdio
=======
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
>>>>>>> main
        '';
      };
      vm = self.packages.x86_64-linux.default;
    };
  };
}
