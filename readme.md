Build instructions
1. Install SDL2 and SDL2_gfx
2. Create a git submodule for the imgui library
  `git submodule add 'https://github.com/ocornut/imgui' imgui`
3. Create a git submodule for the teenyat library
  `git submodule add 'https://github.com/miniat-amos/TeenyAT.git' robot_vm`
(you can skip steps 2 and 3 if you clone with the --recursive-submodules flag)
5. Compile with `make`
6. Run the executable with `./robots`
5. All set. Get robot programs from --link to be created-- and use them in your arena.

![](https://github.com/usernyan/teenyat-robots/blob/master/robots-rec.gif?raw=true)
