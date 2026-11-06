{
  description = "hudname - Metal Gear ASI mod skeleton";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    minhook = {
      url = "github:TsudaKageyu/minhook/v1.3.4";
      flake = false;
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      minhook,
    }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      mingw = pkgs.pkgsCross.mingwW64;
      # CMakeLists.txt is the single source of the version.
      version = builtins.head (
        builtins.match ".*project\\(hudname VERSION ([0-9.]+).*" (builtins.readFile ./CMakeLists.txt)
      );
    in
    {
      packages.${system}.default = mingw.stdenv.mkDerivation {
        pname = "hudname";
        inherit version;
        src = self;
        cmakeBuildType = "MinSizeRel";
        nativeBuildInputs = [
          mingw.buildPackages.cmake
          mingw.buildPackages.ninja
        ];
        cmakeFlags = [ "-DMINHOOK_DIR=${minhook}" ];
      };

      devShells.${system} = {
        default = pkgs.mkShellNoCC {
          packages = [ pkgs.clang-tools ];
          inputsFrom = [ self.packages.${system}.default ];
        };
        # Hooks sit at hard RVAs in a runtime-decrypted .text: verify offsets
        # against a live dump, not the file on disk.
        re = pkgs.mkShellNoCC {
          packages = [
            pkgs.ghidra-bin
            (pkgs.python3.withPackages (ps: [ ps.capstone ]))
          ];
        };
      };

      formatter.${system} = pkgs.nixfmt-rfc-style;
    };
}
