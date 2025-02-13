{ src, stdenvNoCC, lib, coreboot-toolchain }: stdenvNoCC.mkDerivation (finalAttrs: {
  inherit src;

  pname = "damos";
  version = "1.0.0";

  # nativeBuildInputs = [
  #
  # ];
  buildInputs = [
    coreboot-toolchain.riscv
  ];

  installPhase = ''
    mkdir -p $out
    cp kernel.elf $out/kernel.elf
  '';

  meta = {
    description = "";
    homepage = "";
    license = lib.licenses.mit;
    maintainers = [];
  };
})
