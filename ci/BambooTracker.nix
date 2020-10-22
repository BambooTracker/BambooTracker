{ pkgs ? import <nixpkgs> {}
,buildVersion }:

let
  bambootracker-local = pkgs.bambootracker.overrideAttrs (oldAttrs: {
    version = buildVersion;
    src = ./..;
  });
in {
  build = bambootracker-local;
}
