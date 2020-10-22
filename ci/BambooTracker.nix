{ pkgs ? import <nixpkgs> {}
,buildVersion }:

let
  bambootracker-local = pkgs.bambootracker.overrideAttrs (oldAttrs: {
    version = buildVersion;
    src = ./..;
    sourceRoot = "BambooTracker/BambooTracker";
    preConfigure = "pwd && ls -ahl && exit 1";
  });
in {
  build = bambootracker-local;
}
