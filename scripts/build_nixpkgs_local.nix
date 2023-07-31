{ buildVersion }:

let
  packageOverrides = pkgs: {
    bambootracker = (pkgs.bambootracker.override {
			# RtAudio in stable Nixpkgs still on 5.0.0
      rtaudio = pkgs.rtaudio.overrideAttrs (oa: rec {
        version = "6.0.0";
        src = pkgs.fetchFromGitHub {
          owner = "thestk";
          repo = "rtaudio";
          rev = version;
          hash = "sha256-/pbtezm3Ceo9WsMI1REJ9y72aZck55tRzrsifNeKrXA=";
        };
      });
    }).overrideAttrs (oldAttrs: {
      version = buildVersion;
      src = ./..;
      patches = [];
      preConfigure = "lupdate Project.pro";
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
