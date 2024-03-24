{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.clangStdenv.mkDerivation {
      name = "glycon";

      nativeBuildInputs = with pkgs; [
        meson
        ninja
        avrdude
        pkg-config
        editline.dev
        knightos-scas
        pkgsCross.avr.buildPackages.gcc
        pkgsCross.avr.buildPackages.binutils
        pkgsCross.avr.avrlibc
      ];
    };
  };
}
