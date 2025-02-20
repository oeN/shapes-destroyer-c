{ pkgs, lib, config, inputs, ... }:
let
frameworks = pkgs.darwin.apple_sdk.frameworks;
in 
{
  packages = with pkgs; [ 
    meson
    ninja
  ];

  languages.python.enable = true;
}
