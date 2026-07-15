{ buildVersion }:

let
  packageOverrides = pkgs: {
    bambootracker = pkgs.bambootracker.overrideAttrs (oldAttrs: {
      version = buildVersion;
      src = ./..;
      patches = [ ];
      preConfigure = (oldAttrs.preConfigure or "") + ''
        lupdate Project.pro
      '';
    });
  };
  pkgs = import <nixpkgs> { config = { inherit packageOverrides; }; };
in
pkgs.bambootracker-qt6
