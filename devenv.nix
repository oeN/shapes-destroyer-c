{ pkgs, lib, config, inputs, ... }:

{
  packages = with pkgs; [ 
    cmake
    lldb
  ];

  languages.c.enable = true;
}
