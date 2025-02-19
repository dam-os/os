{ pkg-config, src, stdenvNoCC, lib, coreboot-toolchain }: stdenvNoCC.mkDerivation (finalAttrs: {
  inherit src;

  pname = "damos";
  version = "1.0.0";

  # nativeBuildInputs = [
  #
  # ];
  buildInputs = [
    coreboot-toolchain.riscv
    pkg-config
  ];

  installPhase = ''
    mkdir -p $out
    cp build/kernel.elf $out/kernel.elf
  '';

  meta = {
    description = "";
    homepage = "";
    license = lib.licenses.mit;
    maintainers = [];
  };
})
