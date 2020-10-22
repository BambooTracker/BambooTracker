{ pkgs ? import <nixpkgs> {}
,buildVersion }:

let
  bambootracker-local = pkgs.bambootracker.overrideAttrs (oldAttrs: {
    version = buildVersion;
    src = ./..;
    sourceRoot = "BambooTracker";
  });
in {
  build = bambootracker-local;
}
