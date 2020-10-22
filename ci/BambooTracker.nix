{ pkgs ? import <nixpkgs> {} }:

let
  bambootracker-local = pkgs.bambootracker.overrideAttrs (oldAttrs: {
    src = ../;
  });
in {
  build = bambootracker-local;
}
