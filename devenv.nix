{ pkgs, lib, config, inputs, ... }:
let
frameworks = pkgs.darwin.apple_sdk.frameworks;
in 
{
  # packages = with pkgs; [ 
  #   cmake
  #   lldb
  #   frameworks.Kernel
  # ];

  # languages.c.enable = true;
}
