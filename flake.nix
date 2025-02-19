{
  description = "Flake for the development and compilation environment for the dam operating system";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-24.11";
  };

  outputs = { self, nixpkgs }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShells.x86_64-linux = {
      default = self.outputs.devShells.x86_64-linux.os;
      os = pkgs.mkShellNoCC {
        ENVIRONMENT_VARIABLE = "value";

        packages = with pkgs; [
           coreboot-toolchain.riscv
           qemu
           clang-tools_19 
        ];
        shellHook = "echo 'Happy hacking'";
      };
    };

    packages.x86_64-linux = {
      damos = pkgs.callPackage ./nix/package.nix { src = ./.; };
      default = pkgs.writeShellApplication {
        name = "damos-vm";
        text = ''
          ${pkgs.qemu}/bin/qemu-system-riscv64 -machine virt -bios none -kernel ${self.packages.x86_64-linux.damos}/kernel.elf -serial mon:stdio
        '';
      };
      vm = self.packages.x86_64-linux.default;
    };
  };
}
