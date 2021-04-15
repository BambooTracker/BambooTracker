{ buildVersion }:

let
  packageOverrides = pkgs: {
    bambootracker = pkgs.bambootracker.overrideAttrs (oldAttrs: {
      version = buildVersion;
      src = ./..;
      patches = [];
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
