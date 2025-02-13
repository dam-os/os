{ src, stdenvNoCC, lib }: stdenvNoCC.mkDerivation (finalAttrs: {
  inherit src;

  pname = "damos";
  version = "1.0.0";

  # nativeBuildInputs = [
  #
  # ];
  # buildInputs = [
  #
  # ];

  meta = {
    description = "";
    homepage = "";
    license = lib.licenses.mit;
    maintainers = [];
  };
})
