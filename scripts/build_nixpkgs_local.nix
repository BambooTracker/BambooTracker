{ buildVersion }:

let
  packageOverrides = pkgs: {
    bambootracker-qt6 = pkgs.bambootracker-qt6.overrideAttrs (oldAttrs: {
      version = buildVersion;
      src = ./..;
      patches = [];
      preConfigure = (oldAttrs.preConfigure or "") + ''
        lupdate Project.pro
      '';
    });
  };
  pkgs = import <nixpkgs> { config = { inherit packageOverrides; }; };
  nix-bundle-patched = pkgs.nix-bundle.overrideAttrs (oa: {
    # https://github.com/matthewbauer/nix-bundle/pull/98
    patches = (oa.patches or []) ++ [
      (pkgs.fetchpatch {
        name = "0001-nix-bundle-nix-user-chroot-Link-libgcc_s.so.1.patch";
        url = "https://github.com/nix-community/nix-bundle/commit/c047b32dc4d004189c3564eae458aecc5d28f25d.patch";
        hash = "sha256-VR45xP7rw3GzIrkznHLPPf9OIVZnlnezUjCKhfQJ47s=";
      })
    ];
    # https://github.com/matthewbauer/nix-bundle/issues/72
    postInstall = (oa.postInstall or "") + ''
      substituteInPlace $out/share/nix-bundle/nix-user-chroot/Makefile \
        --replace 'g++' 'g++ -static-libstdc++'
    '';
  });
in {
  build = pkgs.bambootracker-qt6;
  bundle = (import "${nix-bundle-patched}/share/nix-bundle/default.nix" {}).nix-bootstrap {
    target = pkgs.bambootracker-qt6;
    run = "/bin/BambooTracker";
  };
}
