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
  nix-bundle-patched = pkgs.nix-bundle.overrideAttrs (oa: {
    # https://github.com/matthewbauer/nix-bundle/pull/98
    patches = (oa.patches or []) ++ [
      (pkgs.fetchpatch {
        name = "0001-nix-bundle-nix-user-chroot-Link-libgcc_s.so.1.patch";
        url = "https://github.com/matthewbauer/nix-bundle/pull/98/commits/c047b32dc4d004189c3564eae458aecc5d28f25d.patch";
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
  build = pkgs.bambootracker;
  bundle = (import "${nix-bundle-patched}/share/nix-bundle/default.nix" {}).nix-bootstrap {
    target = pkgs.bambootracker;
    run = "/bin/BambooTracker";
  };
}
