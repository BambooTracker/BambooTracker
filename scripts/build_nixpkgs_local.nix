{
  buildVersion,
  variant ? "release",
}:

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
  lib = pkgs.lib;

  # release by default
  release = pkgs.bambootracker-qt6;

  debug = release.overrideAttrs (oa: {
    qmakeFlags = (oa.qmakeFlags or [ ]) ++ [
      "CONFIG-=release"
      "CONFIG+=debug"
    ];

    dontStrip = true;
  });

  variants = {
    inherit debug release;
  };
in
assert lib.asserts.assertOneOf "variant" variant (builtins.attrNames variants);
variants.${variant}
