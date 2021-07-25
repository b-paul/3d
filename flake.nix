{
  description = "3d engine";

  inputs.nixpkgs.url = "nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }: {

    defaultPackage.x86_64-linux = 
    with import nixpkgs { system = "x86_64-linux"; };
    stdenv.mkDerivation {
      name = "3d-engine";
      buildInputs = [ pkgs.xorg.libxcb ];
      #src = lib.escape [" "] (toString ./.);
      src = self;
      buildPhase = "make";
      installPhase = "mkdir -p $out/bin; install -t $out/bin prog";
    };

    defaultApp.x86_64-linux = {
      type = "app";
      program = "${self.defaultPackage.x86_64-linux}/bin/prog";
    };

  };
}
