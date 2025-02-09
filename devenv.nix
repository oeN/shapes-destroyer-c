{ pkgs, lib, config, inputs, ... }:

{
  packages = with pkgs; [ 
    cmake
  ];

  languages.c.enable = true;
}
