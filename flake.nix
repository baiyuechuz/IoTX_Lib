{
  description = "IoTX_Lib – ESP32 Arduino library devshell";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        platformio
        python3
        gcc
        gnumake
        pkg-config
        picocom
        clang-tools
      ];

      shellHook = ''
        if [ -z "$ZSH_VERSION" ]; then
          export SHELL=${pkgs.zsh}/bin/zsh
          exec ${pkgs.zsh}/bin/zsh -l
        fi

        export PLATFORMIO_CORE_DIR="$PWD/.platformio-core"
        export PLATFORMIO_HOME_DIR="$PWD/.platformio"
        export PATH="$PWD/.platformio-penv/bin:$PATH"
        echo "IoTX_Lib devshell"
        echo "PlatformIO: $(pio --version 2>/dev/null || true)"
      '';
    };
  };
}
