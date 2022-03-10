{ buildVersion }:

let
  packageOverrides = pkgs: {
    bambootracker = pkgs.bambootracker.overrideAttrs (oldAttrs: {
      version = buildVersion;
      src = ./..;
      patches = [];
      preConfigure = "lupdate Project.pro";

      nativeBuildInputs = with pkgs; [ cmake libsForQt5.qttools libsForQt5.wrapQtAppsHook pkg-config ];
      cmakeFlags = [
        "-DSYSTEM_RTAUDIO=ON"
        "-DSYSTEM_RTMIDI=ON"
      ];
      postConfigure = ""; #"export VERBOSE=1";
    });
  };
  pkgs = import <nixpkgs> { config = { inherit packageOverrides; }; };
in {
  build = pkgs.bambootracker;
  bundle = (import "${pkgs.nix-bundle}/share/nix-bundle/default.nix" {}).nix-bootstrap {
    target = pkgs.bambootracker;
    run = "/bin/BambooTracker";
  };
}
