{ pkgs ? import <nixpkgs> {}
, buildVersion }:

let
  bambootracker-local = pkgs.bambootracker.overrideAttrs (oldAttrs: {
    version = buildVersion;
    src = ./..;
    sourceRoot = "BambooTracker/BambooTracker";
  });
in {
  build = bambootracker-local;
  bundle = (import "${pkgs.nix-bundle}/share/nix-bundle/default.nix" {}).nix-bootstrap {
    target = bambootracker-local;
    run = "/bin/BambooTracker";
  };
}
