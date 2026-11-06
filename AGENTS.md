# Agent notes

- Build with `nix build`; deploy with `./deploy.sh` after every successful build.
- The user controls the game. Do not send input, navigate menus, launch, quit or restart
  it. Give the in-game setup needed and wait. Read-only screenshots and memory reads are fine.
- Every hook site needs RVA plus expected bytes taken from a live, decrypted dump.
- Keep the link fully static; check a new dependency does not add a DLL import
  (`x86_64-w64-mingw32-objdump -p result/*.asi | grep 'DLL Name'`).
